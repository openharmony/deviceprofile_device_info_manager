/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_profile_manager.h"

#include <algorithm>
#include <dlfcn.h>
#include <list>
#include <thread>

#include "datetime_ex.h"
#include "dm_constants.h"

#include "content_sensor_manager_utils.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_enums.h"
#include "distributed_device_profile_log.h"
#include "event_handler_factory.h"
#include "i_sync_completed_callback.h"
#include "kv_adapter.h"
#include "multi_user_manager.h"
#include "permission_manager.h"
#include "profile_cache.h"
#include "profile_control_utils.h"
#include "profile_utils.h"
#include "static_profile_manager.h"
#include "trust_profile_manager.h"

namespace OHOS {
namespace DistributedDeviceProfile {
constexpr const char *LIB_DP_ADAPTER_NAME = "libdeviceprofileadapter.z.so";
IMPLEMENT_SINGLE_INSTANCE(DeviceProfileManager);
namespace {
    const std::string APP_ID = "distributed_device_profile_service";
    const std::string STORE_ID = "dp_kv_store";
    const std::string TAG = "DeviceProfileManager";
    const std::unordered_set<std::string> NON_OHBASE_NEED_CLEAR_SVR_NAMES {
        "collaborationFwk", "Nfc_Publish_Br_Mac_Address" };
}

int32_t DeviceProfileManager::Init()
{
    HILOGI("call!");
    int32_t initResult = DP_MANAGER_INIT_FAIL;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        deviceProfileStore_ = std::make_shared<KVAdapter>(APP_ID, STORE_ID,
            std::make_shared<KvDataChangeListener>(STORE_ID),
            std::make_shared<KvSyncCompletedListener>(STORE_ID), std::make_shared<KvDeathRecipient>(STORE_ID),
            DistributedKv::TYPE_DYNAMICAL);
        initResult = deviceProfileStore_->Init();
        if (initResult != DP_SUCCESS) {
            HILOGE("deviceProfileStore init failed");
            return initResult;
        }
        std::string localDeviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
        std::string dbKeyPrefix = ProfileUtils::GenerateDeviceProfileKey(localDeviceId);
        std::map<std::string, std::string> values;
        int32_t status = deviceProfileStore_->GetByPrefix(dbKeyPrefix, values);
        if (status == DP_SUCCESS) {
            isFirst_.store(values.empty());
        }
    }
    LoadDpSyncAdapter();
    HILOGI("Init finish, res: %{public}d", initResult);
    return initResult;
}

int32_t DeviceProfileManager::UnInit()
{
    HILOGI("call!");
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore_ is nullptr");
            return DP_KV_DB_PTR_NULL;
        }
        deviceProfileStore_->UnInit();
        deviceProfileStore_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(putTempCacheMutex_);
        putTempCache_.clear();
    }
    UnloadDpSyncAdapter();
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::ReInit()
{
    HILOGI("call!");
    UnInit();
    return Init();
}

int32_t DeviceProfileManager::PutDeviceProfile(const DeviceProfile& deviceProfile)
{
    HILOGI("deviceProfile: %{public}s", deviceProfile.AnnoymizeDump().c_str());
    if (!ProfileUtils::IsDevProfileValid(deviceProfile)) {
        HILOGE("the profile is invalid! deviceProfile: %{public}s", deviceProfile.AnnoymizeDump().c_str());
        return DP_INVALID_PARAMS;
    }
    std::map<std::string, std::string> entries;
    if (deviceProfile.IsMultiUser()) {
        ProfileUtils::DeviceProfileToEntries(deviceProfile, entries, true);
        if (MultiUserManager::GetInstance().GetCurrentForegroundUserID() == deviceProfile.GetUserId()) {
            ProfileUtils::DeviceProfileToEntries(deviceProfile, entries);
        } else {
            HILOGI("the profile does not belong to the current user.");
        }
    } else {
        ProfileUtils::DeviceProfileToEntries(deviceProfile, entries);
    }
    if (isFirst_.load()) {
        AddToPutTempCache(entries);
        return DP_SUCCESS;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_KV_DB_PTR_NULL;
        }
        if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
            HILOGE("PutDeviceProfile fail! deviceProfile: %{public}s", deviceProfile.AnnoymizeDump().c_str());
            return DP_PUT_KV_DB_FAIL;
        }
    }
    HILOGD("PutDeviceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutServiceProfile(const ServiceProfile& serviceProfile)
{
    HILOGI("serviceProfile: %{public}s", serviceProfile.dump().c_str());
    if (!ProfileUtils::IsSvrProfileValid(serviceProfile)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    std::map<std::string, std::string> entries;
    if (serviceProfile.IsMultiUser()) {
        ProfileUtils::ServiceProfileToEntries(serviceProfile, entries, true);
        if (MultiUserManager::GetInstance().GetCurrentForegroundUserID() == serviceProfile.GetUserId()) {
            ProfileUtils::ServiceProfileToEntries(serviceProfile, entries);
        } else {
            HILOGI("the profile does not belong to the current user.");
        }
    } else {
        ProfileUtils::ServiceProfileToEntries(serviceProfile, entries);
    }
    if (isFirst_.load()) {
        AddToPutTempCache(entries);
        return DP_SUCCESS;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_KV_DB_PTR_NULL;
        }
        if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
            HILOGE("PutServiceProfile fail! serviceProfile: %{public}s", serviceProfile.dump().c_str());
            return DP_PUT_KV_DB_FAIL;
        }
    }
    HILOGD("PutServiceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutServiceProfileBatch(const std::vector<ServiceProfile>& serviceProfiles)
{
    HILOGD("call!");
    std::map<std::string, std::string> entries;
    for (const auto& serviceProfile : serviceProfiles) {
        if (!ProfileUtils::IsSvrProfileValid(serviceProfile)) {
            HILOGE("the profile is invalid! serviceProfile:%{public}s", serviceProfile.dump().c_str());
            continue;
        }
        if (serviceProfile.IsMultiUser()) {
            ProfileUtils::ServiceProfileToEntries(serviceProfile, entries, true);
            if (MultiUserManager::GetInstance().GetCurrentForegroundUserID() == serviceProfile.GetUserId()) {
                ProfileUtils::ServiceProfileToEntries(serviceProfile, entries);
            } else {
                HILOGI("the profile does not belong to the current user.");
            }
        } else {
            ProfileUtils::ServiceProfileToEntries(serviceProfile, entries);
        }
    }
    if (isFirst_.load()) {
        AddToPutTempCache(entries);
        return DP_SUCCESS;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_KV_DB_PTR_NULL;
        }
        if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
            HILOGE("PutServiceProfileBatch fail!");
            return DP_PUT_KV_DB_FAIL;
        }
    }
    HILOGD("PutServiceProfileBatch success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutCharacteristicProfile(const CharacteristicProfile& charProfile)
{
    HILOGI("charProfile: %{public}s", charProfile.dump().c_str());
    if (!ProfileUtils::IsCharProfileValid(charProfile)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    std::map<std::string, std::string> entries;
    if (charProfile.IsMultiUser()) {
        ProfileUtils::CharacteristicProfileToEntries(charProfile, entries, true);
        if (MultiUserManager::GetInstance().GetCurrentForegroundUserID() == charProfile.GetUserId()) {
            ProfileUtils::CharacteristicProfileToEntries(charProfile, entries);
        } else {
            HILOGI("the profile does not belong to the current user.");
        }
    } else {
        ProfileUtils::CharacteristicProfileToEntries(charProfile, entries);
    }
    if (isFirst_.load()) {
        AddToPutTempCache(entries);
        return DP_SUCCESS;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_KV_DB_PTR_NULL;
        }
        if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
            HILOGE("PutCharacteristicProfile fail! charProfile: %{public}s", charProfile.dump().c_str());
            return DP_PUT_KV_DB_FAIL;
        }
    }
    HILOGD("PutCharacteristicProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutCharacteristicProfileBatch(const std::vector<CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    std::map<std::string, std::string> entries;
    for (const auto& charProfile : charProfiles) {
        if (!ProfileUtils::IsCharProfileValid(charProfile)) {
            HILOGE("the profile is invalid! charProfile:%{public}s", charProfile.dump().c_str());
            continue;
        }
        if (charProfile.IsMultiUser()) {
            ProfileUtils::CharacteristicProfileToEntries(charProfile, entries, true);
            if (MultiUserManager::GetInstance().GetCurrentForegroundUserID() == charProfile.GetUserId()) {
                ProfileUtils::CharacteristicProfileToEntries(charProfile, entries);
            } else {
                HILOGI("the profile does not belong to the current user.");
            }
        } else {
            ProfileUtils::CharacteristicProfileToEntries(charProfile, entries);
        }
    }
    if (isFirst_.load()) {
        AddToPutTempCache(entries);
        return DP_SUCCESS;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_KV_DB_PTR_NULL;
        }
        if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
            HILOGE("PutCharacteristicProfileBatch fail!");
            return DP_PUT_KV_DB_FAIL;
        }
    }
    HILOGD("PutCharacteristicProfileBatch success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetDeviceProfile(const std::string& deviceId, DeviceProfile& deviceProfile)
{
    deviceProfile.SetDeviceId(deviceId);
    int32_t res = IsMultiUserValid(deviceProfile);
    if (res != DP_SUCCESS) {
        HILOGE("GetDeviceProfile IsMultiUserValid failed, res: %{public}d", res);
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetDeviceProfile(deviceProfileStore_, deviceId, deviceProfile);
    }
    if (res != DP_SUCCESS) {
        HILOGE("GetDeviceProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("GetDeviceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetServiceProfile(const std::string& deviceId, const std::string& serviceName,
    ServiceProfile& serviceProfile)
{
    serviceProfile.SetDeviceId(deviceId);
    int32_t res = IsMultiUserValid(serviceProfile);
    if (res != DP_SUCCESS) {
        HILOGE("GetServiceProfile IsMultiUserValid failed, res: %{public}d", res);
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetServiceProfile(deviceProfileStore_, deviceId, serviceName,
            serviceProfile);
    }
    if (res != DP_SUCCESS) {
        HILOGE("GetServiceProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("GetServiceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
    const std::string& characteristicKey, CharacteristicProfile& charProfile)
{
    charProfile.SetDeviceId(deviceId);
    int32_t res = IsMultiUserValid(charProfile);
    if (res != DP_SUCCESS) {
        HILOGE("IsMultiUserValid fail, res:%{public}d, devId:%{public}s, svrName:%{public}s, charKey:%{public}s!",
            res, ProfileUtils::GetAnonyString(deviceId).c_str(), serviceName.c_str(), characteristicKey.c_str());
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetCharacteristicProfile(deviceProfileStore_, deviceId, serviceName,
            characteristicKey, charProfile);
    }
    if (res != DP_SUCCESS) {
        HILOGE("fail, reason: %{public}d! devId:%{public}s, svrName:%{public}s, charKey:%{public}s!",
            res, ProfileUtils::GetAnonyString(deviceId).c_str(), serviceName.c_str(), characteristicKey.c_str());
        return res;
    }
    HILOGD("GetCharacteristicProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::DeleteServiceProfile(const std::string& deviceId, const std::string& serviceName,
    bool isMultiUser, int32_t userId)
{
    HILOGD("call!");
    int32_t res = 0;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::DeleteServiceProfile(deviceProfileStore_, deviceId, serviceName, isMultiUser,
            userId);
    }
    if (res != DP_SUCCESS) {
        HILOGE("DeleteServiceProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("DeleteServiceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::DeleteCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
    const std::string& characteristicKey, bool isMultiUser, int32_t userId)
{
    HILOGD("call!");
    int32_t res = 0;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::DeleteCharacteristicProfile(deviceProfileStore_, deviceId, serviceName,
            characteristicKey, isMultiUser, userId);
    }
    if (res != DP_SUCCESS) {
        HILOGE("DeleteCharacteristicProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("DeleteCharacteristicProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::DeleteRemovedUserData(int32_t userId)
{
    if (userId < MIN_USER_ID || userId > MAX_USER_ID) {
        HILOGE("Invalid userId: %{public}d", userId);
        return DP_INVALID_PARAM;
    }
    std::map<std::string, std::string> allLocalEntries;
    std::vector<std::string> keysToDelete;
    std::string localDeviceId = ProfileCache::GetInstance().GetLocalUdid();
    if (localDeviceId.empty()) {
        HILOGE("GetLocalUdid fail");
        return DP_GET_LOCAL_UDID_FAILED;
    }
    int32_t getRet = GetProfilesByKeyPrefix(localDeviceId, allLocalEntries);
    if (getRet != DP_SUCCESS) {
        HILOGE("GetLocalProfile fail,deviceId: %{public}s,reason: %{public}d!",
            ProfileUtils::GetAnonyString(localDeviceId).c_str(), getRet);
        return getRet;
    }
    for (const auto& pair : allLocalEntries) {
        if (userId == ProfileUtils::GetUserIdFromDbKey(pair.first)) {
            keysToDelete.emplace_back(pair.first);
        }
    }
    if (keysToDelete.empty()) {
        HILOGI("userId:%{public}d has no multi-user data.", userId);
        return DP_SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("dynamicProfileStore is nullptr!");
        return DP_KV_DB_PTR_NULL;
    }
    int32_t delRet = deviceProfileStore_->DeleteBatch(keysToDelete);
    if (delRet != DP_SUCCESS) {
        HILOGE("DeleteBatch fail, reason: %{public}d!", delRet);
        return delRet;
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllDeviceProfile(std::vector<DeviceProfile>& deviceProfiles)
{
    HILOGD("call!");
    int32_t res = 0;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetAllDeviceProfile(deviceProfileStore_, deviceProfiles);
    }
    if (res != DP_SUCCESS) {
        HILOGE("GetAllDeviceProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("GetAllDeviceProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllServiceProfile(std::vector<ServiceProfile>& serviceProfiles)
{
    HILOGD("call!");
    int32_t res = 0;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetAllServiceProfile(deviceProfileStore_, serviceProfiles);
    }
    if (res != DP_SUCCESS) {
        HILOGE("serviceProfiles fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("serviceProfiles success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllCharacteristicProfile(std::vector<CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    int32_t res = 0;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        res = ProfileControlUtils::GetAllCharacteristicProfile(deviceProfileStore_, charProfiles);
    }
    if (res != DP_SUCCESS) {
        HILOGE("GetAllCharacteristicProfile fail, reason: %{public}d!", res);
        return res;
    }
    HILOGD("GetAllCharacteristicProfile success");
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::SyncDeviceProfile(const DistributedDeviceProfile::DpSyncOptions& syncOptions,
    sptr<IRemoteObject> syncCompletedCallback)
{
    HILOGI("call!");
    if (syncCompletedCallback == nullptr) {
        HILOGE("Params is invalid!");
        return DP_INVALID_PARAMS;
    }
    std::vector<std::string> ohBasedDevices;
    std::vector<std::string> notOHBasedDevices;
    ProfileCache::GetInstance().FilterAndGroupOnlineDevices(syncOptions.GetDeviceList(),
        ohBasedDevices, notOHBasedDevices);
    if (ohBasedDevices.empty() && notOHBasedDevices.empty()) {
        HILOGE("Params is invalid!");
        return DP_INVALID_PARAMS;
    }
    std::string callerDescriptor = PermissionManager::GetInstance().GetCallerProcName();
    if (!ohBasedDevices.empty()) {
        ProfileCache::GetInstance().AddSyncListener(callerDescriptor, syncCompletedCallback);
        {
            std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
            if (deviceProfileStore_ == nullptr) {
                HILOGE("deviceProfileStore is nullptr");
                return DP_SYNC_DEVICE_FAIL;
            }
            int32_t syncResult = deviceProfileStore_->Sync(ohBasedDevices, syncOptions.GetSyncMode());
            if (syncResult != DP_SUCCESS) {
                HILOGE("SyncDeviceProfile fail, res: %{public}d!", syncResult);
                return DP_SYNC_DEVICE_FAIL;
            }
        }
    }
    if (!notOHBasedDevices.empty()) {
        auto syncTask = [this, notOHBasedDevices, callerDescriptor, syncCompletedCallback]() {
            SyncWithNotOHBasedDevice(notOHBasedDevices, callerDescriptor, syncCompletedCallback);
        };
        std::thread(syncTask).detach();
    }
    HILOGI("SyncDeviceProfile success, caller: %{public}s!", callerDescriptor.c_str());
    return DP_SUCCESS;
}

bool DeviceProfileManager::LoadDpSyncAdapter()
{
    HILOGI("start.");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (isAdapterSoLoaded_ && (dpSyncAdapter_ != nullptr)) {
        return true;
    }
    int64_t beginTime = GetTickCount();
    std::string soName = std::string(LIB_DP_ADAPTER_NAME);
    if ((soName.length() == 0) || (soName.length() > PATH_MAX)) {
        HILOGE("File %{public}s canonicalization failed", soName.c_str());
        return false;
    }
    void *so_handle = dlopen(soName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soName.c_str(), RTLD_NOW);
    }
    if (so_handle == nullptr) {
        HILOGE("load dp sync adapter so %{public}s failed", soName.c_str());
        return false;
    }
    dlerror();
    auto func = (CreateDPSyncAdapterFuncPtr)dlsym(so_handle, "CreateDPSyncAdaptertObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(so_handle);
        HILOGE("Create object function is not exist.");
        return false;
    }
    auto adapter = func();
    if (adapter == nullptr) {
        dlclose(so_handle);
        HILOGE("adapter is nullptr");
        return false;
    }
    dpSyncAdapter_ = std::shared_ptr<IDPSyncAdapter>(adapter);
    if (dpSyncAdapter_->Initialize() != DP_SUCCESS) {
        dpSyncAdapter_->Release();
        dpSyncAdapter_ = nullptr;
        isAdapterSoLoaded_ = false;
        HILOGE("dp sync adapter init failed");
        return false;
    }
    isAdapterSoLoaded_ = true;
    int64_t endTime = GetTickCount();
    HILOGI("sucess. spend %{public}" PRId64 " ms", endTime - beginTime);
    return true;
}

void DeviceProfileManager::UnloadDpSyncAdapter()
{
    HILOGD("start.");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (dpSyncAdapter_ != nullptr) {
        dpSyncAdapter_->Release();
    }
    dpSyncAdapter_ = nullptr;
    std::string soPathName = std::string(LIB_DP_ADAPTER_NAME);
    if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX)) {
        HILOGE("File %{public}s canonicalization failed", soPathName.c_str());
        return;
    }
    void *so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        HILOGI("dp sync adapter so_handle is not nullptr.");
        dlclose(so_handle);
        isAdapterSoLoaded_ = false;
    }
}

int32_t DeviceProfileManager::SyncWithNotOHBasedDevice(const std::vector<std::string>& notOHBasedDevices,
    const std::string& callerDescriptor, sptr<IRemoteObject> syncCompletedCallback)
{
    if (!LoadDpSyncAdapter()) {
        HILOGE("dp service adapter load failed.");
        SyncWithNotOHBasedDeviceFailed(notOHBasedDevices, syncCompletedCallback);
        return DP_LOAD_SYNC_ADAPTER_FAILED;
    }
    for (const auto& deviceId : notOHBasedDevices) {
        if (RunloadedFunction(deviceId, syncCompletedCallback) != DP_SUCCESS) {
            HILOGE("Sync With NotOHBasedDevice Failed. deviceId:%{public}s",
                ProfileUtils::GetAnonyString(deviceId).c_str());
            SyncWithNotOHBasedDeviceFailed({deviceId}, syncCompletedCallback);
        }
    }
    return DP_SUCCESS;
}

void DeviceProfileManager::SyncWithNotOHBasedDeviceFailed(const std::vector<std::string>& notOHBasedDevices,
    sptr<IRemoteObject> syncCompletedCallback)
{
    std::map<std::string, SyncStatus> syncResults;
    for (const auto& deviceId : notOHBasedDevices) {
        syncResults[deviceId] = SyncStatus::FAILED;
    }
    sptr<ISyncCompletedCallback> syncListenerProxy = iface_cast<ISyncCompletedCallback>(syncCompletedCallback);
    if (syncListenerProxy == nullptr) {
        HILOGE("Cast to ISyncCompletedCallback failed");
        return;
    }
    syncListenerProxy->OnSyncCompleted(syncResults);
}

int32_t DeviceProfileManager::RunloadedFunction(const std::string& deviceId, sptr<IRemoteObject> syncCompletedCallback)
{
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (dpSyncAdapter_ == nullptr) {
        HILOGE("dpSyncAdapter is nullptr.");
        return DP_LOAD_SYNC_ADAPTER_FAILED;
    }
    if (dpSyncAdapter_->DetectRemoteDPVersion(deviceId) != DP_SUCCESS) {
        HILOGE("dp service adapter detect remote version failed.");
        return DP_LOAD_SYNC_ADAPTER_FAILED;
    }
    const std::list<std::string> deviceIdList = { deviceId };
    if (dpSyncAdapter_->SyncProfile(deviceIdList, syncCompletedCallback) != DP_SUCCESS) {
        HILOGE("dp service adapter sync profile failed.");
        return DP_LOAD_SYNC_ADAPTER_FAILED;
    }
    HILOGD("dp service adapter sync profile success.");
    return DP_SUCCESS;
}

std::vector<DistributedKv::Entry> DeviceProfileManager::GetEntriesByKeys(const std::vector<std::string>& keys)
{
    HILOGD("call!");
    std::vector<DistributedKv::Entry> entries;
    if (keys.empty()) {
        HILOGE("keys empty.");
        return entries;
    }
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("dynamicProfileStore is nullptr!");
            return entries;
        }
        for (const auto& key : keys) {
            std::string value;
            if (deviceProfileStore_->Get(key, value) != DP_SUCCESS) {
                continue;
            }
            DistributedKv::Entry entry;
            DistributedKv::Key kvKey(key);
            entry.key = kvKey;
            entry.value = value;
            entries.emplace_back(entry);
        }
    }
    return entries;
}

void DeviceProfileManager::AddToPutTempCache(const std::map<std::string, std::string>& values)
{
    if (values.empty()) {
        HILOGW("values empty!");
        return;
    }
    HILOGI("values.size : %{public}zu", values.size());
    {
        std::lock_guard<std::mutex> lock(putTempCacheMutex_);
        for (const auto& [key, value] : values) {
            putTempCache_[key] = value;
        }
    }
}

int32_t DeviceProfileManager::SavePutTempCache(std::map<std::string, std::string>& entries)
{
    HILOGI("business entries.size : %{public}zu", entries.size());
    {
        std::lock_guard<std::mutex> lock(putTempCacheMutex_);
        if (!putTempCache_.empty()) {
            for (const auto& [key, value] : putTempCache_) {
                entries[key] = value;
            }
        }
    }
    if (entries.empty()) {
        HILOGW("all entries empty!");
        isFirst_.store(false);
        {
            std::lock_guard<std::mutex> lock(putTempCacheMutex_);
            putTempCache_.clear();
        }
        return DP_SUCCESS;
    }
    HILOGI("all entries.size : %{public}zu", entries.size());
    int32_t ret = DP_SUCCESS;
    {
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_GET_KV_DB_FAIL;
        }
        ret = deviceProfileStore_->PutBatch(entries);
        if (ret != DP_SUCCESS) {
            HILOGE("PutBatch fail! ret:%{public}d", ret);
            return ret;
        }
    }
    isFirst_.store(false);
    {
        std::lock_guard<std::mutex> lock(putTempCacheMutex_);
        putTempCache_.clear();
    }
    HILOGI("put all entries success");
    return ret;
}

bool DeviceProfileManager::IsFirstInitDB()
{
    return isFirst_.load();
}

void DeviceProfileManager::ResetFirst()
{
    isFirst_.store(false);
}

void DeviceProfileManager::OnDeviceOnline(const TrustedDeviceInfo& deviceInfo)
{
    FixDataOnDeviceOnline(deviceInfo);
    NotifyNotOHBaseOnline(deviceInfo);
    if (ContentSensorManagerUtils::GetInstance().IsDeviceE2ESync()) {
        HILOGI("need E2ESync, deviceInfo:%{public}s", deviceInfo.dump().c_str());
        E2ESyncDynamicProfile(deviceInfo);
        StaticProfileManager::GetInstance().E2ESyncStaticProfile(deviceInfo);
    }
}

void DeviceProfileManager::FixDataOnDeviceOnline(const TrustedDeviceInfo& deviceInfo)
{
    HILOGI("deviceInfo=%{public}s", deviceInfo.dump().c_str());
    if (deviceInfo.GetNetworkId().empty()) {
        HILOGE("networkId is empty!");
        return;
    }
    auto task = [this, deviceInfo]() {
        std::string localUdid = ProfileCache::GetInstance().GetLocalUdid();
        std::string localUuid = ProfileCache::GetInstance().GetLocalUuid();
        if (localUdid.empty() || localUuid.empty()) {
            HILOGE("Get local udid fail.");
            return;
        }
        std::map<std::string, std::string> localDataByOwner;
        if (GetProfilesByOwner(localUuid, localDataByOwner) != DP_SUCCESS) {
            HILOGE("GetProfilesByOwner fail, localUuid=%{public}s", ProfileUtils::GetAnonyString(localUuid).c_str());
            return;
        }
        FixLocalData(localUdid, localDataByOwner);
        if (deviceInfo.GetUdid().empty()) {
            HILOGE("remoteUdid empty. remoteNetworkId=%{public}s",
                ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
            return;
        }
        if (deviceInfo.GetOsType() != OHOS_TYPE) {
            FixRemoteDataWhenPeerIsNonOH(deviceInfo.GetUdid());
            return;
        }
        FixRemoteDataWhenPeerIsOHBase(deviceInfo.GetUdid(), localDataByOwner);
    };
    auto handler = EventHandlerFactory::GetInstance().GetEventHandler();
    if (handler == nullptr || !handler->PostTask(task)) {
        HILOGE("Post FixDataOnDeviceOnline task faild");
        return;
    }
}

int32_t DeviceProfileManager::DeleteBatchByKeys(const std::vector<std::string>& delKeys)
{
    HILOGD("delKeys.size:%{public}zu", delKeys.size());
    if (delKeys.empty()) {
        HILOGW("delKeys is empty");
        return DP_SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("dynamicProfileStore is nullptr!");
        return DP_KV_DB_PTR_NULL;
    }
    if (deviceProfileStore_->DeleteBatch(delKeys) != DP_SUCCESS) {
        HILOGE("DeleteBatch fail");
        return DP_DEL_KV_DB_FAIL;
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetProfilesByOwner(const std::string& uuid, std::map<std::string, std::string>& values)
{
    HILOGD("uuid:%{public}s", ProfileUtils::GetAnonyString(uuid).c_str());
    if (uuid.empty()) {
        HILOGW("uuid is empty");
        return DP_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("dynamicProfileStore is nullptr!");
        return DP_KV_DB_PTR_NULL;
    }
    if (deviceProfileStore_->GetDeviceEntries(uuid, values) != DP_SUCCESS) {
        HILOGE("GetDeviceEntries fail, uuid=%{public}s", ProfileUtils::GetAnonyString(uuid).c_str());
        return DP_GET_DEVICE_ENTRIES_FAIL;
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetProfilesByKeyPrefix(const std::string& udid,
    std::map<std::string, std::string>& values)
{
    HILOGD("udid:%{public}s", ProfileUtils::GetAnonyString(udid).c_str());
    if (udid.empty()) {
        HILOGW("udid is empty");
        return DP_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("dynamicProfileStore is nullptr!");
        return DP_KV_DB_PTR_NULL;
    }
    if (deviceProfileStore_->GetByPrefix(DEV_PREFIX + SEPARATOR + udid, values) != DP_SUCCESS) {
        HILOGE("Get dev profile by prefix fail, udid=%{public}s", ProfileUtils::GetAnonyString(udid).c_str());
        return DP_GET_KV_DB_FAIL;
    }
    if (deviceProfileStore_->GetByPrefix(SVR_PREFIX + SEPARATOR + udid, values) != DP_SUCCESS) {
        HILOGE("Get svr profile by prefix fail, udid=%{public}s", ProfileUtils::GetAnonyString(udid).c_str());
        return DP_GET_KV_DB_FAIL;
    }
    if (deviceProfileStore_->GetByPrefix(CHAR_PREFIX + SEPARATOR + udid, values) != DP_SUCCESS) {
        HILOGE("Get char profile by prefix fail, udid=%{public}s", ProfileUtils::GetAnonyString(udid).c_str());
        return DP_GET_KV_DB_FAIL;
    }
    return DP_SUCCESS;
}

void DeviceProfileManager::NotifyNotOHBaseOnline(const TrustedDeviceInfo& deviceInfo)
{
    HILOGI("deviceInfo:%{public}s", deviceInfo.dump().c_str());
    if (deviceInfo.GetNetworkId().empty()) {
        HILOGE("networkId is empty!");
        return;
    }
    if (deviceInfo.GetOsType() == OHOS_TYPE) {
        HILOGD("device is ohbase. networkId=%{public}s",
            ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
        return;
    }
    if (!ProfileUtils::IsP2p(deviceInfo.GetAuthForm())) {
        HILOGD("is not point 2 point. remoteNetworkId=%{public}s",
            ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
        return;
    }
    auto task = [this, deviceInfo]() {
        if (deviceInfo.GetUdid().empty()) {
            HILOGE("remoteUdid empty. remoteNetworkId=%{public}s",
                ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
            return;
        }
        std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
        if (dpSyncAdapter_ == nullptr) {
            HILOGE("dpSyncAdapter is nullptr.");
            return;
        }
        int32_t ret = dpSyncAdapter_->NotOHBaseDeviceOnline(deviceInfo.GetUdid(), deviceInfo.GetNetworkId(),
            ProfileUtils::IsP2p(deviceInfo.GetAuthForm()));
        if (ret != DP_SUCCESS) {
            HILOGE("NotOHBaseDeviceOnline fail. ret=%{public}d, remoteNetworkId=%{public}s, authForm=%{public}d",
                ret, ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str(), deviceInfo.GetAuthForm());
            return;
        }
    };
    auto handler = EventHandlerFactory::GetInstance().GetEventHandler();
    if (handler == nullptr || !handler->PostTask(task)) {
        HILOGE("Post NotifyNotOHBaseOnline task faild");
        return;
    }
}

void DeviceProfileManager::E2ESyncDynamicProfile(const TrustedDeviceInfo& deviceInfo)
{
    HILOGI("deviceInfo:%{public}s", deviceInfo.dump().c_str());
    auto task = [this, deviceInfo]() {
        HILOGD("networkId:%{public}s", ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
        if (deviceInfo.GetNetworkId().empty()) {
            HILOGE("networkId or extraData is empty!");
            return;
        }
        if (deviceInfo.GetOsType() != OHOS_TYPE) {
            HILOGI("device is not ohbase. remoteNetworkId=%{public}s",
                ProfileUtils::GetAnonyString(deviceInfo.GetNetworkId()).c_str());
            return;
        }
        std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr");
            return;
        }
        int32_t syncResult = deviceProfileStore_->Sync({deviceInfo.GetNetworkId()}, SyncMode::PUSH_PULL);
        if (syncResult != DP_SUCCESS) {
            HILOGE("E2ESyncDynamicProfile fail, res: %{public}d!", syncResult);
            return;
        }
        HILOGI("E2ESyncDynamicProfile success!");
    };
    auto handler = EventHandlerFactory::GetInstance().GetEventHandler();
    if (handler == nullptr || !handler->PostTask(task)) {
        HILOGE("Post E2ESyncDynamicProfile task fail!");
        return;
    }
}

// Clean data that does not belong to the local.
void DeviceProfileManager::FixLocalData(const std::string& localUdid,
    const std::map<std::string, std::string>& localDataByOwner)
{
    HILOGD("localUdid:%{public}s,localDataByOwner.size:%{public}zu",
        ProfileUtils::GetAnonyString(localUdid).c_str(), localDataByOwner.size());
    if (localDataByOwner.empty()) { return; }
    std::map<std::string, std::string> localDataByKeyPrefix;
    if (GetProfilesByKeyPrefix(localUdid, localDataByKeyPrefix) != DP_SUCCESS) {
        HILOGE("GetProfilesByKeyPrefix fail, localUdid=%{public}s", ProfileUtils::GetAnonyString(localUdid).c_str());
        return;
    }
    HILOGD("localDataByKeyPrefix.size:%{public}zu", localDataByKeyPrefix.size());
    if (localDataByKeyPrefix.empty()) { return; }
    std::vector<std::string> delKeys;
    // cloud has local data, but the data is not written by local
    for (const auto& [key, _] : localDataByKeyPrefix) {
        if (localDataByOwner.find(key) == localDataByOwner.end()) {
            HILOGI("delKey: %{public}s", ProfileUtils::GetDbKeyAnonyString(key).c_str());
            delKeys.emplace_back(key);
        }
    }
    HILOGD("delKeys.size:%{public}zu", delKeys.size());
    if (delKeys.empty()) { return; }
    if (DeleteBatchByKeys(delKeys) != DP_SUCCESS) {
        HILOGE("DeleteBatchByKeys fail, localUdid=%{public}s", ProfileUtils::GetAnonyString(localUdid).c_str());
        return;
    }
}

// Clean ohbase data when the peer is non-ohbase
void DeviceProfileManager::FixRemoteDataWhenPeerIsNonOH(const std::string& remoteUdid)
{
    HILOGD("remoteUdid:%{public}s", ProfileUtils::GetAnonyString(remoteUdid).c_str());
    std::map<std::string, std::string> remoteDataByKeyPrefix;
    if (GetProfilesByKeyPrefix(remoteUdid, remoteDataByKeyPrefix) != DP_SUCCESS) {
        HILOGE("GetProfilesByKeyPrefix fail, remoteUdid=%{public}s", ProfileUtils::GetAnonyString(remoteUdid).c_str());
        return;
    }
    std::vector<std::string> delKeys;
    for (const auto& [key, _] : remoteDataByKeyPrefix) {
        std::vector<std::string> res;
        if (ProfileUtils::SplitString(key, SEPARATOR, res) != DP_SUCCESS || res.size() < NUM_3) {
            HILOGW("SplitString fail, key: %{public}s", ProfileUtils::GetDbKeyAnonyString(key).c_str());
            continue;
        }
        if (ProfileUtils::EndsWith(res[NUM_2], OH_PROFILE_SUFFIX)) {
            HILOGI("delKey: %{public}s", ProfileUtils::GetDbKeyAnonyString(key).c_str());
            delKeys.emplace_back(key);
            continue;
        }
        if ((res[0] == SVR_PREFIX || res[0] == CHAR_PREFIX) &&
            NON_OHBASE_NEED_CLEAR_SVR_NAMES.find(res[NUM_2]) != NON_OHBASE_NEED_CLEAR_SVR_NAMES.end()) {
            HILOGI("delKey: %{public}s", ProfileUtils::GetDbKeyAnonyString(key).c_str());
            delKeys.emplace_back(key);
            continue;
        }
    }
    HILOGD("delKeys.size:%{public}zu", delKeys.size());
    if (delKeys.empty()) { return; }
    if (DeleteBatchByKeys(delKeys) != DP_SUCCESS) {
        HILOGE("DeleteBatchByKeys fail, remoteUdid=%{public}s", ProfileUtils::GetAnonyString(remoteUdid).c_str());
        return;
    }
}

// Clean non-ohbase data when the peer is ohbase
void DeviceProfileManager::FixRemoteDataWhenPeerIsOHBase(const std::string& remoteUdid,
    const std::map<std::string, std::string>& localDataByOwner)
{
    HILOGD("remoteUdid:%{public}s", ProfileUtils::GetAnonyString(remoteUdid).c_str());
    std::vector<std::string> delKeys;
    // local has remote data, and the data is written by local
    for (const auto& [key, _] : localDataByOwner) {
        if (key.find(remoteUdid) != std::string::npos) {
            HILOGI("delKey: %{public}s", ProfileUtils::GetDbKeyAnonyString(key).c_str());
            delKeys.emplace_back(key);
        }
    }
    HILOGD("delKeys.size:%{public}zu", delKeys.size());
    if (delKeys.empty()) { return; }
    if (deviceProfileStore_->DeleteBatch(delKeys) != DP_SUCCESS) {
        HILOGE("DeleteBatch failed, remoteUdid=%{public}s", ProfileUtils::GetAnonyString(remoteUdid).c_str());
        return;
    }
}

void DeviceProfileManager::FixDiffProfiles()
{
    HILOGI("call");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (dpSyncAdapter_ == nullptr) {
        HILOGE("dpSyncAdapter is nullptr!");
        return;
    }
    int32_t ret = dpSyncAdapter_->FixDiffProfiles({});
    if (ret!= DP_SUCCESS) {
        HILOGE("FixDiffProfiles fail! ret=%{public}d", ret);
        return;
    }
    return;
}

void DeviceProfileManager::OnUserChange(int32_t lastUserId, int32_t curUserId)
{
    HILOGI("lastUserId:%{public}d,curUserId:%{public}d", lastUserId, curUserId);
    if (lastUserId == curUserId) {
        HILOGW("user not change");
        return;
    }
    std::string localUdid = ProfileCache::GetInstance().GetLocalUdid();
    if (localUdid.empty()) {
        HILOGE("GetLocalUdid fail");
        return;
    }
    std::map<std::string, std::string> profileMap;
    if (GetProfilesByKeyPrefix(localUdid, profileMap) != DP_SUCCESS || profileMap.empty()) {
        HILOGE("Get All Local Profiles fail");
        return;
    }
    std::unordered_set<std::string> lastUserDbKeysWithoutUID;
    std::map<std::string, std::string> curUserProfiles;
    for (const auto& [key, value] : profileMap) {
        int32_t userId = ProfileUtils::GetUserIdFromDbKey(key);
        if (userId != DEFAULT_USER_ID && userId == lastUserId) {
            std::string dbKeyWithoutUID = ProfileUtils::RemoveUserIdFromDbKey(key);
            if (!dbKeyWithoutUID.empty()) {
                lastUserDbKeysWithoutUID.insert(dbKeyWithoutUID);
            }
        }
        if (userId != DEFAULT_USER_ID && userId == curUserId) {
            curUserProfiles[key] = value;
        }
    }
    std::map<std::string, std::string> curUserProfilesWithoutUID;
    for (const auto& [key, value] : curUserProfiles) {
        std::string dbKeyWithoutUID = ProfileUtils::RemoveUserIdFromDbKey(key);
        if (!dbKeyWithoutUID.empty()) {
            lastUserDbKeysWithoutUID.erase(dbKeyWithoutUID);
            curUserProfilesWithoutUID[dbKeyWithoutUID] = value;
        }
    }
    int32_t ret = SaveBatchByKeys(curUserProfilesWithoutUID);
    if (ret != DP_SUCCESS) {
        HILOGE("SaveBatchByKeys fail ret=%{public}d", ret);
        return;
    }
    std::vector<std::string> delKeys{lastUserDbKeysWithoutUID.begin(), lastUserDbKeysWithoutUID.end()};
    ret = DeleteBatchByKeys(delKeys);
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteBatchByKeys fail ret=%{public}d", ret);
        return;
    }
}

int32_t DeviceProfileManager::SaveBatchByKeys(const std::map<std::string, std::string>& entries)
{
    HILOGD("entries.size:%{public}zu", entries.size());
    if (entries.empty()) {
        HILOGW("entries is empty");
        return DP_SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dynamicStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_GET_KV_DB_FAIL;
    }
    int32_t ret = deviceProfileStore_->PutBatch(entries);
    if (ret != DP_SUCCESS) {
        HILOGE("PutBatch fail! ret:%{public}d", ret);
        return ret;
    }
    return DP_SUCCESS;
}

template <typename T>
int32_t DeviceProfileManager::IsMultiUserValid(const T& profile)
{
    if (profile.GetDeviceId().empty() || (profile.IsMultiUser() && profile.GetUserId() <= 0) ||
        (!profile.IsMultiUser() && profile.GetUserId() != DEFAULT_USER_ID)) {
        HILOGE("multi-user params are invalid, isMultiUser: %{public}d, userId: %{public}d",
            profile.IsMultiUser(), profile.GetUserId());
        return DP_GET_MULTI_USER_PROFILE_PARAMS_INVALID;
    }
    if (profile.IsMultiUser() && (profile.GetDeviceId() == ProfileCache::GetInstance().GetLocalUdid()) &&
        (profile.GetUserId() != MultiUserManager::GetInstance().GetCurrentForegroundUserID())) {
        HILOGE("this userId is not foregroundUserId, userId: %{public}d, foregroundId: %{public}d",
            profile.GetUserId(), MultiUserManager::GetInstance().GetCurrentForegroundUserID());
        return DP_GET_LOCAL_PROFILE_IS_NOT_FOREGROUND_ID;
    }
    return DP_SUCCESS;
}

template int32_t DeviceProfileManager::IsMultiUserValid<DeviceProfile>(const DeviceProfile& profile);
template int32_t DeviceProfileManager::IsMultiUserValid<ServiceProfile>(const ServiceProfile& profile);
template int32_t DeviceProfileManager::IsMultiUserValid<CharacteristicProfile>(const CharacteristicProfile& profile);

bool DeviceProfileManager::IsSameAccount(const std::string deviceId, const int32_t userId)
{
    (void)userId;
    if (deviceId.empty()) {
        HILOGE("param is invalid");
        return false;
    }

    std::map<std::string, std::string> params = {
        {BUNDLENAME, EMPTY_STRING},
        {BIND_TYPE, std::to_string(static_cast<int32_t>(BindType::SAME_ACCOUNT))},
        {STATUS, std::to_string(static_cast<int32_t>(Status::ACTIVE))}
    };
    std::vector<AccessControlProfile> profile;
    int32_t res = TrustProfileManager::GetInstance().GetAccessControlProfile(params, profile);
    if (res != DP_SUCCESS || profile.empty()) {
        HILOGE("GetAccessControlProfile failed, res: %{public}d", res);
        return false;
    }
    for (auto& item : profile) {
        if (item.GetTrustDeviceId() == deviceId) {
            HILOGI("profile has the same account");
            return true;
        }
    }
    return false;
}

bool DeviceProfileManager::HasTrustP2PRelation(const std::string deviceId, const int32_t userId)
{
    (void)userId;
    if (deviceId.empty()) {
        HILOGE("param is invalid");
        return false;
    }

    std::map<std::string, std::string> params = {
        {BUNDLENAME, EMPTY_STRING},
        {BIND_TYPE, std::to_string(static_cast<int32_t>(BindType::POINT_TO_POINT))},
        {STATUS, std::to_string(static_cast<int32_t>(Status::ACTIVE))}
    };
    std::vector<AccessControlProfile> profile;
    int32_t res = TrustProfileManager::GetInstance().GetAccessControlProfile(params, profile);
    if (res != DP_SUCCESS || profile.empty()) {
        HILOGE("HasTrustP2PRelation failed, res: %{public}d", res);
        return false;
    }
    for (auto& item : profile) {
        if (item.GetTrustDeviceId() == deviceId) {
            HILOGI("profile has the trust p2p relationship");
            return true;
        }
    }
    return false;
}

} // namespace DeviceProfile
} // namespace OHOS
