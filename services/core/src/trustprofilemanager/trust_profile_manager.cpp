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

#include "trust_profile_manager.h"
#include "cJSON.h"
#include "subscribe_profile_manager.h"
#include "distributed_device_profile_log.h"
#include "rdb_adapter.h"
#include "profile_utils.h"
#include "device_manager.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "accesser.h"
#include "accessee.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(TrustProfileManager);
namespace {
    const std::string TAG = "TrustProfileManager";
    const std::string NAME = "name";
}

int32_t TrustProfileManager::Init()
{
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        rdbStore_ = std::make_shared<RdbAdapter>();
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_INIT_DB_FAILED;
        }
        int32_t ret = rdbStore_->Init();
        if (ret != DP_SUCCESS) {
            HILOGE("rdbStore_ Init failed");
            return DP_INIT_DB_FAILED;
        }
    }
    this->CreateTable();
    this->CreateUniqueIndex();
    if (!IsAceeCreIdExistToAceeTable() && AddAceeCreIdColumnToAceeTable() != DP_SUCCESS) {
        HILOGE("acee table add aceeCreId failed");
        return DP_CREATE_TABLE_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UnInit()
{
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->UnInit();
        if (ret != DP_SUCCESS) {
            HILOGE("rdbStore_ Uninit failed");
            return DP_UNINIT_FAIL;
        }
        rdbStore_ = nullptr;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutTrustDeviceProfile(const TrustDeviceProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::TrustDeviceProfileToEntries(profile, values);
    int64_t rowId = ROWID_INIT;
    int32_t ret = RET_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Put(rowId, TRUST_DEVICE_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("trust_device_table insert failed");
            return DP_PUT_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccessControlProfile(const AccessControlProfile& profile)
{
    AccessControlProfile accessControlProfile(profile);
    bool isExists = false;
    {
        std::lock_guard<std::mutex> lock(aclMutex_);
        CheckDeviceIdAndUserIdExists(profile, isExists);
        int32_t ret = this->SetAccessControlProfileId(accessControlProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("SetAccessControlProfileId failed");
            return ret;
        }
        ret = this->PutAccesserProfile(accessControlProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("PutAccesserProfile failed");
            return ret;
        }
        ret = this->PutAccesseeProfile(accessControlProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("PutAccesseeProfile failed");
            return ret;
        }
        if (IsAclExists(accessControlProfile) == DP_DATA_EXISTS) {
            HILOGE("acl is exists");
            return DP_SUCCESS;
        }
        ValuesBucket values;
        ProfileUtils::AccessControlProfileToEntries(accessControlProfile, values);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int64_t rowId = ROWID_INIT;
        if (rdbStore_->Put(rowId, ACCESS_CONTROL_TABLE, values) != DP_SUCCESS) {
            HILOGE("access_control_table insert failed");
            return DP_PUT_ACL_PROFILE_FAIL;
        }
        HILOGI("PutAclProfile : %{public}s", accessControlProfile.dump().c_str());
    }
    int32_t putRet = this->PutAclCheck(accessControlProfile, isExists);
    if (putRet != DP_SUCCESS) {
        HILOGE("PutAclCheck failed");
        return putRet;
    }
    DistributedHardware::DeviceManager::GetInstance().DpAclAdd(accessControlProfile.GetAccessControlId(),
        accessControlProfile.GetTrustDeviceId(), accessControlProfile.GetBindType());
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateTrustDeviceProfile(const TrustDeviceProfile& profile)
{
    std::string deviceId = profile.GetDeviceId();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = resultSet->GoToFirstRow();
    if (ret != DP_SUCCESS) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    TrustDeviceProfile oldProfile;
    ProfileUtils::ConvertToTrustDeviceProfile(resultSet, oldProfile);
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::TrustDeviceProfileToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Update(changeRowCnt, TRUST_DEVICE_TABLE, values, DEVICEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetDeviceId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("Update trust_device_table failed");
            return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    this->UpdateTrustDeviceProfileNotify(oldProfile, profile);
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccessControlProfile(const AccessControlProfile& profile)
{
    AccessControlProfile oldProfile;
    {
        std::lock_guard<std::mutex> lock(aclMutex_);
        int32_t ret = this->UpdateAclCheck(profile, oldProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("UpdateAclCheck faild");
            return ret;
        }
        this->UpdateAccesserProfile(profile);
        this->UpdateAccesseeProfile(profile);
        ValuesBucket values;
        ProfileUtils::AccessControlProfileToEntries(profile, values);
        int32_t changeRowCnt = CHANGEROWCNT_INIT;
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Update(changeRowCnt, ACCESS_CONTROL_TABLE, values, ACCESSCONTROLID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("update access_control_table failed");
            return DP_UPDATE_ACL_PROFILE_FAIL;
        }
    }
    this->NotifyCheck(profile, oldProfile);
    HILOGI("UpdateAclProfile : %{public}s", profile.dump().c_str());
    int32_t status = STATUS_INIT;
    this->GetResultStatus(profile.GetTrustDeviceId(), status);
    TrustDeviceProfile trustProfile;
    ProfileUtils::ConvertToTrustDeviceProfile(profile, trustProfile);
    trustProfile.SetStatus(status);
    int32_t updateRet = this->UpdateTrustDeviceProfile(trustProfile);
    if (updateRet != DP_SUCCESS) {
        HILOGE("UpdateTrustDeviceProfile failed");
        return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetTrustDeviceProfile(const std::string& deviceId, TrustDeviceProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = resultSet->GoToFirstRow();
    if (ret != DP_SUCCESS) {
        HILOGE("not find trust device data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    ProfileUtils::ConvertToTrustDeviceProfile(resultSet, profile);
    resultSet->Close();
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAllTrustDeviceProfile(std::vector<TrustDeviceProfile>& profile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trust_device_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        TrustDeviceProfile trustProfile;
        ProfileUtils::ConvertToTrustDeviceProfile(resultSet, trustProfile);
        profile.push_back(trustProfile);
    }
    resultSet->Close();
    if (profile.empty()) {
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, const std::string& bundleName,
    int32_t bindType, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN) {
        HILOGE("bundleName is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, bundleName : %{public}s, bindtype : %{public}d, status : %{public}d",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(), bundleName.c_str(), bindType, status);
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_BINDTYPE_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(bindType), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("bindType and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByUserIdAndBundleName(resultSet, userId, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByUserIdAndBundleName faild");
        return DP_NOT_FIND_DATA;
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by userId bundleName bindType status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, const std::string& bundleName,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN || trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("bundleName or trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, bundleName : %{public}s, trustDeviceId : %{public}s, status : %{public}d",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(),
        bundleName.c_str(), ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(trustDeviceId), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByUserIdAndBundleName(resultSet, userId, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByUserIdAndBundleName faild");
        return ret;
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by userId bundleName trustDeviceId status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfileByTokenId(int64_t tokenId,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, tokenId : %{public}" PRId64 ", trustDeviceId : %{public}s, status : %{public}d",
        tokenId, ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_STATUS,
        std::vector<ValueObject>{ ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByTokenId(resultSet, trustDeviceId, tokenId, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByTokenId faild");
        return ret;
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("tokenId not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId,
    const std::string& accountId, std::vector<AccessControlProfile>& profile)
{
    if (accountId.size() > MAX_STRING_LEN) {
        HILOGE("accountId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, accountId : %{public}s",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(),
        ProfileUtils::GetAnonyString(accountId).c_str());
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = this->GetAclProfileByUserIdAndAccountId(
            resultSet, accesserId, accesseeId, userId, accountId, profile);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAclProfileByUserIdAndAccountId faild");
            return ret;
        }
    }
    resultSet->Close();
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by userId accountId not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGD("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, std::vector<AccessControlProfile> &profile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = GetAccessControlProfiles(resultSet, accesserId, accesseeId, userId, profile);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAccessControlProfile faild");
            resultSet->Close();
            return ret;
        }
    }
    resultSet->Close();
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by userId not find data, userId: %{public}s",
            ProfileUtils::GetAnonyString(std::to_string(userId)).c_str());
        return DP_NOT_FIND_DATA;
    }
    HILOGD("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const QueryType& queryType,
    const QueryProfile& queryProfile, std::vector<AccessControlProfile>& profile)
{
    std::vector<AccessControlProfile> aclProfiles;
    int32_t ret = GetAllAccessControlProfiles(aclProfiles);
    if (ret != DP_SUCCESS) {
        HILOGE("GetAllAccessControlProfiles faild");
        return ret;
    }
    if (queryType == QueryType::ACER_AND_ACEE_TOKENID) {
        GetAclByAcerAndAceeTokenId(queryProfile, aclProfiles, profile);
    }
    if (queryType == QueryType::ACER_TOKENID) {
        GetAclByAcerTokenId(queryProfile, aclProfiles, profile);
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by userId and tokenId not find data, queryProfile : %{public}s", queryProfile.dump().c_str());
        return DP_NOT_FIND_DATA;
    }
    HILOGI("profile size : %{public}zu", profile.size());
    return DP_SUCCESS;
}

void TrustProfileManager::GetAclByAcerTokenId(const QueryProfile& queryProfile,
    const std::vector<AccessControlProfile>& aclProfiles, std::vector<AccessControlProfile>& profile)
{
    for (auto aclProfile : aclProfiles) {
        if (aclProfile.GetTrustDeviceId() != queryProfile.GetAccesseeDeviceId() &&
            aclProfile.GetTrustDeviceId() != queryProfile.GetAccesserDeviceId()) {
            continue;
        }
        if (CheckForWardByAcer(queryProfile, aclProfile) || CheckReverseByAcer(queryProfile, aclProfile)) {
            profile.emplace_back(aclProfile);
        }
    }
    return;
}

void TrustProfileManager::GetAclByAcerAndAceeTokenId(const QueryProfile& queryProfile,
    const std::vector<AccessControlProfile>& aclProfiles, std::vector<AccessControlProfile>& profile)
{
    for (auto aclProfile : aclProfiles) {
        if (aclProfile.GetTrustDeviceId() != queryProfile.GetAccesseeDeviceId() &&
            aclProfile.GetTrustDeviceId() != queryProfile.GetAccesserDeviceId()) {
            continue;
        }
        if (CheckForWardByAcerAndAcee(queryProfile, aclProfile) ||
            CheckReverseByAcerAndAcee(queryProfile, aclProfile)) {
            profile.emplace_back(aclProfile);
        }
    }
    return;
}

int32_t TrustProfileManager::GetAllAccessControlProfile(std::vector<AccessControlProfile>& profiles)
{
    std::lock_guard<std::mutex> lock(aclMutex_);
    int32_t ret = GetAllAccessControlProfiles(profiles);
    if (ret != DP_SUCCESS) {
        HILOGE("GetAllAccessControlProfile failed");
        return ret;
    }
    RemoveLnnAcl(profiles);
    if (profiles.empty()) {
        HILOGE("not find data");
        return DP_NOT_FIND_DATA;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAllAclIncludeLnnAcl(std::vector<AccessControlProfile>& profiles)
{
    std::lock_guard<std::mutex> lock(aclMutex_);
    return GetAllAccessControlProfiles(profiles);
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::string& bundleName,
    int32_t bindType, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN) {
        HILOGE("bundleName is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, bundleName : %{public}s, bindType : %{public}d, status : %{public}d",
        bundleName.c_str(), bindType, status);
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_BINDTYPE_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(bindType), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("bindType and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByBundleName(resultSet, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByBundleName faild");
        return ret;
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by bundleName bindType status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::string& bundleName,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN || trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("bundleName or trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, bundleName : %{public}s, trustDeviceId : %{public}s, status : %{public}d",
        bundleName.c_str(), ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(trustDeviceId), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByBundleName(resultSet, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByBundleName faild");
        return ret;
    }
    RemoveLnnAcl(profile);
    if (profile.empty()) {
        HILOGE("by bundleName trustDeviceId status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::map<std::string, std::string>& params,
    std::vector<AccessControlProfile>& profile)
{
    std::lock_guard<std::mutex> lock(aclMutex_);
    if (params.find(TRUST_DEVICE_ID) != params.end() &&
        ProfileUtils::IsPropertyValid(params, STATUS, INT32_MIN, INT32_MAX)) {
        if (ProfileUtils::IsPropertyValid(params, USERID, INT32_MIN, INT32_MAX) &&
            params.find(BUNDLENAME) != params.end()) {
            return GetAccessControlProfile(std::atoi(params.at(USERID).c_str()),
                params.at(BUNDLENAME), params.at(TRUST_DEVICE_ID), std::atoi(params.at(STATUS).c_str()), profile);
        }
        if (params.find(BUNDLENAME) != params.end()) {
            return GetAccessControlProfile(params.at(BUNDLENAME),
                params.at(TRUST_DEVICE_ID), std::atoi(params.at(STATUS).c_str()), profile);
        }
        if (ProfileUtils::IsPropertyValidInt64(params, TOKENID)) {
            return GetAccessControlProfileByTokenId(std::atoi(params.at(TOKENID).c_str()),
                params.at(TRUST_DEVICE_ID), std::atoi(params.at(STATUS).c_str()), profile);
        }
    }
    if (ProfileUtils::IsPropertyValid(params, BIND_TYPE, INT32_MIN, INT32_MAX) &&
        ProfileUtils::IsPropertyValid(params, STATUS, INT32_MIN, INT32_MAX)) {
        if (ProfileUtils::IsPropertyValid(params, USERID, INT32_MIN, INT32_MAX) &&
            params.find(BUNDLENAME) != params.end()) {
            return GetAccessControlProfile(std::atoi(params.at(USERID).c_str()),
                params.at(BUNDLENAME), std::atoi(params.at(BIND_TYPE).c_str()),
                std::atoi(params.at(STATUS).c_str()), profile);
        }
        if (params.find(BUNDLENAME) != params.end()) {
            return GetAccessControlProfile(params.at(BUNDLENAME), std::atoi(params.at(BIND_TYPE).c_str()),
                std::atoi(params.at(STATUS).c_str()), profile);
        }
    }
    if (ProfileUtils::IsPropertyValid(params, USERID, INT32_MIN, INT32_MAX)) {
        if (params.find(ACCOUNTID) != params.end()) {
            return GetAccessControlProfile(std::atoi(params.at(USERID).c_str()), params.at(ACCOUNTID), profile);
        }
        return GetAccessControlProfile(std::atoi(params.at(USERID).c_str()), profile);
    }
    QueryProfile queryProfile;
    QueryType queryType;
    if (GenerateQueryProfile(params, queryType, queryProfile)) {
        return GetAccessControlProfile(queryType, queryProfile, profile);
    }
    HILOGE("params is error");
    return DP_INVALID_PARAMS;
}

int32_t TrustProfileManager::DeleteTrustDeviceProfile(const std::string& deviceId)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    resultSet->GoToFirstRow();
    TrustDeviceProfile profile;
    ProfileUtils::ConvertToTrustDeviceProfile(resultSet, profile);
    resultSet->Close();
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        int32_t ret = rdbStore_->Delete(deleteRows, TRUST_DEVICE_TABLE, DEVICEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(deviceId) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete trust_device_table data failed");
            return DP_DELETE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccessControlProfile(int64_t accessControlId)
{
    AccessControlProfile profile;
    {
        std::lock_guard<std::mutex> lock(aclMutex_);
        std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSCONTROLID,
            std::vector<ValueObject>{ ValueObject(accessControlId) });
        if (resultSet == nullptr) {
            HILOGE("resultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        int32_t rowCount = ROWCOUNT_INIT;
        resultSet->GetRowCount(rowCount);
        if (rowCount == 0) {
            HILOGE("by accessControlId not find data");
            resultSet->Close();
            return DP_NOT_FIND_DATA;
        }
        if (resultSet->GoToNextRow() != DP_SUCCESS) {
            HILOGE("get AccessControlProfileResult failed");
            resultSet->Close();
            return DP_NOT_FIND_DATA;
        }
        ProfileUtils::ConvertToAccessControlProfile(resultSet, profile);
        resultSet->Close();
        int32_t ret = this->DeleteAccessControlProfileCheck(profile);
        if (ret != DP_SUCCESS) {
            HILOGE("DeleteAccessControlProfileCheck failed");
            return ret;
        }
    }
    int32_t delRet = this->DeleteTrustDeviceCheck(profile);
    if (delRet != DP_SUCCESS) {
        HILOGE("DeleteTrustDeviceCheck failed");
        return delRet;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CreateTable()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return DP_GET_RDBSTORE_FAIL;
    }
    int32_t ret = rdbStore_->CreateTable(CREATE_TURST_DEVICE_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("trust_device_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESS_CONTROL_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("access_control_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSER_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accesser_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSEE_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accessee_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CreateUniqueIndex()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return DP_GET_RDBSTORE_FAIL;
    }
    int32_t ret = rdbStore_->CreateTable(CREATE_TURST_DEVICE_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("trust_device_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESS_CONTROL_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("access_control_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSER_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accesser_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSEE_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accessee_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    return DP_SUCCESS;
}

bool TrustProfileManager::CheckForWardByAcerAndAcee(const QueryProfile& queryProfile,
    const AccessControlProfile& aclProfile)
{
    if (aclProfile.GetAccesser().GetAccesserDeviceId() != queryProfile.GetAccesserDeviceId() ||
        aclProfile.GetAccessee().GetAccesseeDeviceId() != queryProfile.GetAccesseeDeviceId() ||
        aclProfile.GetStatus() != static_cast<int32_t>(Status::ACTIVE)) {
        return false;
    }
    if ((aclProfile.GetAccesser().GetAccesserUserId() != queryProfile.GetAccesserUserId() &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID_EXTRA) ||
        (aclProfile.GetAccessee().GetAccesseeUserId() != queryProfile.GetAccesseeUserId() &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID_EXTRA)) {
        return false;
    }
    if ((aclProfile.GetBindType() == static_cast<uint32_t>(BindType::SAME_ACCOUNT) &&
        aclProfile.GetAccesser().GetAccesserAccountId() == queryProfile.GetAccesserAccountId() &&
        aclProfile.GetAccessee().GetAccesseeAccountId() == queryProfile.GetAccesseeAccountId()) ||
        aclProfile.GetBindLevel() == static_cast<uint32_t>(BindLevel::USER)) {
        return true;
    }
    if (aclProfile.GetAccesser().GetAccesserTokenId() == queryProfile.GetAccesserTokenId() &&
        aclProfile.GetAccessee().GetAccesseeTokenId() == queryProfile.GetAccesseeTokenId()) {
        return true;
    }
    return false;
}

bool TrustProfileManager::CheckForWardByAcer(const QueryProfile& queryProfile,
    const AccessControlProfile& aclProfile)
{
    if (aclProfile.GetAccesser().GetAccesserDeviceId() != queryProfile.GetAccesserDeviceId() ||
        aclProfile.GetAccessee().GetAccesseeDeviceId() != queryProfile.GetAccesseeDeviceId() ||
        aclProfile.GetStatus() != static_cast<int32_t>(Status::ACTIVE)) {
        return false;
    }
    if (aclProfile.GetAccesser().GetAccesserUserId() != queryProfile.GetAccesserUserId() &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID_EXTRA) {
        return false;
    }
    if ((aclProfile.GetBindType() == static_cast<uint32_t>(BindType::SAME_ACCOUNT) &&
        aclProfile.GetAccesser().GetAccesserAccountId() == queryProfile.GetAccesserAccountId()) ||
        aclProfile.GetBindLevel() == static_cast<uint32_t>(BindLevel::USER)) {
        return true;
    }
    if (aclProfile.GetAccesser().GetAccesserTokenId() == queryProfile.GetAccesserTokenId()) {
        return true;
    }
    return false;
}

bool TrustProfileManager::CheckReverseByAcerAndAcee(const QueryProfile& queryProfile,
    const AccessControlProfile& aclProfile)
{
    if (aclProfile.GetAccesser().GetAccesserDeviceId() != queryProfile.GetAccesseeDeviceId() ||
        aclProfile.GetAccessee().GetAccesseeDeviceId() != queryProfile.GetAccesserDeviceId() ||
        aclProfile.GetStatus() != static_cast<int32_t>(Status::ACTIVE)) {
        return false;
    }
    if ((aclProfile.GetAccesser().GetAccesserUserId() != queryProfile.GetAccesseeUserId() &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccesser().GetAccesserUserId() != DEFAULT_USER_ID_EXTRA) ||
        (aclProfile.GetAccessee().GetAccesseeUserId() != queryProfile.GetAccesserUserId() &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID_EXTRA)) {
        return false;
    }
    if ((aclProfile.GetBindType() == static_cast<uint32_t>(BindType::SAME_ACCOUNT) &&
        aclProfile.GetAccesser().GetAccesserAccountId() == queryProfile.GetAccesseeAccountId() &&
        aclProfile.GetAccessee().GetAccesseeAccountId() == queryProfile.GetAccesserAccountId()) ||
        aclProfile.GetBindLevel() == static_cast<uint32_t>(BindLevel::USER)) {
        return true;
    }
    if (aclProfile.GetAccesser().GetAccesserTokenId() == queryProfile.GetAccesseeTokenId() &&
        aclProfile.GetAccessee().GetAccesseeTokenId() == queryProfile.GetAccesserTokenId()) {
        return true;
    }
    return false;
}

bool TrustProfileManager::CheckReverseByAcer(const QueryProfile& queryProfile,
    const AccessControlProfile& aclProfile)
{
    if (aclProfile.GetAccesser().GetAccesserDeviceId() != queryProfile.GetAccesseeDeviceId() ||
        aclProfile.GetAccessee().GetAccesseeDeviceId() != queryProfile.GetAccesserDeviceId() ||
        aclProfile.GetStatus() != static_cast<int32_t>(Status::ACTIVE)) {
        return false;
    }
    if (aclProfile.GetAccessee().GetAccesseeUserId() != queryProfile.GetAccesserUserId() &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID &&
        aclProfile.GetAccessee().GetAccesseeUserId() != DEFAULT_USER_ID_EXTRA) {
        return false;
    }
    if ((aclProfile.GetBindType() == static_cast<uint32_t>(BindType::SAME_ACCOUNT) &&
        aclProfile.GetAccessee().GetAccesseeAccountId() == queryProfile.GetAccesserAccountId()) ||
        aclProfile.GetBindLevel() == static_cast<uint32_t>(BindLevel::USER)) {
        return true;
    }
    if (aclProfile.GetAccessee().GetAccesseeTokenId() == queryProfile.GetAccesserTokenId()) {
        return true;
    }
    return false;
}

bool TrustProfileManager::GenerateQueryProfile(const std::map<std::string, std::string>& params,
    QueryType& queryType, QueryProfile& queryProfile)
{
    if (params.find(ACCESSER_DEVICE_ID) != params.end() && params.find(ACCESSEE_DEVICE_ID) != params.end() &&
        params.find(ACCESSER_ACCOUNT_ID) != params.end() &&
        ProfileUtils::IsPropertyValid(params, ACCESSER_USER_ID, INT32_MIN, INT32_MAX) &&
        ProfileUtils::IsPropertyValidInt64(params, ACCESSER_TOKEN_ID)) {
        if (params.find(ACCESSEE_ACCOUNT_ID) != params.end() &&
            ProfileUtils::IsPropertyValid(params, ACCESSEE_USER_ID, INT32_MIN, INT32_MAX) &&
            ProfileUtils::IsPropertyValidInt64(params, ACCESSEE_TOKEN_ID)) {
            queryProfile.SetAccesserDeviceId(params.at(ACCESSER_DEVICE_ID));
            queryProfile.SetAccesserUserId(std::atoi(params.at(ACCESSER_USER_ID).c_str()));
            queryProfile.SetAccesserAccountId(params.at(ACCESSER_ACCOUNT_ID));
            queryProfile.SetAccesserTokenId(std::atoi(params.at(ACCESSER_TOKEN_ID).c_str()));
            queryProfile.SetAccesseeDeviceId(params.at(ACCESSEE_DEVICE_ID));
            queryProfile.SetAccesseeUserId(std::atoi(params.at(ACCESSEE_USER_ID).c_str()));
            queryProfile.SetAccesseeAccountId(params.at(ACCESSEE_ACCOUNT_ID));
            queryProfile.SetAccesseeTokenId(std::atoi(params.at(ACCESSEE_TOKEN_ID).c_str()));
            queryType = QueryType::ACER_AND_ACEE_TOKENID;
            return true;
        }
        queryProfile.SetAccesserDeviceId(params.at(ACCESSER_DEVICE_ID));
        queryProfile.SetAccesserTokenId(std::atoi(params.at(ACCESSER_TOKEN_ID).c_str()));
        queryProfile.SetAccesserUserId(std::atoi(params.at(ACCESSER_USER_ID).c_str()));
        queryProfile.SetAccesserAccountId(params.at(ACCESSER_ACCOUNT_ID));
        queryProfile.SetAccesseeDeviceId(params.at(ACCESSEE_DEVICE_ID));
        queryType = QueryType::ACER_TOKENID;
        return true;
    }
    return false;
}

int32_t TrustProfileManager::GetAllAccessControlProfiles(std::vector<AccessControlProfile>& profiles)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = this->GetAccessControlProfile(resultSet, accesserId, accesseeId, profiles);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAccessControlProfile faild");
            resultSet->Close();
            return ret;
        }
    }
    resultSet->Close();
    if (profiles.empty()) {
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByUserIdAndBundleName(std::shared_ptr<ResultSet> resultSet,
    int32_t userId, const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t>(BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t>(BindLevel::USER)) {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId, accesseeId, userId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId,
                accesseeId, userId, bundleName, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByUserIdAndAccountId(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, int32_t userId, const std::string& accountId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID_ACCESSERACCOUNTID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId), ValueObject(accountId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSEEID_ACCESSEEACCOUNTID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId), ValueObject(accountId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByTokenId(std::shared_ptr<ResultSet> resultSet,
    const std::string& trustDeviceId, int64_t tokenId, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t> (BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t>(BindLevel::USER)) {
            int32_t ret = this->GetAccessControlProfilesByDeviceId(
                resultSet, accesserId, accesseeId, trustDeviceId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfile failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfilesByTokenId(resultSet, accesserId,
                accesseeId, trustDeviceId, tokenId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfilesByTokenId failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByBundleName(std::shared_ptr<ResultSet> resultSet,
    const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t> (BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t>(BindLevel::USER)) {
            int32_t ret = this->GetAccessControlProfile(resultSet, accesserId, accesseeId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfile failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId, accesseeId, bundleName, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccesserProfile(const AccessControlProfile& profile)
{
    Accesser accesser = profile.GetAccesser();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accesser.GetAccesserDeviceId()), ValueObject(accesser.GetAccesserUserId()),
        ValueObject(accesser.GetAccesserAccountId()), ValueObject(accesser.GetAccesserTokenId()),
        ValueObject(accesser.GetAccesserBundleName()), ValueObject(accesser.GetAccesserHapSignature()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserBindLevel())),
        ValueObject(accesser.GetAccesserDeviceName()), ValueObject(accesser.GetAccesserServiceName()),
        ValueObject(accesser.GetAccesserCredentialIdStr()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserStatus())),
        ValueObject(accesser.GetAccesserSessionKeyId())});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        HILOGI("accesser is exists");
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::AccesserToEntries(profile, values);
    int64_t rowId = ROWID_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Put(rowId, ACCESSER_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("accesser_table insert failed");
            return DP_PUT_ACCESSER_PROFILE_FAIL;
        }
    }
    HILOGI("PutAccesser : %{public}s", profile.GetAccesser().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccesseeProfile(const AccessControlProfile& profile)
{
    Accessee accessee = profile.GetAccessee();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accessee.GetAccesseeDeviceId()), ValueObject(accessee.GetAccesseeUserId()),
        ValueObject(accessee.GetAccesseeAccountId()), ValueObject(accessee.GetAccesseeTokenId()),
        ValueObject(accessee.GetAccesseeBundleName()), ValueObject(accessee.GetAccesseeHapSignature()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeBindLevel())),
        ValueObject(accessee.GetAccesseeDeviceName()), ValueObject(accessee.GetAccesseeServiceName()),
        ValueObject(accessee.GetAccesseeCredentialIdStr()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeStatus())),
        ValueObject(accessee.GetAccesseeSessionKeyId())});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        HILOGI("accessee is exists");
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::AccesseeToEntries(profile, values);
    int64_t rowId = ROWID_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Put(rowId, ACCESSEE_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("accessee_table insert failed");
            return DP_PUT_ACCESSEE_PROFILE_FAIL;
        }
    }
    HILOGI("PutAccessee : %{public}s", profile.GetAccessee().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccessControlId(AccessControlProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.SetAccessControlId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    int64_t accessControlId = ACCESSCONTROLID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESS_CONTROL_ID, columnIndex);
    resultSet->GetLong(columnIndex, accessControlId);
    resultSet->Close();
    profile.SetAccessControlId(accessControlId+1);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccesserId(AccessControlProfile& profile)
{
    Accesser accesser = profile.GetAccesser();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accesser.GetAccesserDeviceId()), ValueObject(accesser.GetAccesserUserId()),
        ValueObject(accesser.GetAccesserAccountId()), ValueObject(accesser.GetAccesserTokenId()),
        ValueObject(accesser.GetAccesserBundleName()), ValueObject(accesser.GetAccesserHapSignature()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserBindLevel())),
        ValueObject(accesser.GetAccesserDeviceName()), ValueObject(accesser.GetAccesserServiceName()),
        ValueObject(accesser.GetAccesserCredentialIdStr()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserStatus())),
        ValueObject(accesser.GetAccesserSessionKeyId())});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    int64_t accesserId = ACCESSERID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        profile.SetAccesserId(accesserId);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSER_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.GetAccesser().SetAccesserId(1);
        profile.SetAccesserId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
    resultSet->GetLong(columnIndex, accesserId);
    resultSet->Close();
    accesserId = accesserId + 1;
    profile.SetAccesserId(accesserId);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccesseeId(AccessControlProfile& profile)
{
    Accessee accessee = profile.GetAccessee();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accessee.GetAccesseeDeviceId()), ValueObject(accessee.GetAccesseeUserId()),
        ValueObject(accessee.GetAccesseeAccountId()), ValueObject(accessee.GetAccesseeTokenId()),
        ValueObject(accessee.GetAccesseeBundleName()), ValueObject(accessee.GetAccesseeHapSignature()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeBindLevel())),
        ValueObject(accessee.GetAccesseeDeviceName()), ValueObject(accessee.GetAccesseeServiceName()),
        ValueObject(accessee.GetAccesseeCredentialIdStr()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeStatus())),
        ValueObject(accessee.GetAccesseeSessionKeyId())});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    int64_t accesseeId = ACCESSEEID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        profile.SetAccesseeId(accesseeId);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSEE_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.GetAccessee().SetAccesseeId(1);
        profile.SetAccesseeId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
    resultSet->GetLong(columnIndex, accesseeId);
    resultSet->Close();
    accesseeId = accesseeId + 1;
    profile.SetAccesseeId(accesseeId);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccesserProfile(const AccessControlProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::AccesserToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Update(changeRowCnt, ACCESSER_TABLE, values, ACCESSERID_EQUAL_CONDITION,
            std::vector<ValueObject> {ValueObject(profile.GetAccesserId())});
        if (ret != DP_SUCCESS) {
            HILOGE("accesser_table update failed");
            return DP_UPDATE_ACCESSER_PROFILE_FAIL;
        }
    }
    HILOGI("UpdateAccesser : %{public}s", profile.GetAccesser().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccesseeProfile(const AccessControlProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::AccesseeToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Update(changeRowCnt, ACCESSEE_TABLE, values, ACCESSEEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetAccesseeId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("accessee_table update failed");
            return DP_UPDATE_ACCESSEE_PROFILE_FAIL;
        }
    }
    HILOGI("UpdateAccessee : %{public}s", profile.GetAccessee().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateTrustDeviceProfileNotify(const TrustDeviceProfile& oldProfile,
    const TrustDeviceProfile &newProfile)
{
    if (oldProfile.GetStatus() != newProfile.GetStatus()) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileUpdate(oldProfile, newProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileUpdate failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetResultStatus(const std::string& trustDeviceId, int32_t& trustDeviceStatus)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID,
        std::vector<ValueObject>{ ValueObject(trustDeviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t columnIndex = COLUMNINDEX_INIT;
    trustDeviceStatus = 0;
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t status = STATUS_INIT;
        resultSet->GetColumnIndex(STATUS, columnIndex);
        resultSet->GetInt(columnIndex, status);
        if (status == 1) {
            trustDeviceStatus = 1;
            break;
        }
    }
    resultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
        std::vector<ValueObject>{ ValueObject(accesserId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("not find data");
        accesserResultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
        std::vector<ValueObject>{ ValueObject(accesseeId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("not find data");
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
    accesserResultSet->Close();
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfilesByDeviceId(
    std::shared_ptr<ResultSet> resultSet, int64_t accesserId, int64_t accesseeId,
    const std::string& trustDeviceId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERDEVICEID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(trustDeviceId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEDEVICEID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(trustDeviceId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccessControlProfileCheck(AccessControlProfile& profile)
{
    Accessee accessee;
    int32_t ret = this->DeleteAccesseeCheck(profile.GetAccesseeId(), accessee);
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteAccesseeCheck failed");
        return ret;
    }
    Accesser accesser;
    ret = this->DeleteAccesserCheck(profile.GetAccesserId(), accesser);
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteAccesserCheck failed");
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        ret = rdbStore_->Delete(deleteRows, ACCESS_CONTROL_TABLE, ACCESSCONTROLID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete access_control_table failed");
            return DP_DELETE_ACCESS_CONTROL_PROFILE_FAIL;
        }
    }
    HILOGI("DeleteAclProfile : %{public}s", profile.dump().c_str());
    AccessControlProfile resultProfile(profile);
    resultProfile.SetAccesser(accesser);
    resultProfile.SetAccessee(accessee);
    profile = resultProfile;
    return DP_SUCCESS;
}

std::shared_ptr<ResultSet> TrustProfileManager::GetResultSet(
    const std::string& sql, std::vector<ValueObject> condition)
{
    if (sql.empty() || sql.length() > MAX_STRING_LEN) {
        HILOGE("sql is invalid");
        return nullptr;
    }
    if (condition.size() > MAX_PARAM_SIZE) {
        HILOGE("condition is invalid");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return nullptr;
    }
    return rdbStore_->Get(sql, condition);
}

int32_t TrustProfileManager::SetAccessControlProfileId(AccessControlProfile& accessControlProfile)
{
    int32_t ret = this->SetAccessControlId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccessControlId failed");
        return ret;
    }
    ret = this->SetAccesserId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccesserId failed");
        return ret;
    }
    ret = this->SetAccesseeId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccesseeId failed");
        return ret;
    }
    Accesser accesser(accessControlProfile.GetAccesser());
    accesser.SetAccesserId(accessControlProfile.GetAccesserId());
    accessControlProfile.SetAccesser(accesser);
    Accessee accessee(accessControlProfile.GetAccessee());
    accessee.SetAccesseeId(accessControlProfile.GetAccesseeId());
    accessControlProfile.SetAccessee(accessee);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, int32_t userId, const std::string& bundleName,
    std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID_ACCESSERBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId), ValueObject(bundleName) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSEEID_ACCESSEEBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId), ValueObject(bundleName) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, int32_t userId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSERID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(bundleName) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(bundleName) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfilesByTokenId(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, const std::string& trustDeviceId,
    int64_t tokenId, std::vector<AccessControlProfile> &profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERDEVICEID,
            std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(trustDeviceId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEETOKENID,
                std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(tokenId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEDEVICEID,
            std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(trustDeviceId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERTOKENID,
            std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(tokenId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        ProfileUtils::ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccesserCheck(int64_t accesserId, Accesser& accesser)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSERID,
        std::vector<ValueObject>{ ValueObject(accesserId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
        std::vector<ValueObject>{ ValueObject(accesserId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GoToNextRow();
    ProfileUtils::ConvertToAccesser(resultSet, accesser);
    resultSet->Close();
    if (rowCount == DELETE_ACCESSER_CONDITION) {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        int32_t ret = rdbStore_->Delete(deleteRows, ACCESSER_TABLE, ACCESSERID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete accesser_table accesserId failed");
            return DP_DELETE_ACCESSER_PROFILE_FAIL;
        }
        HILOGI("DeleteAccesser : %{public}s", accesser.dump().c_str());
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAclCheck(const AccessControlProfile& profile, AccessControlProfile& oldProfile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSCONTROLID,
        std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("accessControlId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    resultSet->GoToNextRow();
    ProfileUtils::ConvertToAccessControlProfile(resultSet, oldProfile);
    resultSet->Close();
    if (oldProfile.GetAccesseeId() != profile.GetAccessee().GetAccesseeId() ||
        oldProfile.GetAccesserId() != profile.GetAccesser().GetAccesserId() ||
        oldProfile.GetAccesserId() != profile.GetAccesserId() ||
        oldProfile.GetAccesseeId() != profile.GetAccesseeId()) {
        HILOGE("UpdateAclCheck:Can't Update not allowed attribute");
        return DP_UPDATE_ACL_NOT_ALLOW;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAclCheck(const AccessControlProfile& profile, bool peerDevInfoExists)
{
    TrustDeviceProfile trustProfile;
    ProfileUtils::ConvertToTrustDeviceProfile(profile, trustProfile);
    if (!peerDevInfoExists && !IsLnnAcl(profile)) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileAdd(trustProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileAdd failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    std::string trustDeviceId = profile.GetTrustDeviceId();
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(trustDeviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount == 0) {
        this->PutTrustDeviceProfile(trustProfile);
        return DP_SUCCESS;
    }
    int32_t status = STATUS_INIT;
    if (this->GetResultStatus(trustDeviceId, status) != DP_SUCCESS) {
        HILOGE("GetResultStatus failed");
        return DP_GET_RESULTSET_FAIL;
    }
    trustProfile.SetStatus(status);
    if (this->UpdateTrustDeviceProfile(trustProfile) != DP_SUCCESS) {
        HILOGE("UpdateTrustDeviceProfile failed");
        return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::IsAclExists(const AccessControlProfile &profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ALL_EXCEPT_STATUS, std::vector<ValueObject>{
        ValueObject(profile.GetAccesserId()), ValueObject(profile.GetAccesseeId()),
        ValueObject(profile.GetTrustDeviceId()), ValueObject(profile.GetSessionKey()),
        ValueObject(static_cast<int32_t>(profile.GetBindType())),
        ValueObject(static_cast<int32_t>(profile.GetAuthenticationType())),
        ValueObject(static_cast<int32_t>(profile.GetDeviceIdType())),
        ValueObject(profile.GetDeviceIdHash()), ValueObject(profile.GetValidPeriod()),
        ValueObject(profile.GetLastAuthTime()), ValueObject(static_cast<int32_t>(profile.GetBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount != 0) {
        HILOGI("accessControlProfile is exists");
        return DP_DATA_EXISTS;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CheckDeviceIdAndUserIdActive(const AccessControlProfile& profile, int32_t& resultCount)
{
    std::string peerDeviceId = profile.GetTrustDeviceId();
    std::string localDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t peerUserId = profile.GetAccesser().GetAccesserUserId();
    int32_t localUserId = profile.GetAccessee().GetAccesseeUserId();
    if (profile.GetAccessee().GetAccesseeDeviceId() == peerDeviceId) {
        peerUserId = profile.GetAccessee().GetAccesseeUserId();
        localUserId = profile.GetAccesser().GetAccesserUserId();
        localDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    }
    std::vector<AccessControlProfile> aclProfiles;
    int32_t ret = GetAllAccessControlProfiles(aclProfiles);
    if (ret != DP_SUCCESS) {
        HILOGE("GetAllAccessControlProfiles failed");
        return ret;
    }
    RemoveLnnAcl(aclProfiles);
    for (auto aclProfile : aclProfiles) {
        if (peerDeviceId != aclProfile.GetTrustDeviceId() ||
            STATUS_ACTIVE != aclProfile.GetStatus()) {
            continue;
        }
        if ((localDeviceId == aclProfile.GetAccesser().GetAccesserDeviceId() &&
            localUserId == aclProfile.GetAccesser().GetAccesserUserId() &&
            peerDeviceId == aclProfile.GetAccessee().GetAccesseeDeviceId() &&
            peerUserId == aclProfile.GetAccessee().GetAccesseeUserId()) ||
            (peerDeviceId == aclProfile.GetAccesser().GetAccesserDeviceId() &&
            peerUserId == aclProfile.GetAccesser().GetAccesserUserId() &&
            localDeviceId == aclProfile.GetAccessee().GetAccesseeDeviceId() &&
            localUserId == aclProfile.GetAccessee().GetAccesseeUserId())) {
            resultCount++;
            HILOGE("localUserId and peerUserId have active acl exist");
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CheckDeviceIdAndUserIdExists(const AccessControlProfile& profile, bool& isExists)
{
    std::string peerDeviceId = profile.GetTrustDeviceId();
    std::string localDeviceId = profile.GetAccessee().GetAccesseeDeviceId();
    int32_t peerUserId = profile.GetAccesser().GetAccesserUserId();
    int32_t localUserId = profile.GetAccessee().GetAccesseeUserId();
    if (profile.GetAccessee().GetAccesseeDeviceId() == peerDeviceId) {
        peerUserId = profile.GetAccessee().GetAccesseeUserId();
        localUserId = profile.GetAccesser().GetAccesserUserId();
        localDeviceId = profile.GetAccesser().GetAccesserDeviceId();
    }
    std::vector<AccessControlProfile> aclProfiles;
    int32_t ret = GetAllAccessControlProfiles(aclProfiles);
    if (ret != DP_SUCCESS) {
        HILOGE("GetAllAccessControlProfiles failed");
        return ret;
    }
    RemoveLnnAcl(aclProfiles);
    for (auto aclProfile : aclProfiles) {
        if (peerDeviceId != aclProfile.GetTrustDeviceId()) {
            continue;
        }
        if ((localDeviceId == aclProfile.GetAccesser().GetAccesserDeviceId() &&
            localUserId == aclProfile.GetAccesser().GetAccesserUserId() &&
            peerDeviceId == aclProfile.GetAccessee().GetAccesseeDeviceId() &&
            peerUserId == aclProfile.GetAccessee().GetAccesseeUserId()) ||
            (peerDeviceId == aclProfile.GetAccesser().GetAccesserDeviceId() &&
            peerUserId == aclProfile.GetAccesser().GetAccesserUserId() &&
            localDeviceId == aclProfile.GetAccessee().GetAccesseeDeviceId() &&
            localUserId == aclProfile.GetAccessee().GetAccesseeUserId())) {
            isExists = true;
            HILOGE("localUserId and peerUserId acl exist");
            break;
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::NotifyCheck(const AccessControlProfile& profile, const AccessControlProfile& oldProfile)
{
    int32_t resultCount = 0;
    int32_t ret = CheckDeviceIdAndUserIdActive(profile, resultCount);
    if (ret != DP_SUCCESS) {
        HILOGE("CheckDeviceIdAndUserIdActive failed");
        return DP_NOTIFY_TRUST_DEVICE_FAIL;
    }
    HILOGI("resultCount : %{public}d", resultCount);
    TrustDeviceProfile trustProfile;
    ProfileUtils::ConvertToTrustDeviceProfile(profile, trustProfile);
    if (resultCount == 1 && profile.GetStatus() == STATUS_ACTIVE &&
        oldProfile.GetStatus() == STATUS_INACTIVE && !IsLnnAcl(profile)) {
        ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileActive(trustProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileActive failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    if (resultCount == 0 && profile.GetStatus() == STATUS_INACTIVE &&
        oldProfile.GetStatus() == STATUS_ACTIVE && !IsLnnAcl(profile)) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileInactive(trustProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileInactive failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccesseeCheck(int64_t accesseeId, Accessee& accessee)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSEEID,
        std::vector<ValueObject>{ ValueObject(accesseeId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
        std::vector<ValueObject>{ ValueObject(accesseeId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GoToNextRow();
    ProfileUtils::ConvertToAccessee(resultSet, accessee);
    resultSet->Close();
    if (rowCount == DELETE_ACCESSEE_CONDITION) {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        int32_t ret = rdbStore_->Delete(deleteRows, ACCESSEE_TABLE, ACCESSEEID_EQUAL_CONDITION,
            std::vector<ValueObject> {ValueObject(accesseeId)});
        if (ret != DP_SUCCESS) {
            HILOGE("delete accessee_table accesseeId failed");
            return DP_DELETE_ACCESSEE_PROFILE_FAIL;
        }
        HILOGI("DeleteAccessee : %{public}s", accessee.dump().c_str());
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteTrustDeviceCheck(const AccessControlProfile& profile)
{
    TrustDeviceProfile trustProfile;
    ProfileUtils::ConvertToTrustDeviceProfile(profile, trustProfile);
    bool isExists = false;
    CheckDeviceIdAndUserIdExists(profile, isExists);
    if (!isExists && !IsLnnAcl(profile)) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileDelete(trustProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileDelete failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID,
        std::vector<ValueObject>{ ValueObject(profile.GetTrustDeviceId()) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    int32_t ret = RET_INIT;
    if (rowCount == DELETE_TRUST_CONDITION) {
        ret = this->DeleteTrustDeviceProfile(profile.GetTrustDeviceId());
        if (ret != DP_SUCCESS) {
            HILOGE("DeleteTrustDeviceProfile failed");
            return DP_DELETE_TRUST_DEVICE_PROFILE_FAIL;
        }
    } else {
        int32_t status = STATUS_INIT;
        this->GetResultStatus(profile.GetTrustDeviceId(), status);
        TrustDeviceProfile trustDeviceProfile(trustProfile);
        trustDeviceProfile.SetStatus(status);
        ret = this->UpdateTrustDeviceProfile(trustDeviceProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("UpdateTrustDeviceProfile failed");
            return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    return DP_SUCCESS;
}

void TrustProfileManager::RemoveLnnAcl(std::vector<AccessControlProfile>& profiles)
{
    std::vector<AccessControlProfile> aclProfiles(profiles);
    profiles.clear();
    for (auto aclProfile : aclProfiles) {
        if (IsLnnAcl(aclProfile)) {
            continue;
        }
        profiles.emplace_back(aclProfile);
    }
    return;
}

bool TrustProfileManager::IsLnnAcl(const AccessControlProfile& aclProfile)
{
    if (aclProfile.GetExtraData().empty()) {
        return false;
    }
    cJSON* json = cJSON_Parse(aclProfile.GetExtraData().c_str());
    if (!cJSON_IsObject(json)) {
        HILOGW("cJSON_Parse extraData fail!");
        cJSON_Delete(json);
        return false;
    }
    std::string lnnAclValue;
    cJSON* item = cJSON_GetObjectItemCaseSensitive(json, IS_LNN_ACL.c_str());
    if (item != NULL && cJSON_IsString(item)) {
        lnnAclValue = item->valuestring;
    }
    if (lnnAclValue == LNN_ACL_TRUE) {
        item = NULL;
        cJSON_Delete(json);
        return true;
    }
    item = NULL;
    cJSON_Delete(json);
    return false;
}

bool TrustProfileManager::IsAceeCreIdExistToAceeTable()
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(PRAGMA_ACCESSEE_TABLE, std::vector<ValueObject>{});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return false;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        std::string columnName;
        resultSet->GetColumnIndex(NAME, columnIndex);
        resultSet->GetString(columnIndex, columnName);
        if (columnName == ACCESSEE_CREDENTIAL_ID) {
            HILOGE("aceeCreId exist to acee_table");
            return true;
        }
    }
    return false;
}

int32_t TrustProfileManager::AddAceeCreIdColumnToAceeTable()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return DP_GET_RDBSTORE_FAIL;
    }
    int32_t ret = rdbStore_->CreateTable(ALTER_TABLE_ACEE_ADD_COLUMN_ACEE_CREDENTIAL_ID);
    if (ret != DP_SUCCESS) {
        HILOGE("add column accesseeCredentialId to acee table failed");
        return DP_CREATE_TABLE_FAIL;
    }
    return DP_SUCCESS;
}
} // namespace DistributedDeviceProfile
} // namespace OHOS