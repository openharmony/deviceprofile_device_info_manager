/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DP_I_PINCODE_INVALID_CALLBCK_H
#define OHOS_DP_I_PINCODE_INVALID_CALLBCK_H

#include <cstdint>
#include "iremote_broker.h"
#include "distributed_device_profile_enums.h"
#include "local_service_info.h"

namespace OHOS {
namespace DistributedDeviceProfile {

class IPincodeInvalidCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DeviceProfile.IPincodeInvalidCallback");
    IPincodeInvalidCallback() = default;
    virtual ~IPincodeInvalidCallback() = default;
    virtual int32_t OnPincodeInvalid(const LocalServiceInfo& localServiceInfo) = 0;
};
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif // OHOS_DP_I_PINCODE_INVALID_CALLBCK_H
