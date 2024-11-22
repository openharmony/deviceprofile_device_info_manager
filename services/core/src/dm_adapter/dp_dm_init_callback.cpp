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

#include "dp_dm_init_callback.h"

#include "distributed_device_profile_log.h"
#include "dm_adapter.h"

namespace OHOS {
namespace DistributedDeviceProfile {
namespace {
    const std::string TAG = "DpDmInitCallback";
}
void DpDmInitCallback::OnRemoteDied()
{
    HILOGI("call!");
    DMAdapter::GetInstance().ReInit();
}
} // namespace DistributedDeviceProfile
} // namespace OHOS