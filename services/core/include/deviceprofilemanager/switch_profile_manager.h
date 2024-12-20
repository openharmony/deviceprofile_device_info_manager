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

#ifndef OHOS_DP_SWITCH_PROFILE_MANAGER_H
#define OHOS_DP_SWITCH_PROFILE_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "characteristic_profile.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class SwitchProfileManager {
DECLARE_SINGLE_INSTANCE(SwitchProfileManager);

public:
    int32_t Init();
    int32_t UnInit();
    int32_t ReInit();
    int32_t PutCharacteristicProfile(const CharacteristicProfile& charProfile);
    int32_t PutCharacteristicProfileBatch(const std::vector<CharacteristicProfile>& charProfiles);
    int32_t GetCharacteristicProfile(const std::string& deviceId, const std::string& serviceName,
        const std::string& characteristicKey, CharacteristicProfile& charProfile);
    int32_t RefreshLocalSwitchProfile();
    int32_t GetLocalSwitchFromDB(uint32_t& localSwitch, uint32_t& switchLength);
private:
    int32_t GenerateSwitchInfoProfile(const CharacteristicProfile& switchProfile,
        std::unordered_map<std::string, CharacteristicProfile> switchProfileMap);
    std::mutex switchProfileMutex_;
};
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif // OHOS_DP_SWITCH_PROFILE_MANAGER_H
