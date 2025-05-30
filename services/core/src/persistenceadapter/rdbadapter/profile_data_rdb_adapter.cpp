/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "profile_data_rdb_adapter.h"

#include <mutex>
#include <chrono>
#include <unistd.h>

#include "distributed_device_profile_constants.h"
#include "dp_services_constants.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_log.h"
#include "rdb_errno.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(ProfileDataRdbAdapter);
using namespace std::chrono_literals;
namespace {
    const std::set<std::string> TABLES = {
        "device_profile",
        "service_profile",
        "characteristic_profile",
        "device_icon_info",
        "product_info"
    };
    const std::string TAG = "ProfileDatardbAdapter";
}

int32_t ProfileDataRdbAdapter::Init()
{
    int32_t retryTimes = RDB_INIT_MAX_TIMES;
    while (retryTimes > 0) {
        if (GetRDBPtr() == DP_SUCCESS) {
            HILOGI("ProfileDatardbAdapter init success");
            return DP_SUCCESS;
        }
        usleep(RDB_INIT_INTERVAL_TIME);
        retryTimes--;
    }
    HILOGE("ProfileDatardbAdapter init failed");
    return DP_RDBADAPTER_INIT_FAIL;
}

int32_t ProfileDataRdbAdapter::UnInit()
{
    HILOGI("ProfileDatardbAdapter unInit");
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        store_ = nullptr;
    }
    return DP_SUCCESS;
}

int32_t ProfileDataRdbAdapter::Put(int64_t& outRowId, const std::string& table, const ValuesBucket& values)
{
    if (TABLES.find(table) == TABLES.end()) {
        HILOGE("table does not exist");
        return DP_RDBADAPTER_TABLE_NOT_EXIST;
    }
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return DP_RDB_DB_PTR_NULL;
        }
        int32_t ret = store_->Insert(outRowId, table, values);
        if (ret == E_SQLITE_CORRUPT) {
            HILOGE("database corrupt ret:%{public}d", ret);
            int32_t restoreRet = store_->Restore("");
            if (restoreRet != E_OK) {
                HILOGE("Restore failed restoreRet:%{public}d", restoreRet);
                return DP_RDB_DATABASE_RESTORE_FAIL;
            }
            ret = store_->Insert(outRowId, table, values);
        }
        if (ret != E_OK) {
            HILOGE("ProfileDatardbAdapter put failed ret:%{public}d", ret);
            return DP_RDBADAPTER_PUT_FAIL;
        }
    }
    return DP_SUCCESS;
}

int32_t ProfileDataRdbAdapter::Delete(int32_t& deleteRows, const std::string& table, const std::string& whereClause,
    const std::vector<ValueObject>& bindArgs)
{
    if (TABLES.find(table) == TABLES.end()) {
        HILOGE("table does not exist");
        return DP_RDBADAPTER_TABLE_NOT_EXIST;
    }
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return DP_RDB_DB_PTR_NULL;
        }
        int32_t ret = store_->Delete(deleteRows, table, whereClause, bindArgs);
        if (ret == E_SQLITE_CORRUPT) {
            HILOGE("database corrupt ret:%{public}d", ret);
            int32_t restoreRet = store_->Restore("");
            if (restoreRet != E_OK) {
                HILOGE("Restore failed restoreRet:%{public}d", restoreRet);
                return DP_RDB_DATABASE_RESTORE_FAIL;
            }
            ret = store_->Delete(deleteRows, table, whereClause, bindArgs);
        }
        if (ret != E_OK) {
            HILOGE("ProfileDatardbAdapter delete failed ret:%{public}d", ret);
            return DP_RDBADAPTER_DELETE_FAIL;
        }
    }
    return DP_SUCCESS;
}

int32_t ProfileDataRdbAdapter::Update(int32_t& changedRows, const std::string& table, const ValuesBucket& values,
    const std::string& whereClause, const std::vector<ValueObject>& bindArgs)
{
    if (TABLES.find(table) == TABLES.end()) {
        HILOGE("table does not exist");
        return DP_RDBADAPTER_TABLE_NOT_EXIST;
    }
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return DP_RDB_DB_PTR_NULL;
        }
        int32_t ret = store_->Update(changedRows, table, values, whereClause, bindArgs);
        if (ret == E_SQLITE_CORRUPT) {
            HILOGE("database corrupt ret:%{public}d", ret);
            int32_t restoreRet = store_->Restore("");
            if (restoreRet != E_OK) {
                HILOGE("Restore failed restoreRet:%{public}d", restoreRet);
                return DP_RDB_DATABASE_RESTORE_FAIL;
            }
            ret = store_->Update(changedRows, table, values, whereClause, bindArgs);
        }
        if (ret != E_OK) {
            HILOGE("ProfileDatardbAdapter update failed ret:%{public}d", ret);
            return DP_RDBADAPTER_UPDATE_FAIL;
        }
    }
    return DP_SUCCESS;
}

std::shared_ptr<ResultSet> ProfileDataRdbAdapter::Get(const std::string& sql, const std::vector<ValueObject>& args)
{
    std::shared_ptr<ResultSet> resultSet = nullptr;
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return nullptr;
        }
        resultSet = store_->QueryByStep(sql, args);
        if (resultSet == nullptr) {
            HILOGE("resultSet is null");
            return nullptr;
        }
        int32_t rowCount = ROWCOUNT_INIT;
        int32_t ret = resultSet->GetRowCount(rowCount);
        if (ret == E_SQLITE_CORRUPT) {
            HILOGE("database corrupt ret:%{public}d", ret);
            resultSet->Close();
            ret = store_->Restore("");
            if (ret != E_OK) {
                HILOGE("Restore failed ret:%{public}d", ret);
                return nullptr;
            }
            resultSet = store_->QueryByStep(sql, args);
        }
    }
    return resultSet;
}

int32_t ProfileDataRdbAdapter::GetRDBPtr()
{
    int32_t version = RDB_VERSION_5_1;
    ProfileDataOpenCallback helper;
    RdbStoreConfig config(PROFILE_DATA_RDB_PATH + PROFILE_DATA_DATABASE_NAME);
    config.SetSecurityLevel(SecurityLevel::S2);
    config.SetHaMode(HAMode::MAIN_REPLICA);
    config.SetAllowRebuild(true);
    int32_t errCode = E_OK;
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        store_ = RdbHelper::GetRdbStore(config, version, helper, errCode);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return DP_RDB_DB_PTR_NULL;
        }
        NativeRdb::RebuiltType rebuiltType = NativeRdb::RebuiltType::NONE;
        errCode = store_->GetRebuilt(rebuiltType);
        if (errCode != E_OK) {
            HILOGE("getRDBPtr failed errCode:%{public}d", errCode);
            return DP_GET_RDBSTORE_FAIL;
        }
        if (rebuiltType == NativeRdb::RebuiltType::REBUILT) {
            HILOGE("database corrupt");
            int32_t restoreRet = store_->Restore("");
            if (restoreRet != E_OK) {
                HILOGE("Restore failed restoreRet:%{public}d", restoreRet);
                return DP_RDB_DATABASE_RESTORE_FAIL;
            }
        }
    }
    return DP_SUCCESS;
}

bool ProfileDataRdbAdapter::IsInit()
{
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            return false;
        }
    }
    return true;
}

int32_t ProfileDataRdbAdapter::CreateTable(const std::string& sql)
{
    {
        std::lock_guard<std::mutex> lock(ProfileDataRdbAdapterMtx_);
        if (store_ == nullptr) {
            HILOGE("RDBStore_ is null");
            return DP_RDB_DB_PTR_NULL;
        }
        if (store_->ExecuteSql(sql) != E_OK) {
            HILOGE("ProfileDatardbAdapter create table failed");
            return DP_RDBADAPTER_TABLE_NOT_EXIST;
        }
    }
    return DP_SUCCESS;
}

int32_t ProfileDataOpenCallback::OnCreate(RdbStore& store)
{
    HILOGI("rdbStore create");
    return NativeRdb::E_OK;
}

int32_t ProfileDataOpenCallback::OnUpgrade(RdbStore& store, int oldVersion, int newVersion)
{
    HILOGI("rdbStore upgrade : %{public}d -> %{public}d", oldVersion, newVersion);
    if (oldVersion == RDB_VERSION && newVersion == RDB_VERSION_5_1) {
        return UpdateFromVer1To2(store);
    }
    return NativeRdb::E_OK;
}

int32_t ProfileDataOpenCallback::UpdateFromVer1To2(RdbStore& store)
{
    int32_t ret = store.ExecuteSql(ALTER_TABLE_DP_ADD_COLUMN_PRODUCT_NAME_SQL);
    if (ret != NativeRdb::E_OK) {
        HILOGE("add column to device_profile table failed, ret:%{public}d", ret);
        return ret;
    }
    ret = store.ExecuteSql(ALTER_TABLE_DP_RENAME_COLUMN_INTERNAL_MODEL_SQL);
    if (ret != NativeRdb::E_OK) {
        HILOGE("add column to device_icon_info table failed, ret:%{public}d", ret);
        return ret;
    }
    ret = store.ExecuteSql(ALTER_TABLE_DEVICE_ICON_INFO_ADD_COLUMN_INTENAL_MODEL_SQL);
    if (ret != NativeRdb::E_OK) {
        HILOGE("add column to device_icon_info table failed, ret:%{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}
} // namespace DistributedDeviceProfile
} // namespace OHOS
