/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "listener/kv_data_change_listener.h"

#include <algorithm>
#include <cinttypes>

#include "datetime_ex.h"
#include "string_ex.h"

#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "device_profile_manager.h"
#include "profile_utils.h"
#include "profile_cache.h"
#include "subscribe_profile_manager.h"
#include "subscribe_profile_manager.h"
#include "distributed_device_profile_log.h"

#include "types.h"

namespace OHOS {
namespace DistributedDeviceProfile {
namespace {
    const std::string TAG = "KvDataChangeListener";
    const std::string STATIC_STORE_ID = "dp_kv_static_store";
}

KvDataChangeListener::KvDataChangeListener(const std::string& storeId)
{
    HILOGD("construct!");
    storeId_ = storeId;
}

KvDataChangeListener::~KvDataChangeListener()
{
    HILOGD("destruct!");
}

void KvDataChangeListener::OnChange(const DistributedKv::ChangeNotification& changeNotification)
{
    HILOGI("storeId=%{public}s", storeId_.c_str());
    if (storeId_ == STATIC_STORE_ID) {
        return;
    }
    if (!changeNotification.GetInsertEntries().empty() &&
        changeNotification.GetInsertEntries().size() <= MAX_DB_RECORD_SIZE) {
        HandleAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty() &&
        changeNotification.GetUpdateEntries().size() <= MAX_DB_RECORD_SIZE) {
        HandleUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty() &&
        changeNotification.GetDeleteEntries().size() <= MAX_DB_RECORD_SIZE) {
        HandleDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void KvDataChangeListener::OnChange(const DistributedKv::DataOrigin& origin, Keys&& keys)
{
    HILOGI("Cloud data Change. store=%{public}s", origin.store.c_str());
    if (origin.store == STATIC_STORE_ID) {
        return;
    }
    std::vector<DistributedKv::Entry> insertRecords = DeviceProfileManager::GetInstance()
        .GetEntriesByKeys(keys[ChangeOp::OP_INSERT]);
    if (!insertRecords.empty() && insertRecords.size() <= MAX_DB_RECORD_SIZE) {
        HandleAddChange(insertRecords);
    }
    std::vector<DistributedKv::Entry> updateRecords = DeviceProfileManager::GetInstance()
        .GetEntriesByKeys(keys[ChangeOp::OP_UPDATE]);
    if (!updateRecords.empty() && updateRecords.size() <= MAX_DB_RECORD_SIZE) {
        HandleUpdateChange(updateRecords);
    }
    std::vector<std::string> delKeys = keys[ChangeOp::OP_DELETE];
    if (!delKeys.empty() && delKeys.size() <= MAX_DB_RECORD_SIZE) {
        std::vector<DistributedKv::Entry> deleteRecords;
        for (const auto& key : delKeys) {
            DistributedKv::Entry entry;
            DistributedKv::Key kvKey(key);
            entry.key = kvKey;
            deleteRecords.emplace_back(entry);
        }
        HandleDeleteChange(deleteRecords);
    }
}

void KvDataChangeListener::OnSwitchChange(const DistributedKv::SwitchNotification& notification)
{
    HILOGI("Switch data change, deviceId: %{public}s", ProfileUtils::GetAnonyString(notification.deviceId).c_str());
    if (notification.deviceId.empty()) {
        HILOGE("params are valid");
        return;
    }
    // is local or is online
    std::string netWorkId = notification.deviceId;
    std::string udid;
    int32_t res = ProfileCache::GetInstance().GetUdidByNetWorkId(netWorkId, udid);
    if (res != DP_SUCCESS || udid.empty()) {
        HILOGD("get udid fail, netWorkId is invalid: %{public}s",
            ProfileUtils::GetAnonyString(netWorkId).c_str());
        return;
    }
    HandleSwitchUpdateChange(udid, notification.data.value);
}

void KvDataChangeListener::FilterEntries(const std::vector<DistributedKv::Entry>& records,
    std::map<std::string, std::string>& entriesMap, bool isDelete)
{
    std::map<std::string, std::string> ohSuffix2NonMaps;
    std::map<std::string, std::string> non2OhSuffixMaps;
    for (const auto& item : records) {
        std::string dbKey = item.key.ToString();
        std::vector<std::string> res;
        if (ProfileUtils::SplitString(dbKey, SEPARATOR, res) != DP_SUCCESS || res.size() < NUM_3) {
            HILOGW("invalid dbkey(%{public}s)", ProfileUtils::GetDbKeyAnonyString(dbKey).c_str());
            continue;
        }
        if (res[0] != DEV_PREFIX && res[0] != SVR_PREFIX && res[0] != CHAR_PREFIX) {
            HILOGW("%{public}s is invalid dbKey", ProfileUtils::GetDbKeyAnonyString(dbKey).c_str());
            continue;
        }
        if (res[0] == CHAR_PREFIX && res.size() > NUM_4 && res[NUM_4] == CHARACTERISTIC_KEY) {
            HILOGD("%{public}s is charProfileKey", ProfileUtils::GetDbKeyAnonyString(dbKey).c_str());
            continue;
        }
        entriesMap[dbKey] = item.value.ToString();
        if (ProfileUtils::EndsWith(res[NUM_2], OH_PROFILE_SUFFIX)) {
            res[NUM_2] = ProfileUtils::CheckAndRemoveOhSuffix(res[NUM_2]);
            ohSuffix2NonMaps[dbKey] = ProfileUtils::JoinString(res, SEPARATOR);
            continue;
        }
        if (ProfileUtils::IsNeedAddOhSuffix(res[NUM_2], res.front() == SVR_PREFIX)) {
            res[NUM_2] = ProfileUtils::CheckAndAddOhSuffix(res[NUM_2], res.front() == SVR_PREFIX);
            non2OhSuffixMaps[dbKey] = ProfileUtils::JoinString(res, SEPARATOR);
            continue;
        }
    }
    for (const auto& [ohSuffixKey, nonOhSuffixKey] : ohSuffix2NonMaps) {
        entriesMap.erase(nonOhSuffixKey);
        non2OhSuffixMaps.erase(nonOhSuffixKey);
    }
    ohSuffix2NonMaps.clear();
    if (isDelete) { return; }
    if (non2OhSuffixMaps.empty()) { return; }
    std::vector<std::string> ohSuffixKeys;
    for (const auto& [nonOhSuffixKey, ohSuffixKey] : non2OhSuffixMaps) {
        ohSuffix2NonMaps[ohSuffixKey] = nonOhSuffixKey;
        ohSuffixKeys.emplace_back(ohSuffixKey);
    }
    std::vector<DistributedKv::Entry> entries = DeviceProfileManager::GetInstance().GetEntriesByKeys(ohSuffixKeys);
    if (entries.empty()) { return; }
    for (const auto& item : entries) {
        entriesMap.erase(ohSuffix2NonMaps[item.key.ToString()]);
    }
}

void KvDataChangeListener::HandleAddChange(const std::vector<DistributedKv::Entry>& insertRecords)
{
    HILOGD("Handle kv data add change!");
    std::map<std::string, std::string> entries;
    FilterEntries(insertRecords, entries, false);
    for (const auto& [dbKey, dbValue] : entries) {
        ProfileType profileType = ProfileUtils::GetProfileType(dbKey);
        SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, ChangeType::ADD, dbKey, dbValue);
    }
}

void KvDataChangeListener::HandleUpdateChange(const std::vector<DistributedKv::Entry>& updateRecords)
{
    HILOGD("Handle kv data update change!");
    std::map<std::string, std::string> entries;
    FilterEntries(updateRecords, entries, false);
    for (const auto& [dbKey, dbValue] : entries) {
        ProfileType profileType = ProfileUtils::GetProfileType(dbKey);
        SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, ChangeType::UPDATE, dbKey, dbValue);
    }
}

void KvDataChangeListener::HandleDeleteChange(const std::vector<DistributedKv::Entry>& deleteRecords)
{
    HILOGD("Handle kv data delete change!");
    std::map<std::string, std::string> entries;
    FilterEntries(deleteRecords, entries, true);
    for (const auto& [dbKey, dbValue] : entries) {
        ProfileType profileType = ProfileUtils::GetProfileType(dbKey);
        SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, ChangeType::DELETE, dbKey, dbValue);
    }
}

void KvDataChangeListener::HandleSwitchUpdateChange(const std::string udid, uint32_t switchValue)
{
    HILOGI("udid: %{public}s, switch: %{public}u", ProfileUtils::GetAnonyString(udid).c_str(), switchValue);
    std::string serviceName;

    for (int32_t i = (int32_t)SwitchFlag::SWITCH_FLAG_MIN + NUM_1;
        i < (int32_t)SwitchFlag::SWITCH_FLAG_MAX; ++i) {
        std::string itemSwitchValue = std::to_string((switchValue >> i) & NUM_1);
        int32_t res = ProfileCache::GetInstance().GetServiceNameByPos(i, SWITCH_SERVICE_MAP, serviceName);
        if (res != DP_SUCCESS || serviceName.empty()) {
            HILOGE("GetServiceNameByPos failed, pos:%{public}d", i);
            return;
        }
        res = GenerateSwitchNotify(udid, serviceName, SWITCH_STATUS,
        itemSwitchValue, ChangeType::UPDATE);
        if (res != DP_SUCCESS) {
            HILOGE("GenerateSwitchNotify failed, res: %{public}d", res);
            return;
        }
        if (udid == ProfileCache::GetInstance().GetLocalUdid()) {
            ProfileCache::GetInstance().SetCurSwitch(switchValue);
            HILOGD("update curLocalSwitch: %{public}d", ProfileCache::GetInstance().GetSwitch());
        }
    }
}

int32_t KvDataChangeListener::GenerateSwitchNotify(const std::string& udid, const std::string& serviceName,
    const std::string& characteristicProfileKey, const std::string& characteristicProfileValue, ChangeType changeType)
{
    if (!ProfileUtils::IsKeyValid(udid) ||
        !ProfileUtils::IsKeyValid(serviceName) ||
        !ProfileUtils::IsKeyValid(characteristicProfileKey) ||
        !ProfileUtils::IsKeyValid(characteristicProfileValue)) {
        HILOGE("Params are invalid!");
        return DP_INVALID_PARAMS;
    }

    CharacteristicProfile newSwitchProfile = {udid, serviceName, characteristicProfileKey,
        characteristicProfileValue};
    HILOGI("Gen SwitchProfile :%{public}s", newSwitchProfile.dump().c_str());
    if (ProfileCache::GetInstance().IsCharProfileExist(newSwitchProfile)) {
        HILOGW("switch is not change");
        return DP_SUCCESS;
    }
    const CharacteristicProfile cacheProfile = newSwitchProfile;
    ProfileCache::GetInstance().AddCharProfile(cacheProfile);
    std::string dbKey = ProfileUtils::GetDbKeyByProfile(newSwitchProfile);
    ProfileType profileType = ProfileUtils::GetProfileType(dbKey);
    int32_t res = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey,
        newSwitchProfile.GetCharacteristicValue());
    if (res != DP_SUCCESS) {
        HILOGE("NotifyProfileChange failed");
        return DP_GENERATE_SWITCH_NOTIFY_FAIL;
    }
    return DP_SUCCESS;
}
} // namespace DeviceProfile
} // namespace OHOS
