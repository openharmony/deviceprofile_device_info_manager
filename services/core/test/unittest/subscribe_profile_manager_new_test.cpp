/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <string>
#include <vector>
#include <map>
#include "gtest/gtest.h"
#include "refbase.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_enums.h"
#include "subscribe_profile_manager.h"
#include "dp_subscribe_info.h"
#include "trust_device_profile.h"
#include "device_profile.h"
#include "service_profile.h"
#include "characteristic_profile.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace testing::ext;
using namespace std;

class SubscribeProfileManagerNewTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SubscribeProfileManagerNewTest::SetUpTestCase(void) {
}

void SubscribeProfileManagerNewTest::TearDownTestCase(void) {
}

void SubscribeProfileManagerNewTest::SetUp() {
}

void SubscribeProfileManagerNewTest::TearDown() {
}

/**
 * @tc.name: Init001
 * @tc.desc: Test Init method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, Init001, TestSize.Level1)
{
    int32_t ret = SubscribeProfileManager::GetInstance().Init();
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnInit001
 * @tc.desc: Test UnInit method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnInit001, TestSize.Level1)
{
    int32_t ret = SubscribeProfileManager::GetInstance().UnInit();
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange001
 * @tc.desc: Test NotifyProfileChange with empty dbKey
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange001, TestSize.Level1)
{
    ProfileType profileType = ProfileType::DEVICE_PROFILE;
    ChangeType changeType = ChangeType::ADD;
    std::string dbKey = "";
    std::string dbValue = "testValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange002
 * @tc.desc: Test NotifyProfileChange with DEVICE_PROFILE type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange002, TestSize.Level1)
{
    ProfileType profileType = ProfileType::DEVICE_PROFILE;
    ChangeType changeType = ChangeType::ADD;
    std::string dbKey = "dev#deviceId123";
    std::string dbValue = "deviceValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange003
 * @tc.desc: Test NotifyProfileChange with SERVICE_PROFILE type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange003, TestSize.Level1)
{
    ProfileType profileType = ProfileType::SERVICE_PROFILE;
    ChangeType changeType = ChangeType::UPDATE;
    std::string dbKey = "svr#deviceId#serviceName";
    std::string dbValue = "serviceValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange004
 * @tc.desc: Test NotifyProfileChange with CHAR_PROFILE type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange004, TestSize.Level1)
{
    ProfileType profileType = ProfileType::CHAR_PROFILE;
    ChangeType changeType = ChangeType::DELETE;
    std::string dbKey = "char#deviceId#serviceName#charKey";
    std::string dbValue = "charValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile001
 * @tc.desc: Test SubscribeDeviceProfile with default subscribeInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile002
 * @tc.desc: Test SubscribeDeviceProfile with valid subscribeInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile002, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(1);
    subscribeInfo.SetSubscribeKey("deviceId_test_001", "attribute_test_001");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile003
 * @tc.desc: Test SubscribeDeviceProfile with service profile key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile003, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(2);
    subscribeInfo.SetSubscribeKey("deviceId_002", "serviceName_002", "attribute_002");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile004
 * @tc.desc: Test SubscribeDeviceProfile with characteristic profile key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile004, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(3);
    subscribeInfo.SetSubscribeKey("deviceId_003", "serviceName_003", "charKey_003", "attribute_003");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile005
 * @tc.desc: Test SubscribeDeviceProfile with map parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile005, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    SubscribeInfo subscribeInfo1;
    subscribeInfo1.SetSaId(4);
    subscribeInfo1.SetSubscribeKey("deviceId_004", "attribute_004");
    subscribeInfo1.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfos["key1"] = subscribeInfo1;

    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile006
 * @tc.desc: Test SubscribeDeviceProfile with empty map
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile006, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnSubscribeDeviceProfile001
 * @tc.desc: Test UnSubscribeDeviceProfile with default subscribeInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnSubscribeDeviceProfile001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    int32_t ret = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnSubscribeDeviceProfile002
 * @tc.desc: Test UnSubscribeDeviceProfile with valid subscribeInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnSubscribeDeviceProfile002, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(5);
    subscribeInfo.SetSubscribeKey("deviceId_005", "attribute_005");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange005
 * @tc.desc: Test NotifyProfileChange with PROFILE_TYPE_MIN
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange005, TestSize.Level1)
{
    ProfileType profileType = ProfileType::PROFILE_TYPE_MIN;
    ChangeType changeType = ChangeType::ADD;
    std::string dbKey = "testKey";
    std::string dbValue = "testValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange006
 * @tc.desc: Test NotifyProfileChange with different change types
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange006, TestSize.Level1)
{
    std::string dbKey = "dev#deviceId789";
    std::string dbValue = "deviceValue789";

    int32_t ret1 = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::DEVICE_PROFILE, ChangeType::ADD, dbKey, dbValue);
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);

    int32_t ret2 = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::DEVICE_PROFILE, ChangeType::UPDATE, dbKey, dbValue);
    EXPECT_NE(ret2, DP_UPDATE_ACL_PROFILE_FAIL);

    int32_t ret3 = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::DEVICE_PROFILE, ChangeType::DELETE, dbKey, dbValue);
    EXPECT_NE(ret3, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange008
 * @tc.desc: Test NotifyProfileChange with long dbValue
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange008, TestSize.Level1)
{
    ProfileType profileType = ProfileType::SERVICE_PROFILE;
    ChangeType changeType = ChangeType::UPDATE;
    std::string dbKey = "svr#deviceId#serviceName";
    std::string dbValue;
    for (int i = 0; i < 100; ++i) {
        dbValue += "longvalue";
    }
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange009
 * @tc.desc: Test NotifyProfileChange with special characters in key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange009, TestSize.Level1)
{
    ProfileType profileType = ProfileType::CHAR_PROFILE;
    ChangeType changeType = ChangeType::DELETE;
    std::string dbKey = "char#deviceId#@#$%#charKey";
    std::string dbValue = "charValue";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(profileType, changeType, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile008
 * @tc.desc: Test SubscribeDeviceProfile with map containing multiple items
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile008, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    for (int i = 0; i < 5; ++i) {
        SubscribeInfo subscribeInfo;
        subscribeInfo.SetSaId(200 + i);
        subscribeInfo.SetSubscribeKey("deviceId_map_" + std::to_string(i), "attribute_map_" + std::to_string(i));
        subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
        subscribeInfos["key" + std::to_string(i)] = subscribeInfo;
    }
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnSubscribeDeviceProfile003
 * @tc.desc: Test UnSubscribeDeviceProfile with different saIds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnSubscribeDeviceProfile003, TestSize.Level1)
{
    SubscribeInfo subscribeInfo1;
    subscribeInfo1.SetSaId(300);
    subscribeInfo1.SetSubscribeKey("deviceId_unsub_300", "attribute_300");
    subscribeInfo1.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret1 = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo1);
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);

    SubscribeInfo subscribeInfo2;
    subscribeInfo2.SetSaId(301);
    subscribeInfo2.SetSubscribeKey("deviceId_unsub_301", "attribute_301");
    subscribeInfo2.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    int32_t ret2 = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo2);
    EXPECT_NE(ret2, DP_UPDATE_ACL_PROFILE_FAIL);

    SubscribeInfo subscribeInfo3;
    subscribeInfo3.SetSaId(302);
    subscribeInfo3.SetSubscribeKey("deviceId_unsub_302", "attribute_302");
    subscribeInfo3.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    int32_t ret3 = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo3);
    EXPECT_NE(ret3, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: InitAndUnInit001
 * @tc.desc: Test Init and UnInit sequence
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, InitAndUnInit001, TestSize.Level1)
{
    int32_t ret1 = SubscribeProfileManager::GetInstance().Init();
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);

    int32_t ret2 = SubscribeProfileManager::GetInstance().UnInit();
    EXPECT_NE(ret2, DP_UPDATE_ACL_PROFILE_FAIL);

    int32_t ret3 = SubscribeProfileManager::GetInstance().Init();
    EXPECT_NE(ret3, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeAndUnSubscribe001
 * @tc.desc: Test SubscribeDeviceProfile and UnSubscribeDeviceProfile in sequence
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeAndUnSubscribe001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(500);
    subscribeInfo.SetSubscribeKey("deviceId_seq_sub", "attribute_seq");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);

    int32_t ret1 = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);

    int32_t ret2 = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret2, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeAndUnSubscribe002
 * @tc.desc: Test SubscribeDeviceProfile and UnSubscribeDeviceProfile with map
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeAndUnSubscribe002, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    for (int i = 0; i < 3; ++i) {
        SubscribeInfo subscribeInfo;
        subscribeInfo.SetSaId(600 + i);
        subscribeInfo.SetSubscribeKey("deviceId_map_sub_" + std::to_string(i), "attr_" + std::to_string(i));
        subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
        subscribeInfos["mapKey" + std::to_string(i)] = subscribeInfo;
    }

    int32_t ret1 = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChangeWithEmptyValue001
 * @tc.desc: Test NotifyProfileChange with empty dbValue
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChangeWithEmptyValue001, TestSize.Level1)
{
    std::string dbKey = "dev#deviceId_empty_value";
    std::string dbValue = "";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::DEVICE_PROFILE, ChangeType::ADD, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChangeWithEmptyValue002
 * @tc.desc: Test NotifyProfileChange with empty dbKey and dbValue
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChangeWithEmptyValue002, TestSize.Level1)
{
    std::string dbKey = "";
    std::string dbValue = "";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::SERVICE_PROFILE, ChangeType::UPDATE, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange010
 * @tc.desc: Test NotifyProfileChange with device profile key format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange010, TestSize.Level1)
{
    std::string dbKey = "dev#1234567890abcdef";
    std::string dbValue = R"({"deviceName":"test","deviceType":"phone"})";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::DEVICE_PROFILE, ChangeType::ADD, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange011
 * @tc.desc: Test NotifyProfileChange with service profile key format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange011, TestSize.Level1)
{
    std::string dbKey = "svr#deviceId#serviceName#serviceType";
    std::string dbValue = R"({"serviceName":"test","serviceType":"system"})";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::SERVICE_PROFILE, ChangeType::UPDATE, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange012
 * @tc.desc: Test NotifyProfileChange with characteristic profile key format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange012, TestSize.Level1)
{
    std::string dbKey = "char#deviceId#serviceName#charKey";
    std::string dbValue = R"({"key":"value","status":"enabled"})";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::CHAR_PROFILE, ChangeType::DELETE, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile009
 * @tc.desc: Test SubscribeDeviceProfile with long subscribe key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile009, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(3000);
    std::string longKey;
    for (int i = 0; i < 100; ++i) {
        longKey += "longdeviceid_";
    }
    subscribeInfo.SetSubscribeKey(longKey, "attribute_3000");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfile010
 * @tc.desc: Test SubscribeDeviceProfile with special characters in key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile010, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(4000);
    subscribeInfo.SetSubscribeKey("device!@#$%^&*()", "attribute_special");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnSubscribeDeviceProfile004
 * @tc.desc: Test UnSubscribeDeviceProfile with non-existent subscription
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnSubscribeDeviceProfile004, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(999999);
    subscribeInfo.SetSubscribeKey("nonexistent_device", "nonexistent_attr");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    int32_t ret = SubscribeProfileManager::GetInstance().UnSubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}
    
/**
 * @tc.name: SubscribeDeviceProfileMap001
 * @tc.desc: Test SubscribeDeviceProfile with map containing single item
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfileMap001, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(5000);
    subscribeInfo.SetSubscribeKey("single_map_device", "single_map_attr");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfos["single_key"] = subscribeInfo;
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeDeviceProfileMap002
 * @tc.desc: Test SubscribeDeviceProfile with large map
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfileMap002, TestSize.Level1)
{
    std::map<std::string, SubscribeInfo> subscribeInfos;
    for (int i = 0; i < 50; ++i) {
        SubscribeInfo subscribeInfo;
        subscribeInfo.SetSaId(6000 + i);
        subscribeInfo.SetSubscribeKey("large_map_device_" + std::to_string(i), "attr_" + std::to_string(i));
        subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
        subscribeInfos["large_key_" + std::to_string(i)] = subscribeInfo;
    }
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfos);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange013
 * @tc.desc: Test NotifyProfileChange with JSON-like value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange013, TestSize.Level1)
{
    std::string dbKey = "json_key";
    std::string dbValue = R"({"name":"test","version":"1.0","features":["a","b","c"]})";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::SERVICE_PROFILE, ChangeType::ADD, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: NotifyProfileChange014
 * @tc.desc: Test NotifyProfileChange with XML-like value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, NotifyProfileChange014, TestSize.Level1)
{
    std::string dbKey = "xml_key";
    std::string dbValue = "<root><name>test</name><version>1.0</version></root>";
    int32_t ret = SubscribeProfileManager::GetInstance().NotifyProfileChange(
        ProfileType::CHAR_PROFILE, ChangeType::UPDATE, dbKey, dbValue);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: Init002
 * @tc.desc: Test Init after UnInit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, Init002, TestSize.Level1)
{
    SubscribeProfileManager::GetInstance().UnInit();
    int32_t ret = SubscribeProfileManager::GetInstance().Init();
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: UnInit002
 * @tc.desc: Test UnInit multiple times
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, UnInit002, TestSize.Level1)
{
    SubscribeProfileManager::GetInstance().Init();
    int32_t ret1 = SubscribeProfileManager::GetInstance().UnInit();
    EXPECT_NE(ret1, DP_UPDATE_ACL_PROFILE_FAIL);
    int32_t ret2 = SubscribeProfileManager::GetInstance().UnInit();
    EXPECT_NE(ret2, DP_UPDATE_ACL_PROFILE_FAIL);
    SubscribeProfileManager::GetInstance().Init();
}
    

/**
 * @tc.name: SubscribeDeviceProfile011
 * @tc.desc: Test SubscribeDeviceProfile with negative sa id
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeDeviceProfile011, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(-1);
    subscribeInfo.SetSubscribeKey("negative_device", "negative_attr");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeWithTrustKey001
 * @tc.desc: Test SubscribeDeviceProfile with trust device profile key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeWithTrustKey001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(7000);
    subscribeInfo.SetSubscribeKey("trust_profile_key_001");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeWithTrustKey002
 * @tc.desc: Test SubscribeDeviceProfile with trust device profile key and userId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeWithTrustKey002, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(7001);
    subscribeInfo.SetSubscribeKey("trust_profile_key_002", 100);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeWithMultipleChangeTypes001
 * @tc.desc: Test SubscribeDeviceProfile with multiple change types
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeWithMultipleChangeTypes001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(8000);
    subscribeInfo.SetSubscribeKey("multi_change_device", "multi_change_attr");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeWithServiceKey001
 * @tc.desc: Test SubscribeDeviceProfile with service profile key format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeWithServiceKey001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(9000);
    subscribeInfo.SetSubscribeKey("device_9000", "service_9000", "attribute_9000");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: SubscribeWithCharacteristicKey001
 * @tc.desc: Test SubscribeDeviceProfile with characteristic profile key format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, SubscribeWithCharacteristicKey001, TestSize.Level1)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(10000);
    subscribeInfo.SetSubscribeKey("device_10000", "service_10000", "charKey_10000", "attribute_10000");
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    int32_t ret = SubscribeProfileManager::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

/**
 * @tc.name: FinalCleanup
 * @tc.desc: Final cleanup test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SubscribeProfileManagerNewTest, FinalCleanup, TestSize.Level1)
{
    int32_t ret = SubscribeProfileManager::GetInstance().UnInit();
    EXPECT_NE(ret, DP_UPDATE_ACL_PROFILE_FAIL);
}

} // namespace DistributedDeviceProfile
} // namespace OHOS
