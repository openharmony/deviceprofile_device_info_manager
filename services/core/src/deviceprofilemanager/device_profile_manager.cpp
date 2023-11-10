/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <mutex>
#include <memory>
#include <algorithm>
#include "device_profile_manager.h"
#include "kv_adapter.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_log.h"
#include "profile_utils.h"
#include "profile_cache.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(DeviceProfileManager);
namespace {
    const std::string APP_ID = "distributed_device_profile_service";
    const std::string STORE_ID = "dp_kv_store";
    const std::string TAG = "DeviceProfileManager";
    const std::string DP_MANAGER_HANDLER = "dp_manager_handler";
}

int32_t DeviceProfileManager::Init()
{
    HILOGI("call!");
    int32_t initResult = DP_MANAGER_INIT_FAIL;
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        deviceProfileStore_ = std::make_shared<KVAdapter>(APP_ID, STORE_ID, std::make_shared<KvDataChangeListener>(),
            std::make_shared<KvSyncCompletedListener>(), std::make_shared<KvDeathRecipient>());
        initResult = deviceProfileStore_->Init();
    }
    HILOGI("Init finish, res: %d", initResult);
    return initResult;
}

int32_t DeviceProfileManager::UnInit()
{
    HILOGI("call!");
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        deviceProfileStore_->UnInit();
        deviceProfileStore_ = nullptr;
    }
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
    if (!ProfileUtils::IsKeyValid(deviceProfile.GetDeviceId())) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("PutDeviceProfile, deviceId: %s!", ProfileUtils::GetAnonyString(deviceProfile.GetDeviceId()).c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().IsDeviceProfileExist(deviceProfile)) {
        HILOGI("the profile is exist!");
        return DP_CACHE_EXIST;
    }
    std::map<std::string, std::string> entries;
    ProfileUtils::DeviceProfileToEntries(deviceProfile, entries);
    if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
        HILOGE("PutDeviceProfile fail!");
        return DP_PUT_KV_DB_FAIL;
    }
    ProfileCache::GetInstance().AddDeviceProfile(deviceProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutServiceProfile(const ServiceProfile& serviceProfile)
{
    if (!ProfileUtils::IsKeyValid(serviceProfile.GetDeviceId()) ||
        !ProfileUtils::IsKeyValid(serviceProfile.GetServiceName())) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("PutServiceProfile, deviceId: %s, serviceName: %s!",
        ProfileUtils::GetAnonyString(serviceProfile.GetDeviceId()).c_str(), serviceProfile.GetServiceName().c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().IsServiceProfileExist(serviceProfile)) {
        HILOGW("the profile is exist!");
        return DP_CACHE_EXIST;
    }
    std::map<std::string, std::string> entries;
    ProfileUtils::ServiceProfileToEntries(serviceProfile, entries);
    if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
        HILOGE("PutServiceProfile fail!");
        return DP_PUT_KV_DB_FAIL;
    }
    ProfileCache::GetInstance().AddServiceProfile(serviceProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutServiceProfileBatch(const std::vector<ServiceProfile>& serviceProfiles)
{
    HILOGE("PutServiceProfileBatch call!");
    for (const auto& serviceProfile : serviceProfiles) {
        PutServiceProfile(serviceProfile);
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutCharacteristicProfile(const CharacteristicProfile& charProfile)
{
    if (!ProfileUtils::IsKeyValid(charProfile.GetDeviceId()) ||
        !ProfileUtils::IsKeyValid(charProfile.GetServiceName()) ||
        !ProfileUtils::IsKeyValid(charProfile.GetCharacteristicKey())) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("PutCharacteristicProfile, deviceId: %s, serviceName: %s, charKey: %s!",
        ProfileUtils::GetAnonyString(charProfile.GetDeviceId()).c_str(), charProfile.GetServiceName().c_str(),
        charProfile.GetCharacteristicKey().c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().IsCharProfileExist(charProfile)) {
        HILOGW("the profile is exist!");
        return DP_CACHE_EXIST;
    }
    std::map<std::string, std::string> entries;
    ProfileUtils::CharacteristicProfileToEntries(charProfile, entries);
    if (deviceProfileStore_->PutBatch(entries) != DP_SUCCESS) {
        HILOGE("PutCharacteristicProfile fail!");
        return DP_PUT_KV_DB_FAIL;
    }
    ProfileCache::GetInstance().AddCharProfile(charProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::PutCharacteristicProfileBatch(const std::vector<CharacteristicProfile>& charProfiles)
{
    HILOGE("PutCharacteristicProfileBatch call!");
    for (const auto& charProfile : charProfiles) {
        PutCharacteristicProfile(charProfile);
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetDeviceProfile(const std::string& deviceId, DeviceProfile& deviceProfile)
{
    if (!ProfileUtils::IsKeyValid(deviceId)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("GetDeviceProfile, deviceId: %s!", ProfileUtils::GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().GetDeviceProfile(deviceId, deviceProfile) == DP_SUCCESS) {
        HILOGI("GetDeviceProfile in cache!");
        return DP_SUCCESS;
    }
    std::string dbKeyPrefix = ProfileUtils::GenerateDeviceProfileKey(deviceId);
    std::map<std::string, std::string> values;
    if (deviceProfileStore_->GetByPrefix(dbKeyPrefix, values) != DP_SUCCESS) {
        HILOGE("Get data fail!");
        return DP_GET_KV_DB_FAIL;
    }
    HILOGI("GetDeviceProfile in db!");
    ProfileUtils::EntriesToDeviceProfile(values, deviceProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetServiceProfile(const std::string& deviceId, const std::string& serviceName,
    ServiceProfile& serviceProfile)
{
    if (!ProfileUtils::IsKeyValid(deviceId) || !ProfileUtils::IsKeyValid(serviceName)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("PutDeviceProfile, deviceId: %s, serviceName: %s!", ProfileUtils::GetAnonyString(deviceId).c_str(),
        serviceName.c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().GetServiceProfile(deviceId, serviceName, serviceProfile) == DP_SUCCESS) {
        HILOGI("GetServiceProfile in cache!");
        return DP_SUCCESS;
    }
    std::string dbKeyPrefix = ProfileUtils::GenerateServiceProfileKey(deviceId, serviceName);
    std::map<std::string, std::string> values;
    if (deviceProfileStore_->GetByPrefix(dbKeyPrefix, values) != DP_SUCCESS) {
        HILOGE("Get data fail!");
        return DP_GET_KV_DB_FAIL;
    }
    serviceProfile.SetDeviceId(deviceId);
    serviceProfile.SetServiceName(serviceName);
    ProfileUtils::EntriesToServiceProfile(values, serviceProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
    const std::string& characteristicKey, CharacteristicProfile& charProfile)
{
    if (!ProfileUtils::IsKeyValid(deviceId) || !ProfileUtils::IsKeyValid(serviceName) ||
        !ProfileUtils::IsKeyValid(characteristicKey)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("GetCharacteristicProfile, deviceId: %s, serviceName: %s, charKey: %s!",
        ProfileUtils::GetAnonyString(deviceId).c_str(), serviceName.c_str(), characteristicKey.c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    if (ProfileCache::GetInstance().GetCharacteristicProfile(deviceId, serviceName, characteristicKey, charProfile)
        == DP_SUCCESS) {
        HILOGI("GetCharProfile in cache!");
        return DP_SUCCESS;
    }
    std::string profileKeyPrefix = ProfileUtils::GenerateCharProfileKey(deviceId, serviceName, characteristicKey);
    std::map<std::string, std::string> values;
    if (deviceProfileStore_->GetByPrefix(profileKeyPrefix, values) != DP_SUCCESS) {
        HILOGE("Get data fail!");
        return DP_GET_KV_DB_FAIL;
    }
    charProfile.SetDeviceId(deviceId);
    charProfile.SetServiceName(serviceName);
    charProfile.SetCharacteristicKey(characteristicKey);
    ProfileUtils::EntriesToCharProfile(values, charProfile);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::DeleteServiceProfile(const std::string& deviceId, const std::string& serviceName)
{
    if (!ProfileUtils::IsKeyValid(deviceId) || !ProfileUtils::IsKeyValid(serviceName)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("DeleteServiceProfile, deviceId: %s, serviceName: %s!", ProfileUtils::GetAnonyString(deviceId).c_str(),
        serviceName.c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    std::string profileKeyPrefix = ProfileUtils::GenerateServiceProfileKey(deviceId, serviceName);
    if (deviceProfileStore_->DeleteByPrefix(profileKeyPrefix) != DP_SUCCESS) {
        HILOGE("DeleteServiceProfile fail!");
        return DP_DEL_KV_DB_FAIL;
    }
    ProfileCache::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::DeleteCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
    const std::string& characteristicKey)
{
    if (!ProfileUtils::IsKeyValid(deviceId) || !ProfileUtils::IsKeyValid(serviceName) ||
        !ProfileUtils::IsKeyValid(characteristicKey)) {
        HILOGE("the profile is invalid!");
        return DP_INVALID_PARAMS;
    }
    HILOGI("DeleteCharacteristicProfile, deviceId: %s, serviceName: %s, charKey: %s!",
        ProfileUtils::GetAnonyString(deviceId).c_str(), serviceName.c_str(), characteristicKey.c_str());
    std::lock_guard<std::mutex> lock(dpStoreMutex_);
    if (deviceProfileStore_ == nullptr) {
        HILOGE("deviceProfileStore is nullptr!");
        return DP_INVALID_PARAMS;
    }
    std::string profileKeyPrefix = ProfileUtils::GenerateCharProfileKey(deviceId, serviceName, characteristicKey);
    if (deviceProfileStore_->DeleteByPrefix(profileKeyPrefix) != DP_SUCCESS) {
        HILOGE("DeleteCharacteristicProfile fail!");
        return DP_DEL_KV_DB_FAIL;
    }
    ProfileCache::GetInstance().DeleteCharProfile(deviceId, serviceName, characteristicKey);
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllDeviceProfile(std::vector<DeviceProfile>& deviceProfiles)
{
    HILOGI("call!");
    std::map<std::string, std::string> values;
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_INVALID_PARAMS;
        }
        if (deviceProfileStore_->GetByPrefix(DEV_PREFIX, values) != DP_SUCCESS) {
            HILOGE("Get data fail!");
            return DP_GET_KV_DB_FAIL;
        }
    }
    std::map<std::string, std::map<std::string, std::string>> profileEntries;
    for (const auto& item : values) {
        std::string dbKey = item.first;
        std::string dbValue = item.second;
        std::string profileKey = ProfileUtils::GetProfileKey(dbKey);
        profileEntries[profileKey].emplace(dbKey, dbValue);
    }
    for (const auto& item : profileEntries) {
        DeviceProfile deviceProfile;
        ProfileUtils::EntriesToDeviceProfile(item.second, deviceProfile);
        deviceProfiles.push_back(deviceProfile);
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllServiceProfile(std::vector<ServiceProfile>& serviceProfiles)
{
    HILOGI("call!");
    std::map<std::string, std::string> values;
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_INVALID_PARAMS;
        }
        if (deviceProfileStore_->GetByPrefix(SVR_PREFIX, values) != DP_SUCCESS) {
            HILOGE("Get data fail!");
            return DP_GET_KV_DB_FAIL;
        }
    }
    std::map<std::string, std::map<std::string, std::string>> profileEntries;
    for (const auto& item: values) {
        std::string dbKey = item.first;
        std::string dbValue = item.second;
        std::string profileKey = ProfileUtils::GetProfileKey(dbKey);
        profileEntries[profileKey].emplace(dbKey, dbValue);
    }
    for (const auto &item: profileEntries) {
        ServiceProfile serviceProfile;
        ProfileUtils::EntriesToServiceProfile(item.second, serviceProfile);
        serviceProfiles.push_back(serviceProfile);
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::GetAllCharacteristicProfile(std::vector<CharacteristicProfile>& charProfiles)
{
    HILOGI("call!");
    std::map<std::string, std::string> values;
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        if (deviceProfileStore_ == nullptr) {
            HILOGE("deviceProfileStore is nullptr!");
            return DP_INVALID_PARAMS;
        }
        if (deviceProfileStore_->GetByPrefix(CHAR_PREFIX, values) != DP_SUCCESS) {
            HILOGE("Get data fail!");
            return DP_GET_KV_DB_FAIL;
        }
    }
    std::map<std::string, std::map<std::string, std::string>> profileEntries;
    for (auto item: values) {
        std::string dbKey = item.first;
        std::string dbValue = item.second;
        std::string profileKey = ProfileUtils::GetProfileKey(dbKey);
        profileEntries[profileKey].emplace(dbKey, dbValue);
    }
    for (const auto& item: profileEntries) {
        CharacteristicProfile charProfile;
        ProfileUtils::EntriesToCharProfile(item.second, charProfile);
        charProfiles.push_back(charProfile);
    }
    return DP_SUCCESS;
}

int32_t DeviceProfileManager::SyncDeviceProfile(const SyncOptions& syncOptions,
    sptr<IRemoteObject> syncCompletedCallback)
{
    HILOGI("call!");
    if (syncCompletedCallback == nullptr) {
        HILOGE("Params is invalid!");
        return DP_INVALID_PARAMS;
    }
    std::u16string callerDescriptor = syncCompletedCallback->GetObjectDescriptor();
    ProfileCache::GetInstance().AddSyncListener(callerDescriptor, syncCompletedCallback);
    HILOGI("SyncDeviceProfile start!");
    std::vector<std::string> onlineDevices = ProfileUtils::FilterOnlineDevices(syncOptions.GetDeviceList());
    if (onlineDevices.empty()) {
        HILOGE("Params is invalid!");
        return DP_INVALID_PARAMS;
    }
    {
        std::lock_guard<std::mutex> lock(dpStoreMutex_);
        int32_t syncResult = deviceProfileStore_->Sync(onlineDevices, syncOptions.GetSyncMode());
        if (syncResult != DP_SUCCESS) {
            HILOGI("SyncDeviceProfile fail, res: %d!", syncResult);
            return DP_SYNC_DEVICE_FAIL;
        }
    }
    HILOGI("SyncDeviceProfile success, caller: %s!", ProfileUtils::toString(callerDescriptor).c_str());
    return DP_SUCCESS;
}
} // namespace DeviceProfile
} // namespace OHOS