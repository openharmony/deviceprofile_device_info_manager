/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "distributed_device_profile_client.h"

#include <algorithm>
#include <bitset>
#include <chrono>
#include <functional>
#include <new>
#include <string>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <profile_utils.h>
#include "profile_change_listener_stub.h"
#include "device_profile_load_callback.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_log.h"
#include "event_handler.h"
#include "event_runner.h"
#include "i_distributed_device_profile.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iremote_broker.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace std::chrono_literals;

namespace {
    const std::string TAG = "DistributedDeviceProfileClient";
    constexpr int32_t LOAD_SA_TIMEOUT_MS = 10000;
}

IMPLEMENT_SINGLE_INSTANCE(DistributedDeviceProfileClient);

sptr<IDistributedDeviceProfile> DistributedDeviceProfileClient::LoadDeviceProfileService()
{
    sptr<DeviceProfileLoadCallback> loadCallback = new DeviceProfileLoadCallback();
    if (loadCallback == nullptr) {
        HILOGE("loadCallback is nullptr.");
        return nullptr;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("get samgr failed");
        return nullptr;
    }

    int32_t ret = samgrProxy->LoadSystemAbility(DISTRIBUTED_DEVICE_PROFILE_SA_ID, loadCallback);
    if (ret != ERR_OK) {
        HILOGE("Failed to Load systemAbility");
        return nullptr;
    }
    {
        std::unique_lock<std::mutex> lock(serviceLock_);
        auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() { return dpProxy_ != nullptr; });
        if (waitStatus && dpProxy_ != nullptr) {
            HILOGE("Get profile Service success!");
            return dpProxy_;
        }
    }
    return nullptr;
}

void DistributedDeviceProfileClient::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    HILOGI("DistributedDeviceProfileClient FinishStartSA");
    std::lock_guard<std::mutex> lock(serviceLock_);
    if (dpDeathRecipient_ == nullptr) {
        dpDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DeviceProfileDeathRecipient);
    }
    if (remoteObject != nullptr) {
        remoteObject->AddDeathRecipient(dpDeathRecipient_);
        dpProxy_ = iface_cast<IDistributedDeviceProfile>(remoteObject);
        proxyConVar_.notify_one();
    }
}

void DistributedDeviceProfileClient::LoadSystemAbilityFail()
{
    std::lock_guard<std::mutex> lock(serviceLock_);
    dpProxy_ = nullptr;
}

void DistributedDeviceProfileClient::SendSubscribeInfosToService()
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("dpService is nullptr!");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(serviceLock_);
        if (subscribeInfos_.empty() || subscribeInfos_.size() > MAX_SUBSCRIBE_INFO_SIZE) {
            HILOGE("SubscribeInfos size is invalid!");
            return;
        }
        dpService->SendSubscribeInfos(subscribeInfos_);
    }
}

int32_t DistributedDeviceProfileClient::PutAccessControlProfile(const AccessControlProfile& accessControlProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->PutAccessControlProfile(accessControlProfile);
}

int32_t DistributedDeviceProfileClient::UpdateAccessControlProfile(const AccessControlProfile& accessControlProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->UpdateAccessControlProfile(accessControlProfile);
}

int32_t DistributedDeviceProfileClient::GetTrustDeviceProfile(const std::string& deviceId,
    TrustDeviceProfile& trustDeviceProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetTrustDeviceProfile(deviceId, trustDeviceProfile);
}

int32_t DistributedDeviceProfileClient::GetAllTrustDeviceProfile(std::vector<TrustDeviceProfile>& trustDeviceProfiles)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetAllTrustDeviceProfile(trustDeviceProfiles);
}

int32_t DistributedDeviceProfileClient::GetAccessControlProfile(std::map<std::string, std::string> params,
    std::vector<AccessControlProfile>& accessControlProfiles)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    if (params.empty() || params.size() > MAX_PARAM_SIZE) {
        HILOGE("Params size is invalid!");
        return DP_INVALID_PARAMS;
    }
    return dpService->GetAccessControlProfile(params, accessControlProfiles);
}

int32_t DistributedDeviceProfileClient::GetAllAccessControlProfile(
    std::vector<AccessControlProfile>& accessControlProfiles)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetAllAccessControlProfile(accessControlProfiles);
}

int32_t DistributedDeviceProfileClient::DeleteAccessControlProfile(int32_t accessControlId)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->DeleteAccessControlProfile(accessControlId);
}

int32_t DistributedDeviceProfileClient::PutServiceProfile(const ServiceProfile& serviceProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->PutServiceProfile(serviceProfile);
}

int32_t DistributedDeviceProfileClient::PutServiceProfileBatch(const std::vector<ServiceProfile>& serviceProfiles)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    if (serviceProfiles.empty() || serviceProfiles.size() > MAX_PROFILE_SIZE) {
        HILOGE("ServiceProfiles size is invalid!");
        return DP_INVALID_PARAMS;
    }
    return dpService->PutServiceProfileBatch(serviceProfiles);
}

int32_t DistributedDeviceProfileClient::PutCharacteristicProfile(const CharacteristicProfile& characteristicProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->PutCharacteristicProfile(characteristicProfile);
}

int32_t DistributedDeviceProfileClient::PutCharacteristicProfileBatch(
    const std::vector<CharacteristicProfile>& characteristicProfiles)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    if (characteristicProfiles.empty()  || characteristicProfiles.size() > MAX_PROFILE_SIZE) {
        HILOGE("ServiceProfiles size is invalid!");
        return DP_INVALID_PARAMS;
    }
    return dpService->PutCharacteristicProfileBatch(characteristicProfiles);
}

int32_t DistributedDeviceProfileClient::GetDeviceProfile(const std::string& deviceId, DeviceProfile& deviceProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetDeviceProfile(deviceId, deviceProfile);
}

int32_t DistributedDeviceProfileClient::GetServiceProfile(const std::string& deviceId, const std::string& serviceName,
    ServiceProfile& serviceProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetServiceProfile(deviceId, serviceName, serviceProfile);
}

int32_t DistributedDeviceProfileClient::GetCharacteristicProfile(const std::string& deviceId,
    const std::string& serviceName, const std::string& characteristicId, CharacteristicProfile& characteristicProfile)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->GetCharacteristicProfile(deviceId, serviceName, characteristicId, characteristicProfile);
}

int32_t DistributedDeviceProfileClient::DeleteServiceProfile(const std::string& deviceId,
    const std::string& serviceName)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->DeleteServiceProfile(deviceId, serviceName);
}

int32_t DistributedDeviceProfileClient::DeleteCharacteristicProfile(const std::string& deviceId,
    const std::string& serviceName, const std::string& characteristicKey)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    return dpService->DeleteCharacteristicProfile(deviceId, serviceName, characteristicKey);
}

int32_t DistributedDeviceProfileClient::SubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    auto dpService = LoadDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(serviceLock_);
        if (subscribeInfos_.size() > MAX_LISTENER_SIZE) {
            HILOGE("ProfileChangeListeners size is invalid!");
            return DP_EXCEED_MAX_SIZE_FAIL;
        }
        std::string subscribeTag =
            subscribeInfo.GetSubscribeKey() + SEPARATOR + std::to_string(subscribeInfo.GetSaId());
        subscribeInfos_[subscribeTag] = subscribeInfo;
    }
    return dpService->SubscribeDeviceProfile(subscribeInfo);
}

int32_t DistributedDeviceProfileClient::UnSubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    auto dpService = LoadDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(serviceLock_);
        subscribeInfos_.erase(subscribeInfo.GetSubscribeKey() + SEPARATOR + std::to_string(subscribeInfo.GetSaId()));
    }
    return dpService->UnSubscribeDeviceProfile(subscribeInfo);
}

int32_t DistributedDeviceProfileClient::SyncDeviceProfile(const SyncOptions& syncOptions,
    sptr<ISyncCompletedCallback> syncCb)
{
    auto dpService = GetDeviceProfileService();
    if (dpService == nullptr) {
        HILOGE("Get dp service failed");
        return DP_GET_SERVICE_FAILED;
    }
    if (syncCb == nullptr) {
        HILOGE("SyncCb is nullptr!");
        return DP_SYNC_DEVICE_FAIL;
    }
    sptr<IRemoteObject> syncCompletedCallback = syncCb->AsObject();
    if (syncCompletedCallback == nullptr) {
        HILOGE("SyncCb ipc cast fail!");
        return DP_SYNC_DEVICE_FAIL;
    }
    return dpService->SyncDeviceProfile(syncOptions, syncCompletedCallback);
}

sptr<IDistributedDeviceProfile> DistributedDeviceProfileClient::GetDeviceProfileService()
{
    {
        std::lock_guard<std::mutex> lock(serviceLock_);
        if (dpProxy_ != nullptr) {
            return dpProxy_;
        }
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            HILOGE("get samgr failed");
            return nullptr;
        }
        auto object = samgrProxy->CheckSystemAbility(DISTRIBUTED_DEVICE_PROFILE_SA_ID);
        if (object != nullptr) {
            HILOGI("get service succeeded");
            if (dpDeathRecipient_ == nullptr) {
                dpDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DeviceProfileDeathRecipient);
            }
            object->AddDeathRecipient(dpDeathRecipient_);
            dpProxy_ = iface_cast<IDistributedDeviceProfile>(object);
            return dpProxy_;
        }
    }

    HILOGW("remoteObject is null!");
    if (LoadDeviceProfileService()) {
        std::lock_guard<std::mutex> lock(serviceLock_);
        if (dpProxy_ != nullptr) {
            return dpProxy_;
        } else {
            HILOGE("load dp service failed");
            return nullptr;
        }
    }
    HILOGE("load dp service failed");
    return nullptr;
}

void DistributedDeviceProfileClient::OnServiceDied(const sptr<IRemoteObject>& remote)
{
    HILOGI("called");
    std::lock_guard<std::mutex> lock(serviceLock_);
    dpProxy_ = nullptr;
}

void DistributedDeviceProfileClient::DeviceProfileDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DistributedDeviceProfileClient::GetInstance().OnServiceDied(remote.promote());
}
} // namespace DeviceProfile
} // namespace OHOS
