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

#include "profile_utils.h"
#include "trust_profile_manager.h"
#include "dp_subscribe_info.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_log.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_client.h"


using namespace testing::ext;
using namespace OHOS::DistributedDeviceProfile;
using namespace std;
namespace {
    const std::string TAG = "DPSubscribeInfoTest";
}
class DPSubscribeInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    class SubscribeDPChangeListener : public ProfileChangeListenerStub {
    public:
        SubscribeDPChangeListener()
        {
            cout << "SubscribeDPChangeListener constructor" << endl;
        }
        ~SubscribeDPChangeListener()
        {
            cout << "SubscribeDPChangeListener destructor" << endl;
        }
        int32_t OnTrustDeviceProfileAdd(const TrustDeviceProfile& profile)
        {
            cout << "OnTrustDeviceProfileAdd" << profile.dump() <<endl;
            return 0;
        }
        int32_t OnTrustDeviceProfileDelete(const TrustDeviceProfile& profile)
        {
            cout << "OnTrustDeviceProfileDelete" << profile.dump() <<endl;
            return 0;
        }
        int32_t OnTrustDeviceProfileUpdate(const TrustDeviceProfile& oldProfile, const TrustDeviceProfile& newProfile)
        {
            cout << "OnTrustDeviceProfileUpdate: oldProfile " << oldProfile.dump() <<endl;
            cout << "OnTrustDeviceProfileUpdate: newProfile " << newProfile.dump() <<endl;
            return 0;
        }
        int32_t OnDeviceProfileAdd(const DeviceProfile& profile)
        {
            return 0;
        }
        int32_t OnDeviceProfileDelete(const DeviceProfile& profile)
        {
            return 0;
        }
        int32_t OnDeviceProfileUpdate(const DeviceProfile& oldProfile, const DeviceProfile& newProfile)
        {
            return 0;
        }
        int32_t OnServiceProfileAdd(const ServiceProfile& profile)
        {
            return 0;
        }
        int32_t OnServiceProfileDelete(const ServiceProfile& profile)
        {
            return 0;
        }
        int32_t OnServiceProfileUpdate(const ServiceProfile& oldProfile, const ServiceProfile& newProfile)
        {
            return 0;
        }
        int32_t OnCharacteristicProfileAdd(const CharacteristicProfile& profile)
        {
            return 0;
        }
        int32_t OnCharacteristicProfileDelete(const CharacteristicProfile& profile)
        {
            return 0;
        }
        int32_t OnCharacteristicProfileUpdate(const CharacteristicProfile& oldProfile,
            const CharacteristicProfile& newProfile)
        {
            return 0;
        }
    };
};

void DPSubscribeInfoTest::SetUpTestCase()
{
}

void DPSubscribeInfoTest::TearDownTestCase()
{
}

void DPSubscribeInfoTest::SetUp()
{
}

void DPSubscribeInfoTest::TearDown()
{
}

/*
 * @tc.name: GetAccessControlProfile_003
 * @tc.desc: Normal testCase of DPSubscribeInfoTest for CRUD
 * @tc.type: FUNC
 */
HWTEST_F(DPSubscribeInfoTest, Get_001, TestSize.Level1)
{
    uint32_t saId = 4801;
    std::string subscribekey = "trust_device_profile";
    std::unordered_set<ProfileChangeType> subscribeTypes = {ProfileChangeType::TRUST_DEVICE_PROFILE_ADD,
        ProfileChangeType::TRUST_DEVICE_PROFILE_UPDATE, ProfileChangeType::TRUST_DEVICE_PROFILE_DELETE};
    OHOS::sptr<IProfileChangeListener> subscribeDPChangeListener =
	    new(std::nothrow) DPSubscribeInfoTest::SubscribeDPChangeListener;
    SubscribeInfo subscribeInfo(saId, subscribekey, subscribeTypes, subscribeDPChangeListener);
    saId = subscribeInfo.GetSaId();
    std::string subscribekey_ = subscribeInfo.GetSubscribeKey();
    std::unordered_set<ProfileChangeType> subscribeTypes_ = subscribeInfo.GetProfileChangeTypes();
    subscribeInfo.GetListener();
    int32_t ret = DP_SUCCESS;
    EXPECT_EQ(ret, DP_SUCCESS);
}

/*
 * @tc.name: Set_001
 * @tc.desc: Normal testCase of DPSubscribeInfoTest for CRUD
 * @tc.type: FUNC
 */
HWTEST_F(DPSubscribeInfoTest, Set_001, TestSize.Level1)
{
    uint32_t saId = 4801;
    std::string subscribekey = "trust_device_profile";
    std::unordered_set<ProfileChangeType> subscribeTypes = {ProfileChangeType::TRUST_DEVICE_PROFILE_ADD,
        ProfileChangeType::TRUST_DEVICE_PROFILE_UPDATE, ProfileChangeType::TRUST_DEVICE_PROFILE_DELETE};
    OHOS::sptr<IProfileChangeListener> subscribeDPChangeListener = nullptr;
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(saId);
    subscribeInfo.SetSubscribeKey(subscribekey);
    subscribeInfo.SetListener(subscribeDPChangeListener);
    subscribeInfo.dump();
    int32_t ret = DP_SUCCESS;
    EXPECT_EQ(ret, DP_SUCCESS);
}

/*
 * @tc.name: Set_002
 * @tc.desc: Normal testCase of DPSubscribeInfoTest for CRUD
 * @tc.type: FUNC
 */
HWTEST_F(DPSubscribeInfoTest, Set_002, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    std::string deviceId = "";
    std::string deviceAttribute ="";
    subscribeInfo.SetSubscribeKey(deviceId, deviceAttribute);
    std::string serviceName = "";
    std::string serviceAttribute = "";
    subscribeInfo.SetSubscribeKey(deviceId, serviceName, serviceAttribute);
    std::string characteristicKey = "";
    std::string characteristicAttribute = "";
    subscribeInfo.SetSubscribeKey(deviceId, serviceName, characteristicKey, characteristicAttribute);

    int32_t ret = DP_SUCCESS;
    EXPECT_EQ(ret, DP_SUCCESS);
}

/*
 * @tc.name: AddProfileChangeType_001
 * @tc.desc: Normal testCase of DPSubscribeInfoTest for CRUD
 * @tc.type: FUNC
 */
HWTEST_F(DPSubscribeInfoTest, AddProfileChangeType_001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.AddProfileChangeType(ProfileChangeType::TRUST_DEVICE_PROFILE_ADD);
    int32_t ret = DP_SUCCESS;
    EXPECT_EQ(ret, DP_SUCCESS);
}

/*
 * @tc.name: Marshalling_001
 * @tc.desc: Normal testCase of DPSubscribeInfoTest for CRUD
 * @tc.type: FUNC
 */
HWTEST_F(DPSubscribeInfoTest, Marshalling_001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetListener(nullptr);
    OHOS::MessageParcel parcel;
    bool ret = subscribeInfo.Marshalling(parcel);
    EXPECT_EQ(ret, false);
    ret = subscribeInfo.UnMarshalling(parcel);
    EXPECT_EQ(ret, false);
}