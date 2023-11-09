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

#include "device_profile_load_callback.h"

#include "distributed_device_profile_log.h"
#include "distributed_device_profile_client.h"

namespace OHOS { class IRemoteObject; }
namespace OHOS {
namespace DistributedDeviceProfile {

const std::string TAG = "DeviceProfileLoadCallback";

void DeviceProfileLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    DistributedDeviceProfileClient::GetInstance().LoadSystemAbilitySuccess(remoteObject);
    HILOGI("on load system ability success!");
}

void DeviceProfileLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DistributedDeviceProfileClient::GetInstance().LoadSystemAbilityFail();
    HILOGI("on load system ability failed!");
}
}
}