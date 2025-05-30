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

#ifndef OHOS_DP_DEVICE_PROFILE_MANAGER_H
#define OHOS_DP_DEVICE_PROFILE_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "dm_device_info.h"
#include "iremote_object.h"

#include "characteristic_profile.h"
#include "device_profile.h"
#include "dp_sync_options.h"
#include "i_dp_sync_adapter.h"
#include "i_sync_completed_callback.h"
#include "ikv_adapter.h"
#include "kv_data_change_listener.h"
#include "kv_store_death_recipient.h"
#include "kv_sync_completed_listener.h"
#include "service_profile.h"
#include "single_instance.h"
#include "trusted_device_info.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class DeviceProfileManager {
DECLARE_SINGLE_INSTANCE(DeviceProfileManager);

public:
    int32_t Init();
    int32_t UnInit();
    int32_t ReInit();
    int32_t PutDeviceProfile(const DeviceProfile& deviceProfile);
    int32_t PutServiceProfile(const ServiceProfile& serviceProfile);
    int32_t PutServiceProfileBatch(const std::vector<ServiceProfile>& serviceProfiles);
    int32_t PutCharacteristicProfile(const CharacteristicProfile& charProfile);
    int32_t PutCharacteristicProfileBatch(const std::vector<CharacteristicProfile>& charProfiles);
    int32_t GetDeviceProfile(const std::string& deviceId, DeviceProfile& deviceProfile);
    int32_t GetServiceProfile(const std::string& deviceId, const std::string& serviceName,
        ServiceProfile& serviceProfile);
    int32_t GetCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
        const std::string& characteristicKey, CharacteristicProfile& charProfile);
    int32_t DeleteServiceProfile(const std::string& deviceId, const std::string& serviceName, bool isMultiUser = false,
        int32_t userId = DEFAULT_USER_ID);
    int32_t DeleteCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
        const std::string& characteristicKey, bool isMultiUser = false, int32_t userId = DEFAULT_USER_ID);
    int32_t DeleteRemovedUserData(int32_t userId);
    int32_t GetAllDeviceProfile(std::vector<DeviceProfile>& deviceProfiles);
    int32_t GetAllServiceProfile(std::vector<ServiceProfile>& serviceProfiles);
    int32_t GetAllCharacteristicProfile(std::vector<CharacteristicProfile>& charProfiles);
    int32_t SyncDeviceProfile(const DistributedDeviceProfile::DpSyncOptions& syncOptions,
        sptr<IRemoteObject> syncCompletedCallback);
    std::vector<DistributedKv::Entry> GetEntriesByKeys(const std::vector<std::string>& keys);
    int32_t SavePutTempCache(std::map<std::string, std::string>& entries);
    bool IsFirstInitDB();
    void ResetFirst();
    void OnDeviceOnline(const TrustedDeviceInfo& trustedDeviceInfo);
    void OnUserChange(int32_t lastUserId, int32_t curUserId);

private:
    bool LoadDpSyncAdapter();
    void UnloadDpSyncAdapter();
    int32_t RunloadedFunction(const std::string& deviceId, sptr<IRemoteObject> syncCompletedCallback);
    int32_t SyncWithNotOHBasedDevice(const std::vector<std::string>& notOHBasedDevices,
        const std::string& callerDescriptor, sptr<IRemoteObject> syncCompletedCallback);
    void SyncWithNotOHBasedDeviceFailed(const std::vector<std::string>& notOHBasedDevices,
        sptr<IRemoteObject> syncCompletedCallback);
    void AddToPutTempCache(const std::map<std::string, std::string>& values);
    void FixDataOnDeviceOnline(const TrustedDeviceInfo& deviceInfo);
    int32_t DeleteBatchByKeys(const std::vector<std::string>& delKeys);
    int32_t GetProfilesByOwner(const std::string& uuid, std::map<std::string, std::string>& entries);
    int32_t GetProfilesByKeyPrefix(const std::string& udid, std::map<std::string, std::string>& entries);
    // Clean data that does not belong to the local.
    void FixLocalData(const std::string& localUdid, const std::map<std::string, std::string>& localDataByOwner);
    // Clean ohbase data when the peer is non-ohbase
    void FixRemoteDataWhenPeerIsNonOH(const std::string& remoteUdid);
    // Clean non-ohbase data when the peer is ohbase
    void FixRemoteDataWhenPeerIsOHBase(const std::string& remoteUdid,
        const std::map<std::string, std::string>& localDataByOwner);
    void NotifyNotOHBaseOnline(const TrustedDeviceInfo& deviceInfo);
    void E2ESyncDynamicProfile(const TrustedDeviceInfo& deviceInfo);
    int32_t SaveBatchByKeys(const std::map<std::string, std::string>& entries);
    void FixDiffProfiles();
    template <typename T>
    int32_t IsMultiUserValid(const T& profile);
    bool IsSameAccount(const std::string deviceId, const int32_t userId);
    bool HasTrustP2PRelation(const std::string deviceId, const int32_t userId);
    bool isAdapterSoLoaded_ = false;
    std::mutex isAdapterLoadLock_;
    std::mutex dynamicStoreMutex_;
    std::shared_ptr<IKVAdapter> deviceProfileStore_ = nullptr;
    std::shared_ptr<IDPSyncAdapter> dpSyncAdapter_;
    std::atomic<bool> isFirst_{true};
    std::mutex putTempCacheMutex_;
    std::map<std::string, std::string> putTempCache_;
};
} // namespace DeviceProfile
} // namespace OHOS
#endif // OHOS_DP_DEVICE_PROFILE_MANAGER_H
