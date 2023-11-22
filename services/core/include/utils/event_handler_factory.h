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

#ifndef OHOS_DP_EVENT_HANDLER_FACTORY_H
#define OHOS_DP_EVENT_HANDLER_FACTORY_H

#include <map>
#include <memory>
#include <mutex>
#include "event_handler.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class EventHandlerFactory {
DECLARE_SINGLE_INSTANCE(EventHandlerFactory);

public:
    int32_t Init();
    int32_t UnInit();
    std::shared_ptr<AppExecFwk::EventHandler> CreateEventHandler(const std::string& handlerName);

private:
    std::mutex eventHandlerMutex_;
    std::map<std::string, std::shared_ptr<AppExecFwk::EventHandler>> eventHandlerMap_;
};
} // namespace DeviceProfile
} // namespace OHOS
#endif // OHOS_DP_EVENT_HANDLER_FACTORY_H