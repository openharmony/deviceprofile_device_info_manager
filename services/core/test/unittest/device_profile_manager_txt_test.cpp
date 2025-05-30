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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include <string>
#include <vector>
#include <new>
#include "gtest/gtest.h"
#include "refbase.h"
#include "iremote_stub.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_log.h"
#include "distributed_device_profile_enums.h"
#include "device_profile.h"
#include "service_profile.h"
#include "content_sensor_manager_utils.h"
#include "characteristic_profile.h"
#include "i_sync_completed_callback.h"
#include "sync_completed_callback_stub.h"
#include "device_profile_manager.h"
#include "kv_adapter.h"
#include "profile_cache.h"
#undef private
#undef protected

namespace OHOS {
namespace DistributedDeviceProfile {
namespace {
const int32_t PERMS_NUM = 2;
}
using namespace testing::ext;
using namespace std;

class DeviceProfileManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceProfileManagerTest::SetUpTestCase(void) {
}

void DeviceProfileManagerTest::TearDownTestCase(void) {
}

void DeviceProfileManagerTest::SetUp()
{
    const char *perms[PERMS_NUM] = {
        "ohos.permission.DISTRIBUTED_DATASYNC",
        "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER"
    };
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = PERMS_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "deviceprofile",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

void DeviceProfileManagerTest::TearDown() {
}

class SyncCallback : public SyncCompletedCallbackStub {
public:
    void OnSyncCompleted(const map<string, SyncStatus>& syncResults) {
    }
};

/**
 * @tc.name: Init001
 * @tc.desc: Init succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, Init001, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().Init();
    EXPECT_EQ(ret, DP_SUCCESS);
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: UnInit001
 * @tc.desc: UnInit succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, UnInit001, TestSize.Level1)
{
    int32_t ret = DeviceProfileManager::GetInstance().UnInit();
    EXPECT_EQ(ret, DP_SUCCESS);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: UnInit002
 * @tc.desc: deviceProfileStore_ is nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, UnInit002, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().UnInit();
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: ReInit001
 * @tc.desc: ReInit succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, ReInit001, TestSize.Level1)
{
    int32_t ret = DeviceProfileManager::GetInstance().ReInit();
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutDeviceProfile001
 * @tc.desc: PutDeviceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile001, TestSize.Level1)
{
    DeviceProfile deviceProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "anything";
    deviceProfile.SetDeviceId("anything");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetStorageCapability(1);
    deviceProfile.SetOsSysCap("anything");
    deviceProfile.SetOsApiLevel(1);
    deviceProfile.SetOsVersion("anything");
    deviceProfile.SetOsType(1);

    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutDeviceProfile002
 * @tc.desc: PutDeviceProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile002, TestSize.Level1)
{
    DeviceProfile deviceProfile;
    deviceProfile.SetDeviceId("");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetStorageCapability(1);
    deviceProfile.SetOsSysCap("anything");
    deviceProfile.SetOsApiLevel(1);
    deviceProfile.SetOsVersion("anything");
    deviceProfile.SetOsType(1);

    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: PutDeviceProfile003
 * @tc.desc: PutDeviceProfile failed, the profile is exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile003, TestSize.Level1)
{
    DeviceProfile deviceProfile1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "anything1";
    deviceProfile1.SetDeviceId("anything1");
    deviceProfile1.SetDeviceTypeName("anything");
    deviceProfile1.SetDeviceTypeId(0);
    deviceProfile1.SetDeviceName("anything");
    deviceProfile1.SetManufactureName("anything");
    deviceProfile1.SetDeviceModel("anything");
    deviceProfile1.SetStorageCapability(1);
    deviceProfile1.SetOsSysCap("anything");
    deviceProfile1.SetOsApiLevel(1);
    deviceProfile1.SetOsVersion("anything");
    deviceProfile1.SetOsType(1);

    DeviceProfile deviceProfile2;
    deviceProfile2.SetDeviceId("anything1");
    deviceProfile2.SetDeviceTypeName("anything");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("anything");
    deviceProfile2.SetManufactureName("anything");
    deviceProfile2.SetDeviceModel("anything");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("anything");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("anything");
    deviceProfile2.SetOsType(1);

    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile1);
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutDeviceProfile004
 * @tc.desc: PutDeviceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile004, TestSize.Level1)
{
    DeviceProfile deviceProfile10;
    deviceProfile10.SetDeviceId("anything10");
    deviceProfile10.SetDeviceTypeName("anything");
    deviceProfile10.SetDeviceTypeId(0);
    deviceProfile10.SetDeviceName("anything");
    deviceProfile10.SetManufactureName("anything");
    deviceProfile10.SetDeviceModel("anything");
    deviceProfile10.SetStorageCapability(1);
    deviceProfile10.SetOsSysCap("anything");
    deviceProfile10.SetOsApiLevel(1);
    deviceProfile10.SetOsVersion("anything");
    deviceProfile10.SetOsType(1);

    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile10);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: PutDeviceProfile005
 * @tc.desc: PutDeviceProfile failed, PutDeviceProfile fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile005, TestSize.Level1)
{
    DeviceProfile deviceProfile11;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "anything11";
    deviceProfile11.SetDeviceId("anything11");
    deviceProfile11.SetDeviceTypeName("anything");
    deviceProfile11.SetDeviceTypeId(0);
    deviceProfile11.SetDeviceName("anything");
    deviceProfile11.SetManufactureName("anything");
    deviceProfile11.SetDeviceModel("anything");
    deviceProfile11.SetStorageCapability(1);
    deviceProfile11.SetOsSysCap("anything");
    deviceProfile11.SetOsApiLevel(1);
    deviceProfile11.SetOsVersion("anything");
    deviceProfile11.SetOsType(1);

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: PutDeviceProfile006
 * @tc.desc: PutDeviceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutDeviceProfile006, TestSize.Level1)
{
    DeviceProfile deviceProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "anything111";
    deviceProfile.SetDeviceId("anything111");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetStorageCapability(1);
    deviceProfile.SetOsSysCap("anything");
    deviceProfile.SetOsApiLevel(1);
    deviceProfile.SetOsVersion("anything");
    deviceProfile.SetOsType(1);
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile);
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
}

/**
 * @tc.name: PutServiceProfile001
 * @tc.desc: PutServiceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile001, TestSize.Level1)
{
    ServiceProfile serviceProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId";
    serviceProfile.SetDeviceId("deviceId");
    serviceProfile.SetServiceName("serviceName");
    serviceProfile.SetServiceType("serviceType");

    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutServiceProfile002
 * @tc.desc: PutServiceProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile002, TestSize.Level1)
{
    ServiceProfile serviceProfile;
    serviceProfile.SetDeviceId("");
    serviceProfile.SetServiceName("serviceName");
    serviceProfile.SetServiceType("serviceType");

    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: PutServiceProfile003
 * @tc.desc: PutServiceProfile failed, the profile is exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile003, TestSize.Level1)
{
    ServiceProfile serviceProfile1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId1";
    serviceProfile1.SetDeviceId("deviceId1");
    serviceProfile1.SetServiceName("serviceName");
    serviceProfile1.SetServiceType("serviceType");

    ServiceProfile serviceProfile2;
    serviceProfile2.SetDeviceId("deviceId1");
    serviceProfile2.SetServiceName("serviceName");
    serviceProfile2.SetServiceType("serviceType");

    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile1);
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile2);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutServiceProfile004
 * @tc.desc: PutServiceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile004, TestSize.Level1)
{
    ServiceProfile serviceProfile10;
    serviceProfile10.SetDeviceId("deviceId10");
    serviceProfile10.SetServiceName("serviceName10");
    serviceProfile10.SetServiceType("serviceType10");

    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile10);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: PutServiceProfile005
 * @tc.desc: PutServiceProfile failed, PutServiceProfile fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile005, TestSize.Level1)
{
    ServiceProfile serviceProfile11;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId11";
    serviceProfile11.SetDeviceId("deviceId11");
    serviceProfile11.SetServiceName("serviceName11");
    serviceProfile11.SetServiceType("serviceType11");

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: PutServiceProfile006
 * @tc.desc: isFirst_.load() == true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile006, TestSize.Level1)
{
    ServiceProfile serviceProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId112";
    serviceProfile.SetDeviceId("deviceId112");
    serviceProfile.SetServiceName("serviceName");
    serviceProfile.SetServiceType("serviceType");
    DeviceProfileManager::GetInstance().isFirst_.store(true);
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile);
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutServiceProfile007
 * @tc.desc: deviceProfileStore_ == nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfile007, TestSize.Level1)
{
    ServiceProfile serviceProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId113";
    serviceProfile.SetDeviceId("deviceId113");
    serviceProfile.SetServiceName("serviceName");
    serviceProfile.SetServiceType("serviceType");
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile);
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
}

/**
 * @tc.name: PutServiceProfileBatch001
 * @tc.desc: PutServiceProfileBatch succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfileBatch001, TestSize.Level1)
{
    vector<ServiceProfile> serviceProfiles;
    ServiceProfile serviceProfile1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId2";
    serviceProfile1.SetDeviceId("deviceId2");
    serviceProfile1.SetServiceName("serviceName2");
    serviceProfile1.SetServiceType("serviceType2");
    serviceProfiles.push_back(serviceProfile1);

    ServiceProfile serviceProfile2;
    serviceProfile2.SetDeviceId("deviceId3");
    serviceProfile2.SetServiceName("serviceName3");
    serviceProfile2.SetServiceType("serviceType3");
    serviceProfiles.push_back(serviceProfile2);

    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfileBatch(serviceProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    ret = DeviceProfileManager::GetInstance().PutServiceProfileBatch(serviceProfiles);
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
}

/**
 * @tc.name: PutServiceProfileBatch002
 * @tc.desc: PutServiceProfileBatch succeed, but first profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutServiceProfileBatch002, TestSize.Level1)
{
    vector<ServiceProfile> serviceProfiles;
    ServiceProfile serviceProfile1;
    serviceProfile1.SetDeviceId("");
    serviceProfile1.SetServiceName("serviceName");
    serviceProfile1.SetServiceType("serviceType");
    serviceProfiles.push_back(serviceProfile1);

    ServiceProfile serviceProfile4;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId4";
    serviceProfile4.SetDeviceId("deviceId4");
    serviceProfile4.SetServiceName("serviceName4");
    serviceProfile4.SetServiceType("serviceType4");
    serviceProfiles.push_back(serviceProfile4);

    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfileBatch(serviceProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutCharacteristicProfile001
 * @tc.desc: PutCharacteristicProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile001, TestSize.Level1)
{
    CharacteristicProfile charProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId";
    charProfile.SetDeviceId("deviceId");
    charProfile.SetServiceName("serviceName");
    charProfile.SetCharacteristicKey("characteristicKey");
    charProfile.SetCharacteristicValue("characteristicValue");

    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutCharacteristicProfile002
 * @tc.desc: PutCharacteristicProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile002, TestSize.Level1)
{
    CharacteristicProfile charProfile;
    charProfile.SetDeviceId("");
    charProfile.SetServiceName("serviceName");
    charProfile.SetCharacteristicKey("characteristicKey");
    charProfile.SetCharacteristicValue("characteristicValue");

    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: PutCharacteristicProfile003
 * @tc.desc: PutCharacteristicProfile failed, the profile is exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile003, TestSize.Level1)
{
    CharacteristicProfile charProfile1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId1";
    charProfile1.SetDeviceId("deviceId1");
    charProfile1.SetServiceName("serviceName");
    charProfile1.SetCharacteristicKey("characteristicKey");
    charProfile1.SetCharacteristicValue("characteristicValue");

    CharacteristicProfile charProfile2;
    charProfile2.SetDeviceId("deviceId1");
    charProfile2.SetServiceName("serviceName");
    charProfile2.SetCharacteristicKey("characteristicKey");
    charProfile2.SetCharacteristicValue("characteristicValue");

    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile1);
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile2);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: PutCharacteristicProfile004
 * @tc.desc: PutCharacteristicProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile004, TestSize.Level1)
{
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId10";
    CharacteristicProfile charProfile10;
    charProfile10.SetDeviceId("deviceId10");
    charProfile10.SetServiceName("serviceName10");
    charProfile10.SetCharacteristicKey("characteristicKey10");
    charProfile10.SetCharacteristicValue("characteristicValue10");

    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile10);
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: PutCharacteristicProfile005
 * @tc.desc: PutCharacteristicProfile failed, PutCharacteristicProfile fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile005, TestSize.Level1)
{
    CharacteristicProfile charProfile11;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId11";
    charProfile11.SetDeviceId("deviceId11");
    charProfile11.SetServiceName("serviceName11");
    charProfile11.SetCharacteristicKey("characteristicKey11");
    charProfile11.SetCharacteristicValue("characteristicValue11");

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: PutCharacteristicProfile006
 * @tc.desc: isFirst_.load() = true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile006, TestSize.Level1)
{
    CharacteristicProfile charProfile;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId111";
    charProfile.SetDeviceId("deviceId111");
    charProfile.SetServiceName("serviceName");
    charProfile.SetCharacteristicKey("characteristicKey");
    charProfile.SetCharacteristicValue("characteristicValue");
    DeviceProfileManager::GetInstance().isFirst_.store(true);
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: PutCharacteristicProfileBatch001
 * @tc.desc: PutCharacteristicProfileBatch succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfileBatch001, TestSize.Level1)
{
    vector<CharacteristicProfile> charProfiles;
    CharacteristicProfile charProfile1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId2";
    charProfile1.SetDeviceId("deviceId2");
    charProfile1.SetServiceName("serviceName2");
    charProfile1.SetCharacteristicKey("characteristicKey2");
    charProfile1.SetCharacteristicValue("characteristicValue2");
    charProfiles.push_back(charProfile1);

    CharacteristicProfile charProfile2;
    charProfile2.SetDeviceId("deviceId3");
    charProfile2.SetServiceName("serviceName3");
    charProfile2.SetCharacteristicKey("characteristicKey3");
    charProfile2.SetCharacteristicValue("characteristicValue3");
    charProfiles.push_back(charProfile2);

    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfileBatch(charProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);

    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    ret = DeviceProfileManager::GetInstance().PutCharacteristicProfileBatch(charProfiles);
    EXPECT_EQ(ret, DP_KV_DB_PTR_NULL);
}

/**
 * @tc.name: PutCharacteristicProfileBatch002
 * @tc.desc: PutCharacteristicProfileBatch succeed, but first profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfileBatch002, TestSize.Level1)
{
    vector<CharacteristicProfile> charProfiles;
    CharacteristicProfile charProfile1;
    charProfile1.SetDeviceId("");
    charProfile1.SetServiceName("serviceName");
    charProfile1.SetCharacteristicKey("characteristicKey");
    charProfile1.SetCharacteristicValue("characteristicValue");
    charProfiles.push_back(charProfile1);

    CharacteristicProfile charProfile4;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId4";
    charProfile4.SetDeviceId("deviceId4");
    charProfile4.SetServiceName("serviceName4");
    charProfile4.SetCharacteristicKey("characteristicKey4");
    charProfile4.SetCharacteristicValue("characteristicValue4");
    charProfiles.push_back(charProfile4);

    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfileBatch(charProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetDeviceProfile001
 * @tc.desc: GetDeviceProfile succeed, GetDeviceProfile in cache.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetDeviceProfile001, TestSize.Level1)
{
    DeviceProfile deviceProfile2;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId";
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    deviceProfile2.SetDeviceId(deviceId);
    deviceProfile2.SetDeviceTypeName("anything");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("anything");
    deviceProfile2.SetManufactureName("anything");
    deviceProfile2.SetDeviceModel("anything");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("anything");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("anything");
    deviceProfile2.SetOsType(1);

    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);

    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
    outDeviceProfile.SetDeviceId(deviceId);
    string outDeviceId = outDeviceProfile.GetDeviceId();
    outDeviceProfile.GetDeviceTypeName();
    outDeviceProfile.GetDeviceTypeId();
    outDeviceProfile.GetDeviceName();
    outDeviceProfile.GetManufactureName();
    outDeviceProfile.GetDeviceModel();
    outDeviceProfile.GetStorageCapability();
    outDeviceProfile.GetOsSysCap();
    outDeviceProfile.GetOsApiLevel();
    outDeviceProfile.GetOsVersion();
    outDeviceProfile.GetOsType();
    EXPECT_EQ(outDeviceId, deviceId);
}

/**
 * @tc.name: GetDeviceProfile002
 * @tc.desc: GetDeviceProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetDeviceProfile002, TestSize.Level1)
{
    string deviceId = "";
    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_NE(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetDeviceProfile003
 * @tc.desc: GetDeviceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetDeviceProfile003, TestSize.Level1)
{
    string deviceId = "anything12";
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetDeviceProfile004
 * @tc.desc: GetDeviceProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetDeviceProfile004, TestSize.Level1)
{
    string deviceId = "#anything13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: GetServiceProfile001
 * @tc.desc: GetServiceProfile succeed, GetServiceProfile in cache.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetServiceProfile001, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    ServiceProfile serviceProfile5;
    serviceProfile5.SetDeviceId(deviceId);
    serviceProfile5.SetServiceName("serviceName5");
    serviceProfile5.SetServiceType("serviceType5");
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile5);

    string serviceName = "serviceName5";
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);

    string outDeviceId = outServiceProfile.GetDeviceId();
    outServiceProfile.GetServiceName();
    outServiceProfile.GetServiceType();
    EXPECT_EQ(outDeviceId, deviceId);
}

/**
 * @tc.name: GetServiceProfile002
 * @tc.desc: GetServiceProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetServiceProfile002, TestSize.Level1)
{
    string deviceId = "";
    string serviceName = "serviceName";
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_NE(ret, DP_SUCCESS);

    deviceId = "deviceId";
    serviceName = "";
    ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_NE(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetServiceProfile003
 * @tc.desc: GetServiceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetServiceProfile003, TestSize.Level1)
{
    string deviceId = "deviceId12";
    string serviceName = "serviceName12";
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetServiceProfile004
 * @tc.desc: GetServiceProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetServiceProfile004, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    string serviceName = "serviceName13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: GetCharacteristicProfile001
 * @tc.desc: GetCharacteristicProfile succeed, GetCharProfile in cache.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetCharacteristicProfile001, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    CharacteristicProfile charProfile5;
    charProfile5.SetDeviceId(deviceId);
    charProfile5.SetServiceName("serviceName5");
    charProfile5.SetCharacteristicKey("characteristicKey5");
    charProfile5.SetCharacteristicValue("characteristicValue5");
    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile5);

    string serviceName = "serviceName5";
    string characteristicKey = "characteristicKey5";
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_EQ(ret, DP_SUCCESS);

    string outDeviceId = outCharProfile.GetDeviceId();
    outCharProfile.GetServiceName();
    outCharProfile.GetCharacteristicKey();
    outCharProfile.GetCharacteristicValue();
    EXPECT_EQ(outDeviceId, deviceId);
}

/**
 * @tc.name: GetCharacteristicProfile002
 * @tc.desc: GetCharacteristicProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetCharacteristicProfile002, TestSize.Level1)
{
    string deviceId = "";
    string serviceName = "serviceName";
    string characteristicKey = "characteristicKey";
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_NE(ret, DP_SUCCESS);

    deviceId = "deviceId";
    serviceName = "serviceName";
    characteristicKey = "";
    ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_NE(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetCharacteristicProfile003
 * @tc.desc: GetCharacteristicProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetCharacteristicProfile003, TestSize.Level1)
{
    string deviceId = "deviceId12";
    string serviceName = "serviceName12";
    string characteristicKey = "characteristicKey12";
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetCharacteristicProfile004
 * @tc.desc: GetCharacteristicProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetCharacteristicProfile004, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    string serviceName = "serviceName13";
    string characteristicKey = "characteristicKey13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: DeleteServiceProfile001
 * @tc.desc: DeleteServiceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteServiceProfile001, TestSize.Level1)
{
    ServiceProfile serviceProfile6;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId6";
    serviceProfile6.SetDeviceId("deviceId6");
    serviceProfile6.SetServiceName("serviceName6");
    serviceProfile6.SetServiceType("serviceType6");
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile6);

    string deviceId = "deviceId6";
    string serviceName = "serviceName6";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: DeleteServiceProfile002
 * @tc.desc: DeleteServiceProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteServiceProfile002, TestSize.Level1)
{
    string deviceId = "";
    string serviceName = "serviceName";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);

    deviceId = "deviceId";
    serviceName = "";
    ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: DeleteServiceProfile003
 * @tc.desc: DeleteServiceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteServiceProfile003, TestSize.Level1)
{
    string deviceId = "deviceId14";
    string serviceName = "serviceName14";
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: DeleteServiceProfile004
 * @tc.desc: DeleteServiceProfile failed, DeleteServiceProfile fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteServiceProfile004, TestSize.Level1)
{
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId15";
    string deviceId = "deviceId15";
    string serviceName = "serviceName15";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    int32_t ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_DEL_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: SetIsMultiUser001
 * @tc.desc: SetIsMultiUser Success, SetIsMultiUser Success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SetIsMultiUser001, TestSize.Level1)
{
    ServiceProfile serviceProfile7;
    bool isMultiUser = true;
    serviceProfile7.SetIsMultiUser(isMultiUser);
    EXPECT_EQ(true, serviceProfile7.isMultiUser_);
}

/**
 * @tc.name: IsMultiUser001
 * @tc.desc: IsMultiUser Success, IsMultiUser Success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUser001, TestSize.Level1)
{
    ServiceProfile serviceProfile8;
    bool isMultiUser = true;
    serviceProfile8.SetIsMultiUser(isMultiUser);
    bool ret = serviceProfile8.IsMultiUser();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: SetUserId001
 * @tc.desc: SetUserId Success, SetUserId Success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SetUserId001, TestSize.Level1)
{
    ServiceProfile serviceProfile9;
    int32_t userId = 1;
    serviceProfile9.SetUserId(userId);
    EXPECT_EQ(1, serviceProfile9.userId_);
}

/**
 * @tc.name: GetUserId001
 * @tc.desc: GetUserId Success, GetUserId Success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetUserId001, TestSize.Level1)
{
    ServiceProfile serviceProfile9;
    int32_t userId = 1;
    serviceProfile9.SetUserId(userId);
    bool ret = serviceProfile9.GetUserId();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: DeleteCharacteristicProfile001
 * @tc.desc: DeleteCharacteristicProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteCharacteristicProfile001, TestSize.Level1)
{
    CharacteristicProfile charProfile6;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId6";
    charProfile6.SetDeviceId("deviceId6");
    charProfile6.SetServiceName("serviceName6");
    charProfile6.SetCharacteristicKey("characteristicKey6");
    charProfile6.SetCharacteristicValue("characteristicValue6");
    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile6);

    string deviceId = "deviceId6";
    string serviceName = "serviceName6";
    string characteristicKey = "characteristicKey6";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: DeleteCharacteristicProfile002
 * @tc.desc: DeleteCharacteristicProfile failed, the profile is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteCharacteristicProfile002, TestSize.Level1)
{
    string deviceId = "";
    string serviceName = "serviceName";
    string characteristicKey = "characteristicKey";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);

    deviceId = "deviceId";
    serviceName = "serviceName";
    characteristicKey = "";
    ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
}

/**
 * @tc.name: DeleteCharacteristicProfile003
 * @tc.desc: DeleteCharacteristicProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteCharacteristicProfile003, TestSize.Level1)
{
    string deviceId = "deviceId14";
    string serviceName = "serviceName14";
    string characteristicKey = "characteristicKey14";
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: DeleteCharacteristicProfile004
 * @tc.desc: DeleteCharacteristicProfile failed, DeleteServiceProfile fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteCharacteristicProfile004, TestSize.Level1)
{
    ContentSensorManagerUtils::GetInstance().localUdid_ = "deviceId15";
    string deviceId = "deviceId15";
    string serviceName = "serviceName15";
    string characteristicKey = "characteristicKey15";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    int32_t ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_DEL_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: GetAllDeviceProfile001
 * @tc.desc: GetAllDeviceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllDeviceProfile001, TestSize.Level1)
{
    vector<DeviceProfile> deviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllDeviceProfile(deviceProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetAllDeviceProfile002
 * @tc.desc: GetAllDeviceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllDeviceProfile002, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    vector<DeviceProfile> deviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllDeviceProfile(deviceProfiles);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetAllDeviceProfile003
 * @tc.desc: GetAllDeviceProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllDeviceProfile003, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    vector<DeviceProfile> deviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllDeviceProfile(deviceProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: GetAllServiceProfile001
 * @tc.desc: GetAllServiceProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllServiceProfile001, TestSize.Level1)
{
    vector<ServiceProfile> serviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllServiceProfile(serviceProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetAllServiceProfile002
 * @tc.desc: GetAllServiceProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllServiceProfile002, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    vector<ServiceProfile> serviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllServiceProfile(serviceProfiles);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetAllServiceProfile003
 * @tc.desc: GetAllServiceProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllServiceProfile003, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    vector<ServiceProfile> serviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllServiceProfile(serviceProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: GetAllCharacteristicProfile001
 * @tc.desc: GetAllCharacteristicProfile succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllCharacteristicProfile001, TestSize.Level1)
{
    vector<CharacteristicProfile> charProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllCharacteristicProfile(charProfiles);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: GetAllCharacteristicProfile002
 * @tc.desc: GetAllCharacteristicProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllCharacteristicProfile002, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    vector<CharacteristicProfile> charProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllCharacteristicProfile(charProfiles);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetAllCharacteristicProfile003
 * @tc.desc: GetAllCharacteristicProfile failed, Get data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetAllCharacteristicProfile003, TestSize.Level1)
{
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
    vector<CharacteristicProfile> charProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllCharacteristicProfile(charProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
    DeviceProfileManager::GetInstance().isFirst_.store(false);
}

/**
 * @tc.name: SyncDeviceProfile001
 * @tc.desc: SyncDeviceProfile failed, Params is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SyncDeviceProfile001, TestSize.Level1)
{
    DistributedDeviceProfile::DpSyncOptions syncOptions;
    OHOS::sptr<OHOS::IRemoteObject> syncCb = nullptr;

    syncOptions.AddDevice("deviceId1");
    syncOptions.AddDevice("deviceId2");
    syncOptions.SetSyncMode(SyncMode::MIN);

    int32_t errCode = DeviceProfileManager::GetInstance().SyncDeviceProfile(syncOptions, syncCb);
    EXPECT_EQ(errCode, DP_INVALID_PARAMS);
}

/**
 * @tc.name: SyncDeviceProfile002
 * @tc.desc: SyncDeviceProfile failed, Params is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SyncDeviceProfile002, TestSize.Level1)
{
    DistributedDeviceProfile::DpSyncOptions syncOptions;
    OHOS::sptr<OHOS::IRemoteObject> syncCb = OHOS::sptr<SyncCompletedCallbackStub>(new SyncCallback());

    syncOptions.AddDevice("deviceId1");
    syncOptions.AddDevice("deviceId2");
    syncOptions.SetSyncMode(SyncMode::MAX);

    int32_t errCode = DeviceProfileManager::GetInstance().SyncDeviceProfile(syncOptions, syncCb);
    EXPECT_EQ(errCode, DP_INVALID_PARAMS);
}

/**
 * @tc.name: DeviceProfileMarshalling001
 * @tc.desc: DeviceProfile Marshalling and UnMarshalling succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeviceProfileMarshalling001, TestSize.Level1)
{
    OHOS::MessageParcel data;
    DeviceProfile deviceProfile;
    deviceProfile.SetDeviceId("anything");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetStorageCapability(1);
    deviceProfile.SetOsSysCap("anything");
    deviceProfile.SetOsApiLevel(1);
    deviceProfile.SetOsVersion("anything");
    deviceProfile.SetOsType(1);

    bool res1 = deviceProfile.Marshalling(data);
    EXPECT_EQ(true, res1);

    bool res2 = deviceProfile.UnMarshalling(data);
    EXPECT_EQ(true, res2);
}

/**
 * @tc.name: DeviceProfileOperator001
 * @tc.desc: DeviceProfileOperator true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeviceProfileOperator001, TestSize.Level1)
{
    DeviceProfile deviceProfile1;
    deviceProfile1.SetDeviceId("anything1");
    deviceProfile1.SetDeviceTypeName("anything1");
    deviceProfile1.SetDeviceTypeId(0);
    deviceProfile1.SetDeviceName("anything1");
    deviceProfile1.SetManufactureName("anything1");
    deviceProfile1.SetDeviceModel("anything1");
    deviceProfile1.SetStorageCapability(1);
    deviceProfile1.SetOsSysCap("anything1");
    deviceProfile1.SetOsApiLevel(1);
    deviceProfile1.SetOsVersion("anything1");
    deviceProfile1.SetOsType(1);

    DeviceProfile deviceProfile2;
    deviceProfile2.SetDeviceId("anything2");
    deviceProfile2.SetDeviceTypeName("anything2");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("anything2");
    deviceProfile2.SetManufactureName("anything2");
    deviceProfile2.SetDeviceModel("anything2");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("anything2");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("anything2");
    deviceProfile2.SetOsType(1);

    bool res = deviceProfile1 != deviceProfile2;
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: DeviceProfileDump001
 * @tc.desc: DeviceProfileDump succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeviceProfileDump001, TestSize.Level1)
{
    DeviceProfile deviceProfile;
    deviceProfile.SetDeviceId("anything");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetStorageCapability(1);
    deviceProfile.SetOsSysCap("anything");
    deviceProfile.SetOsApiLevel(1);
    deviceProfile.SetOsVersion("anything");
    deviceProfile.SetOsType(1);

    string strJson = deviceProfile.dump();
    char fistChar = strJson.front();
    char lastChar = strJson.back();
    EXPECT_EQ('{', fistChar);
    EXPECT_EQ('}', lastChar);
}

/**
 * @tc.name: ServiceProfileConstructor001
 * @tc.desc: ServiceProfileConstructor succeed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, ServiceProfileConstructor001, TestSize.Level1)
{
    ServiceProfile serviceProfile = ServiceProfile("deviceId", "serviceName", "serviceType");
    EXPECT_EQ("deviceId", serviceProfile.GetDeviceId());
}

/**
 * @tc.name: LoadDpSyncAdapter001
 * @tc.desc: LoadDpSyncAdapter first branch.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, LoadDpSyncAdapter001, TestSize.Level1)
{
    char path[PATH_MAX + 1] = {0x00};
    std::string soName = "/system/lib/libdeviceprofileadapter.z.so";
    bool ret = false;
    if ((soName.length() == 0) || (soName.length() > PATH_MAX) || (realpath(soName.c_str(), path) == nullptr)) {
        ret = true;
    } else {
        DeviceProfileManager::GetInstance().isAdapterSoLoaded_ = true;
        ret = DeviceProfileManager::GetInstance().LoadDpSyncAdapter();
    }
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: GetInKvDB001
 * @tc.desc: GetDeviceProfile,GetServiceProfile, GetCharacteristicProfile succeed, in KV DB.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetInKvDB001, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    DeviceProfile deviceProfile2;
    deviceProfile2.SetDeviceId(deviceId);
    deviceProfile2.SetDeviceTypeName("GetInKvDB001_DeviceTypeName");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("GetInKvDB001_DeviceName");
    deviceProfile2.SetManufactureName("GetInKvDB001_ManufactureName");
    deviceProfile2.SetDeviceModel("GetInKvDB001_DeviceModel");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("GetInKvDB001_OsSysCap");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("GetInKvDB001_OsVersion");
    deviceProfile2.SetOsType(1);
    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);

    ServiceProfile serviceProfile5;
    serviceProfile5.SetDeviceId(deviceId);
    serviceProfile5.SetServiceName("GetInKvDB001_ServiceName");
    serviceProfile5.SetServiceType("GetInKvDB001_ServiceType");
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile5);

    CharacteristicProfile charProfile5;
    charProfile5.SetDeviceId(deviceId);
    charProfile5.SetServiceName("GetInKvDB001_ServiceName");
    charProfile5.SetCharacteristicKey("GetInKvDB001_CharacteristicKey");
    charProfile5.SetCharacteristicValue("GetInKvDB001_CharacteristicValue");
    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile5);

    ProfileCache::GetInstance().DeleteDeviceProfile(deviceId);
    ProfileCache::GetInstance().DeleteServiceProfile("GetInKvDB001_DeviceId", "GetInKvDB001_ServiceName");
    ProfileCache::GetInstance().DeleteCharProfile("GetInKvDB001_DeviceId", "GetInKvDB001_ServiceName",
                                                            "GetInKvDB001_CharacteristicKey");

    DeviceProfile outDeviceProfile;
    int32_t ret1 = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret1, DP_SUCCESS);

    string serviceName2 = "GetInKvDB001_ServiceName";
    ServiceProfile outServiceProfile;
    int32_t ret2 = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName2, outServiceProfile);
    EXPECT_EQ(ret2, DP_SUCCESS);

    string serviceName3 = "GetInKvDB001_ServiceName";
    string characteristicKey3 = "GetInKvDB001_CharacteristicKey";
    CharacteristicProfile outCharProfile;
    int32_t ret3 = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName3,
                                                                               characteristicKey3, outCharProfile);
    EXPECT_EQ(ret3, DP_SUCCESS);
}

/**
 * @tc.name: RunloadedFunction001
 * @tc.desc: RunloadedFunction001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, RunloadedFunction001, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> syncCb = OHOS::sptr<SyncCompletedCallbackStub>(new SyncCallback());
    string peerNetworkId = "peerNetworkId";
    int32_t ret = DeviceProfileManager::GetInstance().RunloadedFunction(peerNetworkId, syncCb);
    EXPECT_EQ(ret, DP_LOAD_SYNC_ADAPTER_FAILED);
}
/**
 * @tc.name: RunloadedFunction002
 * @tc.desc: RunloadedFunction002
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, RunloadedFunction002, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> syncCb = OHOS::sptr<SyncCompletedCallbackStub>(new SyncCallback());
    string peerNetworkId = "peerNetworkId";
    int32_t ret = DeviceProfileManager::GetInstance().RunloadedFunction(peerNetworkId, syncCb);
    EXPECT_EQ(ret, DP_LOAD_SYNC_ADAPTER_FAILED);
}

HWTEST_F(DeviceProfileManagerTest, GetInKvDB002, TestSize.Level1)
{
    string deviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    DeviceProfile deviceProfile2;
    deviceProfile2.SetDeviceId(deviceId);
    deviceProfile2.SetDeviceTypeName("GetInKvDB001_DeviceTypeName2");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("GetInKvDB001_DeviceName2");
    deviceProfile2.SetManufactureName("GetInKvDB001_ManufactureName2");
    deviceProfile2.SetDeviceModel("GetInKvDB001_DeviceModel2");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("GetInKvDB001_OsSysCap2");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("GetInKvDB001_OsVersion2");
    deviceProfile2.SetOsType(1);
    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);
    
    ServiceProfile serviceProfile5;
    serviceProfile5.SetDeviceId(deviceId);
    serviceProfile5.SetServiceName("GetInKvDB001_ServiceName2");
    serviceProfile5.SetServiceType("GetInKvDB001_ServiceType2");
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile5);
    
    CharacteristicProfile charProfile5;
    charProfile5.SetDeviceId(deviceId);
    charProfile5.SetServiceName("GetInKvDB001_ServiceName2");
    charProfile5.SetCharacteristicKey("GetInKvDB001_CharacteristicKey2");
    charProfile5.SetCharacteristicValue("GetInKvDB001_CharacteristicValue2");
    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile5);
    
    ProfileCache::GetInstance().DeleteDeviceProfile(deviceId);
    ProfileCache::GetInstance().DeleteServiceProfile("GetInKvDB001_DeviceId2", "GetInKvDB001_ServiceName");
    ProfileCache::GetInstance().DeleteCharProfile("GetInKvDB001_DeviceId2", "GetInKvDB001_ServiceName",
    "GetInKvDB001_CharacteristicKey2");
    
    DeviceProfile outDeviceProfile;
    DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    
    string serviceName2 = "GetInKvDB001_ServiceName2";
    ServiceProfile outServiceProfile;
    DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName2, outServiceProfile);
    
    string serviceName3 = "GetInKvDB001_ServiceName2";
    string characteristicKey3 = "GetInKvDB001_CharacteristicKey2";
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName3,
                                                                                characteristicKey3, outCharProfile);
    EXPECT_EQ(ret, DP_SUCCESS);
}

/**
 * @tc.name: HasTrustP2PRelation001
 * @tc.desc: param is invalid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, HasTrustP2PRelation001, TestSize.Level1)
{
    bool ret = DeviceProfileManager::GetInstance().HasTrustP2PRelation("", 100);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: HasTrustP2PRelation002
 * @tc.desc: HasTrustP2PRelation failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, HasTrustP2PRelation002, TestSize.Level1)
{
    bool ret = DeviceProfileManager::GetInstance().HasTrustP2PRelation("deviceId", 100);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameAccount001
 * @tc.desc: param is invalid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsSameAccount001, TestSize.Level1)
{
    bool ret = DeviceProfileManager::GetInstance().IsSameAccount("", 100);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSameAccount002
 * @tc.desc: GetAccessControlProfile failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsSameAccount002, TestSize.Level1)
{
    bool ret = DeviceProfileManager::GetInstance().IsSameAccount("deviceId", 100);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsMultiUserValid001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUserValid001, TestSize.Level1)
{
    ServiceProfile profile("", "serviceName", "serviceType");
    profile.SetIsMultiUser(false);
    profile.SetUserId(DEFAULT_USER_ID);
    
    int32_t result = DeviceProfileManager::GetInstance().IsMultiUserValid(profile);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: IsMultiUserValid002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUserValid002, TestSize.Level1)
{
    ServiceProfile profile("deviceId", "serviceName", "serviceType");
    profile.SetIsMultiUser(true);
    profile.SetUserId(0);
    
    int32_t result = DeviceProfileManager::GetInstance().IsMultiUserValid(profile);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: IsMultiUserValid003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUserValid003, TestSize.Level1)
{
    ServiceProfile profile("deviceId", "serviceName", "serviceType");
    profile.SetIsMultiUser(false);
    profile.SetUserId(DEFAULT_USER_ID + 1);
    
    int32_t result = DeviceProfileManager::GetInstance().IsMultiUserValid(profile);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: IsMultiUserValid004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUserValid004, TestSize.Level1)
{
    ServiceProfile profile(ProfileCache::GetInstance().GetLocalUdid(), "serviceName", "serviceType");
    profile.SetIsMultiUser(true);
    profile.SetUserId(333);
    
    int32_t result = DeviceProfileManager::GetInstance().IsMultiUserValid(profile);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: IsMultiUserValid005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, IsMultiUserValid005, TestSize.Level1)
{
    ServiceProfile profile("deviceId", "serviceName", "serviceType");
    profile.SetIsMultiUser(false);
    profile.SetUserId(DEFAULT_USER_ID);
    
    int32_t result = DeviceProfileManager::GetInstance().IsMultiUserValid(profile);
    EXPECT_EQ(result, DP_SUCCESS);
}

/**
 * @tc.name: SaveBatchByKeys001
 * @tc.desc: entries is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SaveBatchByKeys001, TestSize.Level1)
{
    std::map<std::string, std::string> entries;
    int32_t result = DeviceProfileManager::GetInstance().SaveBatchByKeys(entries);
    EXPECT_EQ(result, DP_SUCCESS);
}

/**
 * @tc.name: SaveBatchByKeys002
 * @tc.desc: succeed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SaveBatchByKeys002, TestSize.Level1)
{
    std::map<std::string, std::string> entries;
    entries.insert(std::make_pair("key1", "value1"));
    int32_t result = DeviceProfileManager::GetInstance().SaveBatchByKeys(entries);
    EXPECT_EQ(result, DP_SUCCESS);
}

/**
 * @tc.name: SaveBatchByKeys003
 * @tc.desc: deviceProfileStore is nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SaveBatchByKeys003, TestSize.Level1)
{
    std::map<std::string, std::string> entries;
    entries.insert(std::make_pair("key1", "value1"));
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t result = DeviceProfileManager::GetInstance().SaveBatchByKeys(entries);
    EXPECT_NE(result, DP_SUCCESS);
    DeviceProfileManager::GetInstance().FixDiffProfiles();
    DeviceProfileManager::GetInstance().dpSyncAdapter_ = nullptr;
    DeviceProfileManager::GetInstance().FixDiffProfiles();
    DeviceProfileManager::GetInstance().FixRemoteDataWhenPeerIsOHBase("remoteUdid", entries);
    DeviceProfileManager::GetInstance().FixRemoteDataWhenPeerIsNonOH("remoteUdid");
}

/**
 * @tc.name: GetProfilesByKeyPrefix001
 * @tc.desc: DP_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetProfilesByKeyPrefix001, TestSize.Level1)
{
    std::string udid = "";
    std::map<std::string, std::string> values;
    int32_t result = DeviceProfileManager::GetInstance().GetProfilesByKeyPrefix(udid, values);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: GetProfilesByKeyPrefix002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetProfilesByKeyPrefix002, TestSize.Level1)
{
    std::string udid = "udid";
    std::map<std::string, std::string> values;
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t result = DeviceProfileManager::GetInstance().GetProfilesByKeyPrefix(udid, values);
    EXPECT_NE(result, DP_SUCCESS);
}

/**
 * @tc.name: DeleteRemovedUserData001
 * @tc.desc: DeleteRemovedUserData001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteRemovedUserData001, TestSize.Level1)
{
    int32_t userId = -1;
    int32_t ret = DeviceProfileManager::GetInstance().DeleteRemovedUserData(userId);
    EXPECT_EQ(ret, DP_INVALID_PARAM);
}

/**
 * @tc.name: DeleteRemovedUserData001
 * @tc.desc: DeleteRemovedUserData001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteRemovedUserData002, TestSize.Level1)
{
    int32_t userId = 100001;
    int32_t ret = DeviceProfileManager::GetInstance().DeleteRemovedUserData(userId);
    EXPECT_EQ(ret, DP_INVALID_PARAM);
}

/**
 * @tc.name: DeleteRemovedUserData003
 * @tc.desc: DeleteRemovedUserData003
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteRemovedUserData003, TestSize.Level1)
{
    int32_t userId = 1;
    int32_t ret = DeviceProfileManager::GetInstance().DeleteRemovedUserData(userId);
    EXPECT_EQ(ret, DP_SUCCESS);
}

#ifdef DEVICE_PROFILE_SWITCH_STATIC_ENABLE
/**
 * @tc.name: DeleteRemovedUserData004
 * @tc.desc: DeleteRemovedUserData004
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteRemovedUserData004, TestSize.Level1)
{
    int32_t userId = 1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteRemovedUserData(userId);
    EXPECT_EQ(ret, DP_SUCCESS);
}
#endif // DEVICE_PROFILE_SWITCH_STATIC_ENABLE

/**
 * @tc.name: DeleteRemovedUserData005
 * @tc.desc: DeleteRemovedUserData005
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, DeleteRemovedUserData005, TestSize.Level1)
{
    int32_t userId = 1;
    ContentSensorManagerUtils::GetInstance().localUdid_ = "localUdid";
    int32_t ret = DeviceProfileManager::GetInstance().DeleteRemovedUserData(userId);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
}

} // namespace DistributedDeviceProfile
} // namespace OHOS
