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

#ifndef OHOS_DP_PERMISSION_MANAGER_H
#define OHOS_DP_PERMISSION_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "cJSON.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class PermissionManager {
DECLARE_SINGLE_INSTANCE(PermissionManager);

public:
    int32_t Init();
    int32_t UnInit();
    bool IsCallerTrust(const std::string& interfaceName);
    bool CheckCallerPermission();
    bool CheckCallerSyncPermission();
    std::string GetCallerProcName();

private:
    bool CheckInterfacePermission(const std::string& interfaceName);
    int32_t LoadPermissionCfg(const std::string& filePath);
    int32_t ParsePermissionJson(const cJSON* const permissionJson);
    void SetRdbPermissionMap(const cJSON* const permissionJson);
    void SetKVPermissionMap(const cJSON* const permissionJson);
    void SetPermissionMap(const cJSON* const permissionJson, const std::string& interfaceName);

private:
    std::mutex permissionMutex_;
    std::unordered_map<std::string, std::unordered_set<std::string>> permissionMap_;
};
} // namespace DeviceProfile
} // namespace OHOS
#endif // OHOS_DP_PERMISSION_MANAGER_H
