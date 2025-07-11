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

#ifndef OHOS_DP_TRUST_PROFILE_MANAGER_H
#define OHOS_DP_TRUST_PROFILE_MANAGER_H

#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <vector>


#include "access_control_profile.h"
#include "irdb_adapter.h"
#include "query_profile.h"
#include "single_instance.h"
#include "trust_device_profile.h"
#include "values_bucket.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace OHOS::NativeRdb;

class TrustProfileManager {
    DECLARE_SINGLE_INSTANCE(TrustProfileManager);

public:
    int32_t Init();
    int32_t UnInit();
    int32_t PutTrustDeviceProfile(const TrustDeviceProfile& profile);
    int32_t PutAccessControlProfile(const AccessControlProfile& profile);
    int32_t UpdateTrustDeviceProfile(const TrustDeviceProfile& profile);
    int32_t UpdateAccessControlProfile(const AccessControlProfile& profile);
    int32_t GetTrustDeviceProfile(const std::string& deviceId, TrustDeviceProfile& profile);
    int32_t GetAllTrustDeviceProfile(std::vector<TrustDeviceProfile>& profile);
    int32_t GetAllAccessControlProfile(std::vector<AccessControlProfile>& profiles);
    int32_t GetAllAclIncludeLnnAcl(std::vector<AccessControlProfile>& profiles);
    int32_t GetAccessControlProfile(const std::map<std::string, std::string>& params,
        std::vector<AccessControlProfile>& profile);
    int32_t DeleteTrustDeviceProfile(const std::string& deviceId);
    int32_t DeleteAccessControlProfile(int64_t accessControlId);

private:
    int32_t CreateTable();
    int32_t CreateUniqueIndex();
    bool CheckForWardByAcerAndAcee(const QueryProfile& queryProfile, const AccessControlProfile& aclProfile);
    bool CheckReverseByAcerAndAcee(const QueryProfile& queryProfile, const AccessControlProfile& aclProfile);
    bool CheckForWardByAcer(const QueryProfile& queryProfile, const AccessControlProfile& aclProfile);
    bool CheckReverseByAcer(const QueryProfile& queryProfile, const AccessControlProfile& aclProfile);
    int32_t GetAccessControlProfile(const QueryType& queryType,
        const QueryProfile& queryProfile, std::vector<AccessControlProfile>& profile);
    bool GenerateQueryProfile(const std::map<std::string, std::string>& params,
        QueryType& queryType, QueryProfile& queryProfile);
    int32_t GetAllAccessControlProfiles(std::vector<AccessControlProfile>& profiles);
    int32_t GetAccessControlProfile(const std::string& bundleName, int32_t bindType,
        int32_t status, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfile(const std::string& bundleName,
        const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfile(int32_t userId, const std::string& bundleName,
        int32_t bindType, int32_t status, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfile(int32_t userId, const std::string& bundleName,
        const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfile(int32_t userId, const std::string& accountId,
        std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfile(int32_t userId, std::vector<AccessControlProfile>& profile);
    void GetAclByAcerTokenId(const QueryProfile& queryProfile,
    const std::vector<AccessControlProfile>& aclProfiles, std::vector<AccessControlProfile>& profile);
    void GetAclByAcerAndAceeTokenId(const QueryProfile& queryProfile,
        const std::vector<AccessControlProfile>& aclProfiles, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfileByTokenId(int64_t tokenId, const std::string& trustDeviceId,
        int32_t status, std::vector<AccessControlProfile>& profile);
    int32_t GetAclProfileByUserIdAndBundleName(std::shared_ptr<ResultSet> resultSet,
        int32_t userId, const std::string& bundleName, std::vector<AccessControlProfile>& profile);
    int32_t GetAclProfileByUserIdAndAccountId(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
        int64_t accesseeId, int32_t userId, const std::string& accountId, std::vector<AccessControlProfile>& profile);
    int32_t GetAclProfileByTokenId(std::shared_ptr<ResultSet> resultSet, const std::string& trustDeviceId,
        int64_t tokenId, std::vector<AccessControlProfile>& profile);
    int32_t GetAclProfileByBundleName(std::shared_ptr<ResultSet> resultSet,
        const std::string& bundleName, std::vector<AccessControlProfile>& profile);
    int32_t PutAccesserProfile(const AccessControlProfile& profile);
    int32_t PutAccesseeProfile(const AccessControlProfile& profile);
    int32_t SetAccessControlId(AccessControlProfile& profile);
    int32_t SetAccesserId(AccessControlProfile& profile);
    int32_t SetAccesseeId(AccessControlProfile& profile);
    int32_t UpdateAccesserProfile(const AccessControlProfile& profile);
    int32_t UpdateAccesseeProfile(const AccessControlProfile& profile);
    int32_t UpdateTrustDeviceProfileNotify(const TrustDeviceProfile& oldProfile,
        const TrustDeviceProfile& newProfile);
    int32_t GetResultStatus(const std::string& trustDeviceId, int32_t& trustDeviceStatus);
    int32_t GetAccessControlProfile(std::shared_ptr<ResultSet> resultSet,
        int64_t accesserId, int64_t accesseeId, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfilesByDeviceId(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
        int64_t accesseeId, const std::string& trustDeviceId, std::vector<AccessControlProfile>& profile);
    int32_t DeleteAccessControlProfileCheck(AccessControlProfile& profile);
    std::shared_ptr<ResultSet> GetResultSet(const std::string& sql, std::vector<ValueObject> condition);
    int32_t SetAccessControlProfileId(AccessControlProfile& accessControlProfile);
    int32_t GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
        int64_t accesseeId, int32_t userId, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet,
        int64_t accesserId, int64_t accesseeId, int32_t userId, const std::string& bundleName,
        std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
        int64_t accesseeId, const std::string& bundleName, std::vector<AccessControlProfile>& profile);
    int32_t GetAccessControlProfilesByTokenId(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
        int64_t accesseeId, const std::string& trustDeviceId, int64_t tokenId,
        std::vector<AccessControlProfile>& profile);
    int32_t DeleteAccesserCheck(int64_t accesserId, Accesser& accesser);
    int32_t DeleteAccesseeCheck(int64_t accesseeId, Accessee& accessee);
    int32_t DeleteTrustDeviceCheck(const AccessControlProfile& profile);
    int32_t UpdateAclCheck(const AccessControlProfile& profile, AccessControlProfile& oldProfile);
    int32_t PutAclCheck(const AccessControlProfile& profile, bool peerDevInfoExists);
    int32_t IsAclExists(const AccessControlProfile& profile);
    int32_t CheckDeviceIdAndUserIdActive(const AccessControlProfile& profile, int32_t& resultCount);
    int32_t CheckDeviceIdAndUserIdExists(const AccessControlProfile& profile, bool& isExists);
    int32_t NotifyCheck(const AccessControlProfile& profile, const AccessControlProfile& oldProfile);
    void RemoveLnnAcl(std::vector<AccessControlProfile>& profiles);
    bool IsLnnAcl(const AccessControlProfile& aclProfile);
    bool IsAceeCreIdExistToAceeTable();
    int32_t AddAceeCreIdColumnToAceeTable();

private:
    std::shared_ptr<IRdbAdapter> rdbStore_;
    std::mutex aclMutex_;
    std::mutex rdbMutex_;
};

} // namespace DistributedDeviceProfile
} // namespace OHOS

#endif // OHOS_DP_TRUST_PROFILE_MANAGER_H
