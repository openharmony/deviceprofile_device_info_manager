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

#include "event_handler_factory.h"

#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_log.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(EventHandlerFactory);

namespace {
    const std::string TAG = "EventHandlerFactory";
}

int32_t EventHandlerFactory::Init()
{
    HILOGI("call!");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ != nullptr) {
        return DP_SUCCESS;
    }
    auto runner = AppExecFwk::EventRunner::Create(DP_HANDLER);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    return DP_SUCCESS;
}

int32_t EventHandlerFactory::UnInit()
{
    HILOGI("UnInit");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveAllEvents();
        eventHandler_ = nullptr;
    }
    return DP_SUCCESS;
}

std::shared_ptr<AppExecFwk::EventHandler> EventHandlerFactory::GetEventHandler()
{
    HILOGD("call!");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return eventHandler_;
}
} // namespace DeviceProfile
} // namespace OHOS
