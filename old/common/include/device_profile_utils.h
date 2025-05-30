/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_PROFILE_PROFILE_UTILS_H
#define OHOS_DEVICE_PROFILE_PROFILE_UTILS_H

#include <iosfwd>
#include <list>

#include "parcel.h"
#include "profile_event.h"

namespace OHOS {
namespace DeviceProfile {
class DeviceProfileUtils {
public:
    static bool WriteProfileEvents(const std::list<ProfileEvent>& profileEvents, Parcel& parcel);
    static bool ReadProfileEvents(Parcel& parcel, std::list<ProfileEvent>& profileEvents);
    static std::string AnonymizeDeviceId(const std::string& deviceId);
    static std::string AnonymizeString(const std::string& value);
};
} // namespace DeviceProfile
} // namespace OHOS
#endif // OHOS_DEVICE_PROFILE_PROFILE_UTILS_H
