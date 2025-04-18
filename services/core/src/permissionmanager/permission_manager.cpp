/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "permission_manager.h"

#include <fstream>
#include <string>
#include <unordered_set>

#include "accesstoken_kit.h"
#include "file_ex.h"
#include "ipc_skeleton.h"
#include "securec.h"

#include "dp_radar_helper.h"
#include "distributed_device_profile_log.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "macro_utils.h"
#include "profile_utils.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace OHOS::Security::AccessToken;

namespace {
    const std::string TAG = "PermissionManager";
    constexpr uint32_t INVALID_TOKEN_ID = 0;
    const std::string INTERFACES = "interfacesAuthority";
    const std::string SERVICES = "servicesAuthority";
    const std::string SERVICES_ALL = "all";
    const std::string SERVICES_SPECIFIC = "specific";
    const std::string SERVICES_PREFIX = "prefix";
    const std::string PERMISSION_JSON_PATH = "/system/etc/deviceprofile/permission.json";
    const std::string DP_SERVICE_ACCESS_PERMISSION = "ohos.permission.ACCESS_SERVICE_DP";
    const std::string DP_SERVICE_SYNC_PERMISSION = "ohos.permission.SYNC_PROFILE_DP";
}

IMPLEMENT_SINGLE_INSTANCE(PermissionManager);

int32_t PermissionManager::Init()
{
    HILOGI("call!");
    if (!LoadPermissionCfg(PERMISSION_JSON_PATH)) {
        return false;
    }
    HILOGI("init succeeded");
    return DP_SUCCESS;
}

int32_t PermissionManager::UnInit()
{
    HILOGI("UnInit");
    std::lock_guard<std::mutex> lockGuard(permissionMutex_);
    permissionMap_.clear();
    return DP_SUCCESS;
}

int32_t PermissionManager::LoadPermissionCfg(const std::string& filePath)
{
    char path[PATH_MAX + 1] = {0x00};
    if (filePath.length() == 0 || filePath.length() > PATH_MAX || realpath(filePath.c_str(), path) == nullptr) {
        HILOGE("File canonicalization failed");
        return DP_PARSE_PERMISSION_JSON_FAIL;
    }
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        HILOGE("load json file failed");
        return DP_PARSE_PERMISSION_JSON_FAIL;
    }
    std::string fileContent(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{});
    ifs.close();

    cJSON* permissionJson = cJSON_Parse(fileContent.c_str());
    if (!cJSON_IsObject(permissionJson)) {
        HILOGE("Permission json parse failed!");
        cJSON_Delete(permissionJson);
        return DP_PARSE_PERMISSION_JSON_FAIL;
    }
    int32_t parseResult = ParsePermissionJson(permissionJson);
    HILOGI("permission json load result %{public}d!", parseResult);
    cJSON_Delete(permissionJson);
    return parseResult;
}

int32_t PermissionManager::ParsePermissionJson(const cJSON* const permissionJson)
{
    int size = cJSON_GetArraySize(permissionJson);
    if (size == 0 || size > MAX_INTERFACE_SIZE) {
        HILOGE("Permission json size is invalid!size: %{public}d!", size);
        return DP_PARSE_PERMISSION_JSON_FAIL;
    }
    SetRdbPermissionMap(permissionJson);
    SetKVPermissionMap(permissionJson);
    char* jsonChars = cJSON_PrintUnformatted(permissionJson);
    if (jsonChars == NULL) {
        HILOGW("cJSON formatted to string failed!");
    } else {
        HILOGD("permission json %{public}s parse success!", jsonChars);
        cJSON_free(jsonChars);
    }
    return DP_SUCCESS;
}

void PermissionManager::SetRdbPermissionMap(const cJSON *const permissionJson)
{
    int size = cJSON_GetArraySize(permissionJson);
    if (size == 0 || size > MAX_INTERFACE_SIZE) {
        HILOGE("Permission json size is invalid!size: %{public}d!", size);
        return;
    }
    SetPermissionMap(permissionJson, PUT_SERVICE_INFO_PROFILE);
    SetPermissionMap(permissionJson, DELETE_SERVICE_INFO_PROFILE);
    SetPermissionMap(permissionJson, UPDATE_SERVICE_INFO_PROFILE);
    SetPermissionMap(permissionJson, GET_SERVICE_INFO_PROFILE_BY_UNIQUE_KEY);
    SetPermissionMap(permissionJson, GET_SERVICE_INFO_PROFILE_LIST_BY_TOKEN_ID);
    SetPermissionMap(permissionJson, GET_ALL_SERVICE_INFO_PROFILE_LIST);
    SetPermissionMap(permissionJson, GET_SERVICE_INFO_PROFILE_LIST_BY_BUNDLE_NAME);
    SetPermissionMap(permissionJson, PUT_LOCAL_SERVICE_INFO);
    SetPermissionMap(permissionJson, UPDATE_LOCAL_SERVICE_INFO);
    SetPermissionMap(permissionJson, GET_LOCAL_SERVICE_INFO_BY_BINDLE_AND_PINTYPE);
    SetPermissionMap(permissionJson, DELETE_LOCAL_SERVICE_INFO);
    SetPermissionMap(permissionJson, PUT_ACCESS_CONTROL_PROFILE);
    SetPermissionMap(permissionJson, UPDATE_ACCESS_CONTROL_PROFILE);
    SetPermissionMap(permissionJson, GET_ACCESS_CONTROL_PROFILE);
    SetPermissionMap(permissionJson, GET_ALL_ACCESS_CONTROL_PROFILE);
    SetPermissionMap(permissionJson, GET_ALL_ACL_INCLUDE_LNN_ACL);
    SetPermissionMap(permissionJson, DELETE_ACCESS_CONTROL_PROFILE);
    SetPermissionMap(permissionJson, GET_TRUST_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, GET_ALL_TRUST_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, PUT_DEVICE_PROFILE_BATCH);
    SetPermissionMap(permissionJson, DELETE_DEVICE_PROFILE_BATCH);
    SetPermissionMap(permissionJson, GET_DEVICE_PROFILES);
    SetPermissionMap(permissionJson, PUT_PRODUCT_INFO_BATCH);
    SetPermissionMap(permissionJson, PUT_DEVICE_ICON_INFO_BATCH);
    SetPermissionMap(permissionJson, GET_DEVICE_ICON_INFOS);
    SetPermissionMap(permissionJson, PUT_SESSION_KEY);
    SetPermissionMap(permissionJson, GET_SESSION_KEY);
    SetPermissionMap(permissionJson, UPDATE_SESSION_KEY);
    SetPermissionMap(permissionJson, DELETE_SESSION_KEY);
    return;
}

void PermissionManager::SetKVPermissionMap(const cJSON *const permissionJson)
{
    int size = cJSON_GetArraySize(permissionJson);
    if (size == 0 || size > MAX_INTERFACE_SIZE) {
        HILOGE("Permission json size is invalid!size: %{public}d!", size);
        return;
    }
    SetPermissionMap(permissionJson, PUT_SERVICE_PROFILE);
    SetPermissionMap(permissionJson, PUT_SERVICE_PROFILE_BATCH);
    SetPermissionMap(permissionJson, PUT_CHARACTERISTIC_PROFILE);
    SetPermissionMap(permissionJson, PUT_CHARACTERISTIC_PROFILE_BATCH);
    SetPermissionMap(permissionJson, GET_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, GET_SERVICE_PROFILE);
    SetPermissionMap(permissionJson, GET_CHARACTERISTIC_PROFILE);
    SetPermissionMap(permissionJson, DELETE_SERVICE_PROFILE);
    SetPermissionMap(permissionJson, DELETE_CHARACTERISTIC_PROFILE);
    SetPermissionMap(permissionJson, SUBSCRIBE_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, UNSUBSCRIBE_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, SYNC_DEVICE_PROFILE);
    SetPermissionMap(permissionJson, PUT_ALL_TRUSTED_DEVICES);
    return;
}

bool PermissionManager::CheckInterfacePermission(const std::string& interfaceName)
{
    std::string callProcName = GetCallerProcName();
    std::unordered_set<std::string> permittedProcNames;
    {
        std::lock_guard<std::mutex> lockGuard(permissionMutex_);
        permittedProcNames = permissionMap_[interfaceName];
    }
    bool checkResult =  (permittedProcNames.count(callProcName) != 0 || permittedProcNames.count(ALL_PROC) != 0);
    HILOGD("success interface %{public}s callProc %{public}s!", interfaceName.c_str(), callProcName.c_str());
    return checkResult;
}

bool PermissionManager::IsCallerTrust(const std::string& interfaceName)
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_FAIL);
    auto tokenID = IPCSkeleton::GetCallingTokenID();
    if (tokenID == INVALID_TOKEN_ID) {
        HILOGW("invalid token id");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenID);
    // currently only support native trusted caller
    if (tokenType != ATokenTypeEnum::TOKEN_NATIVE) {
        HILOGE("TokenType is not native");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    if (!CheckInterfacePermission(interfaceName)) {
        HILOGE("This caller cannot call this interface, interfaceName: %{public}s", interfaceName.c_str());
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
    return true;
}

bool PermissionManager::CheckCallerPermission()
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_FAIL);
    auto tokenID = IPCSkeleton::GetCallingTokenID();
    if (tokenID == INVALID_TOKEN_ID) {
        HILOGW("invalid token id");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenID);
    if (tokenType != ATokenTypeEnum::TOKEN_NATIVE) {
        HILOGE("TokenType is not native");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    std::string callProcName = GetCallerProcName();
    int32_t ret = AccessTokenKit::VerifyAccessToken(tokenID, DP_SERVICE_ACCESS_PERMISSION);
    if (ret != PermissionState::PERMISSION_GRANTED) {
        HILOGE("failed callProc %{public}s!", callProcName.c_str());
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
    return true;
}

bool PermissionManager::CheckCallerSyncPermission()
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_FAIL);
    auto tokenID = IPCSkeleton::GetCallingTokenID();
    if (tokenID == INVALID_TOKEN_ID) {
        HILOGW("invalid token id");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenID);
    if (tokenType != ATokenTypeEnum::TOKEN_NATIVE) {
        HILOGE("TokenType is not native");
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    std::string callProcName = GetCallerProcName();
    int32_t ret = AccessTokenKit::VerifyAccessToken(tokenID, DP_SERVICE_SYNC_PERMISSION);
    if (ret != PermissionState::PERMISSION_GRANTED) {
        HILOGE("failed callProc %{public}s!", callProcName.c_str());
        DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
        return false;
    }
    stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    DpRadarHelper::GetInstance().ReportSaCheckAuth(stageRes);
    HILOGI("success callProc %{public}s!", callProcName.c_str());
    return true;
}

std::string PermissionManager::GetCallerProcName()
{
    // called after CheckCallerTrust, and keep same policy with CheckCallerTrust
    NativeTokenInfo nativeTokenInfo;
    auto tokenID = IPCSkeleton::GetCallingTokenID();
    auto errCode = AccessTokenKit::GetNativeTokenInfo(tokenID, nativeTokenInfo);
    std::string procName;
    if (errCode == EOK) {
        procName = std::move(nativeTokenInfo.processName);
        HILOGD("procName:%{public}s", procName.c_str());
    }
    return procName;
}

void PermissionManager::SetPermissionMap(const cJSON* const permissionJson, const std::string& interfaceName)
{
    cJSON* item = cJSON_GetObjectItem(permissionJson, interfaceName.c_str());
    int32_t itemSize = static_cast<int32_t>(cJSON_GetArraySize(item));
    if (!cJSON_IsArray(item) || itemSize == 0 || itemSize > MAX_INTERFACE_SIZE) {
        HILOGE("PermissionJson not contains the key, %{public}s!", interfaceName.c_str());
        return;
    }
    std::unordered_set<std::string> interfaceNameSets;
    item = item->child;
    while (item != NULL) {
        if (cJSON_IsString(item)) {
            interfaceNameSets.emplace(item->valuestring);
        }
        item = item->next;
    }
    {
        std::lock_guard<std::mutex> lockGuard(permissionMutex_);
        permissionMap_[interfaceName] = interfaceNameSets;
    }
}
} // namespace DeviceProfile
} // namespace OHOS
