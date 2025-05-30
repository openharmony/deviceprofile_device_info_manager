/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_PROFILE_CONTENT_COLLECTOR_H
#define OHOS_DEVICE_PROFILE_CONTENT_COLLECTOR_H

#include "service_characteristic_profile.h"

namespace OHOS {
namespace DeviceProfile {
class ContentCollector {
public:
    ContentCollector() = default;
    virtual ~ContentCollector() = default;
    virtual bool ConvertToProfileData(ServiceCharacteristicProfile& profile) = 0;

    void DoCollect(ServiceCharacteristicProfile& profileData);
};
} // namespace DeviceProfile
} // namespace OHOS
#endif // OHOS_DEVICE_PROFILE_CONTENT_COLLECTOR_H