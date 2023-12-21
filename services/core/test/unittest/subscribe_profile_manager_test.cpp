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

#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <iostream>

#define private public
#define protected public
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_log.h"
#include "distributed_device_profile_errors.h"
#include "subscribe_profile_manager.h"
#include "distributed_device_profile_client.h"
#undef private
#undef protected

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace testing::ext;
// using namespace OHOS::DistributedDeviceProfile;
using namespace std;
namespace {
    const std::string TAG = "SubscribeProfileManagerTest";
}
class SubscribeProfileManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SubscribeProfileManagerTest::SetUpTestCase()
{
}

void SubscribeProfileManagerTest::TearDownTestCase()
{
}

void SubscribeProfileManagerTest::SetUp()
{
}

void SubscribeProfileManagerTest::TearDown()
{
}

class SubscribeDPChangeListener : public ProfileChangeListenerStub {
public:
    SubscribeDPChangeListener()
    {
    }
    ~SubscribeDPChangeListener()
    {
    }
    int32_t OnTrustDeviceProfileAdd(const TrustDeviceProfile &profile)
    {
        return 0;
    }
    int32_t OnTrustDeviceProfileDelete(const TrustDeviceProfile &profile)
    {
        return 0;
    }
    int32_t OnTrustDeviceProfileUpdate(const TrustDeviceProfile &oldProfile, const TrustDeviceProfile &newProfile)
    {
        return 0;
    }
    int32_t OnDeviceProfileAdd(const DeviceProfile &profile)
    {
        return 0;
    }
    int32_t OnDeviceProfileDelete(const DeviceProfile &profile)
    {
        return 0;
    }
    int32_t OnDeviceProfileUpdate(const DeviceProfile &oldProfile, const DeviceProfile &newProfile)
    {
        return 0;
    }
    int32_t OnServiceProfileAdd(const ServiceProfile &profile)
    {
        return 0;
    }
    int32_t OnServiceProfileDelete(const ServiceProfile &profile)
    {
        return 0;
    }
    int32_t OnServiceProfileUpdate(const ServiceProfile &oldProfile, const ServiceProfile &newProfile)
    {
        return 0;
    }
    int32_t OnCharacteristicProfileAdd(const CharacteristicProfile &profile)
    {
        return 0;
    }
    int32_t OnCharacteristicProfileDelete(const CharacteristicProfile &profile)
    {
        return 0;
    }
    int32_t OnCharacteristicProfileUpdate(const CharacteristicProfile &oldProfile,
                                          const CharacteristicProfile &newProfile)
    {
        return 0;
    }
};

/*
 * @tc.name: SubscribeDeviceProfile_001
 * @tc.desc: Normal testCase of SubscribeDeviceProfile for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, SubscribeDeviceProfile_001, TestSize.Level1)
{
    string subscribeKey = "subscribeKey";
    int32_t saId = 4801;
    unordered_set<ProfileChangeType> subscribeTypes = {
        ProfileChangeType::SERVICE_PROFILE_ADD,
        ProfileChangeType::SERVICE_PROFILE_UPDATE,
        ProfileChangeType::SERVICE_PROFILE_DELETE
    };
    OHOS::sptr<IProfileChangeListener> subscribeDPChangeListener = new(nothrow) SubscribeDPChangeListener;
    SubscribeInfo subscribeInfo(saId, subscribeKey, subscribeTypes, subscribeDPChangeListener);
    
    int32_t errCode = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/**
 * @tc.name: UnSubscribeDeviceProfile001
 * @tc.desc: UnSubscribeDeviceProfile success
 * @tc.type: FUNC
 * @tc.require:
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, UnSubscribeDeviceProfile001, TestSize.Level1)
{
    string subscribeKey = "subscribeKey";
    int32_t saId = 4801;
    unordered_set<ProfileChangeType> subscribeTypes = {
            ProfileChangeType::SERVICE_PROFILE_ADD,
            ProfileChangeType::SERVICE_PROFILE_UPDATE,
            ProfileChangeType::SERVICE_PROFILE_DELETE
    };
    OHOS::sptr<IProfileChangeListener> subscribeDPChangeListener = new(nothrow) SubscribeDPChangeListener;
    SubscribeInfo subscribeInfo(saId, subscribeKey, subscribeTypes, subscribeDPChangeListener);
    
    int32_t errCode = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyDeviceProfileAdd_001
 * @tc.desc: Normal testCase of NotifyDeviceProfileAdd for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyDeviceProfileAdd_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyDeviceProfileAdd(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyDeviceProfileUpdate_001
 * @tc.desc: Normal testCase of NotifyDeviceProfileUpdate for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyDeviceProfileUpdate_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyDeviceProfileUpdate(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyDeviceProfileDelete_001
 * @tc.desc: Normal testCase of NotifyDeviceProfileDelete for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyDeviceProfileDelete_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyDeviceProfileDelete(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyServiceProfileAdd_001
 * @tc.desc: Normal testCase of NotifyServiceProfileAdd for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyServiceProfileAdd_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyServiceProfileAdd(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyServiceProfileUpdate_001
 * @tc.desc: Normal testCase of NotifyServiceProfileUpdate for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyServiceProfileUpdate_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyServiceProfileUpdate(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyServiceProfileDelete_001
 * @tc.desc: Normal testCase of NotifyServiceProfileUpdate for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyServiceProfileDelete_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyServiceProfileDelete(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyCharProfileAdd_001
 * @tc.desc: Normal testCase of NotifyCharProfileAdd for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyCharProfileAdd_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyCharProfileAdd(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyCharProfileUpdate_001
 * @tc.desc: Normal testCase of NotifyCharProfileAdd for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyCharProfileUpdate_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyCharProfileUpdate(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}

/*
 * @tc.name: NotifyCharProfileDelete_001
 * @tc.desc: Normal testCase of NotifyCharProfileDelete for CRUD
 * @tc.type: FUNC
 * @tc.require: I4NY1T
 */
HWTEST_F(SubscribeProfileManagerTest, NotifyCharProfileDelete_001, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t errCode = SubscribeProfileManager::GetInstance().NotifyCharProfileDelete(dbKey, dbValue);
    EXPECT_EQ(errCode, DP_SUCCESS);
}
}
}