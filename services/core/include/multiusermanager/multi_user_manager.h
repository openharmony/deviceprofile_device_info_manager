/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DP_MULTIPLE_USER_CONNECTOR_H
#define OHOS_DP_MULTIPLE_USER_CONNECTOR_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "iremote_object.h"
#include "distributed_device_profile_constants.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class MultiUserManager {
    DECLARE_SINGLE_INSTANCE(MultiUserManager);
public:
    int32_t Init();

    int32_t UnInit();

    int32_t GetCurrentForegroundUserID();

    void SetCurrentForegroundUserID(int32_t userId);

    int32_t GetForegroundUserIDFromOs(int32_t& foregroundId);

private:
    int32_t foregroundUserId_ = DEFAULT_USER_ID;
    std::mutex foregroundUserIdLock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DP_MULTIPLE_USER_CONNECTOR_H