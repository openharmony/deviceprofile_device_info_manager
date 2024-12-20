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

#include "content_sensor_manager.h"

#include "app_info_collector.h"
#include "device_info_collector.h"
#include "device_profile_log.h"
#include "hitrace_meter.h"
#include "pasteboard_info_collector.h"
#include "storage_info_collector.h"
#include "syscap_info_collector.h"
#include "system_info_collector.h"

namespace OHOS {
namespace DeviceProfile {
namespace {
const std::string TAG = "ContentSensorManager";
const std::string DP_CONTENT_SENSOR_TRACE = "DP_CONTENT_SENSOR";
}

IMPLEMENT_SINGLE_INSTANCE(ContentSensorManager);

bool ContentSensorManager::Init()
{
    return true;
}

bool ContentSensorManager::Collect()
{
    auto csTask = []() {
        HILOGI("ContentSensorManager Collect");
        std::list<std::shared_ptr<ContentCollector>> taskList;
        taskList.push_back(std::make_shared<DeviceInfoCollector>());
        taskList.push_back(std::make_shared<SystemInfoCollector>());
        taskList.push_back(std::make_shared<SyscapInfoCollector>());
        taskList.push_back(std::make_shared<StorageInfoCollector>());
        taskList.push_back(std::make_shared<AppInfoCollector>());
        taskList.push_back(std::make_shared<PasteboardInfoCollector>());
        HITRACE_METER_NAME(HITRACE_TAG_DEVICE_PROFILE, DP_CONTENT_SENSOR_TRACE);
        for (auto& task : taskList) {
            ServiceCharacteristicProfile profileData;
            if (task == nullptr || !task->ConvertToProfileData(profileData)) {
                continue;
            }
            task->DoCollect(profileData);
        }
    };
    if (csCollectorHandler_ == nullptr || !csCollectorHandler_->PostTask(csTask)) {
        HILOGE("post task failed");
        return false;
    }
    return true;
}
} // namespace DeviceProfile
} // namespace OHOS