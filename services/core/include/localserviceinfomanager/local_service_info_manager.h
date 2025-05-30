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

#ifndef OHOS_DP_LOCAL_SERVICE_INFO_MANAGER_H
#define OHOS_DP_LOCAL_SERVICE_INFO_MANAGER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "kvstore_observer.h"
#include "values_bucket.h"

#include "local_service_info.h"
#include "local_service_info_rdb_adapter.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace OHOS::NativeRdb;
class LocalServiceInfoManager {
    DECLARE_SINGLE_INSTANCE(LocalServiceInfoManager);
public:
    int32_t Init();
    int32_t UnInit();
    int32_t PutLocalServiceInfo(const LocalServiceInfo& localServiceInfo);
    int32_t UpdateLocalServiceInfo(const LocalServiceInfo& localServiceInfo);
    int32_t GetLocalServiceInfoByBundleAndPinType(const std::string& bundleName,
        int32_t pinExchangeType, LocalServiceInfo& localServiceInfo);
    int32_t DeleteLocalServiceInfo(const std::string& bundleName, int32_t pinExchangeType);
private:
    int32_t ConvertToLocalServiceInfo(std::shared_ptr<ResultSet> resultSet, LocalServiceInfo& localServiceInfo);
    int32_t LocalServiceInfoToEntries(const LocalServiceInfo& localServiceInfo, ValuesBucket& values);
    int32_t CreateTable();
    int32_t CreateIndex();

private:
};
} // DistributedDeviceProfile
} // OHOS
#endif // OHOS_DP_SERVICE_INFO_PROFILE_MANAGER_H
