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

#define private   public
#define protected public
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
#include "characteristic_profile.h"
#include "i_sync_completed_callback.h"
#include "sync_completed_callback_stub.h"
#include "device_profile_manager.h"
#include "kv_adapter.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::DistributedDeviceProfile;
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
    DeviceProfileManager::GetInstance().Init();
}

void DeviceProfileManagerTest::TearDown()
{
    DeviceProfileManager::GetInstance().UnInit();
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
    deviceProfile.SetDeviceId("anything");
    deviceProfile.SetDeviceTypeName("anything");
    deviceProfile.SetDeviceTypeId(0);
    deviceProfile.SetDeviceName("anything");
    deviceProfile.SetManufactureName("anything");
    deviceProfile.SetDeviceModel("anything");
    deviceProfile.SetSerialNumberId("anything");
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
    deviceProfile.SetSerialNumberId("anything");
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
    deviceProfile1.SetDeviceId("anything1");
    deviceProfile1.SetDeviceTypeName("anything");
    deviceProfile1.SetDeviceTypeId(0);
    deviceProfile1.SetDeviceName("anything");
    deviceProfile1.SetManufactureName("anything");
    deviceProfile1.SetDeviceModel("anything");
    deviceProfile1.SetSerialNumberId("anything");
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
    deviceProfile2.SetSerialNumberId("anything");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("anything");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("anything");
    deviceProfile2.SetOsType(1);
    
    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile1);
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);
    EXPECT_EQ(ret, DP_CACHE_EXIST);
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
    deviceProfile10.SetSerialNumberId("anything");
    deviceProfile10.SetStorageCapability(1);
    deviceProfile10.SetOsSysCap("anything");
    deviceProfile10.SetOsApiLevel(1);
    deviceProfile10.SetOsVersion("anything");
    deviceProfile10.SetOsType(1);
    
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile10);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
    DeviceProfileManager::GetInstance().Init();
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
    deviceProfile11.SetDeviceId("anything11");
    deviceProfile11.SetDeviceTypeName("anything");
    deviceProfile11.SetDeviceTypeId(0);
    deviceProfile11.SetDeviceName("anything");
    deviceProfile11.SetManufactureName("anything");
    deviceProfile11.SetDeviceModel("anything");
    deviceProfile11.SetSerialNumberId("anything");
    deviceProfile11.SetStorageCapability(1);
    deviceProfile11.SetOsSysCap("anything");
    deviceProfile11.SetOsApiLevel(1);
    deviceProfile11.SetOsVersion("anything");
    deviceProfile11.SetOsType(1);

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    serviceProfile1.SetDeviceId("deviceId1");
    serviceProfile1.SetServiceName("serviceName");
    serviceProfile1.SetServiceType("serviceType");

    ServiceProfile serviceProfile2;
    serviceProfile2.SetDeviceId("deviceId1");
    serviceProfile2.SetServiceName("serviceName");
    serviceProfile2.SetServiceType("serviceType");
    
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile1);
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile2);
    EXPECT_EQ(ret, DP_CACHE_EXIST);
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
    serviceProfile11.SetDeviceId("deviceId11");
    serviceProfile11.SetServiceName("serviceName11");
    serviceProfile11.SetServiceType("serviceType11");

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    EXPECT_EQ(ret, DP_CACHE_EXIST);
}

/**
 * @tc.name: PutCharacteristicProfile004
 * @tc.desc: PutCharacteristicProfile failed, deviceProfileStore is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, PutCharacteristicProfile004, TestSize.Level1)
{
    CharacteristicProfile charProfile10;
    charProfile10.SetDeviceId("deviceId10");
    charProfile10.SetServiceName("serviceName10");
    charProfile10.SetCharacteristicKey("characteristicKey10");
    charProfile10.SetCharacteristicValue("characteristicValue10");
    
    DeviceProfileManager::GetInstance().deviceProfileStore_ = nullptr;
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile10);
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
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
    charProfile11.SetDeviceId("deviceId11");
    charProfile11.SetServiceName("serviceName11");
    charProfile11.SetCharacteristicKey("characteristicKey11");
    charProfile11.SetCharacteristicValue("characteristicValue11");

    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile11);
    EXPECT_EQ(ret, DP_PUT_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    deviceProfile2.SetDeviceId("anything2");
    deviceProfile2.SetDeviceTypeName("anything");
    deviceProfile2.SetDeviceTypeId(0);
    deviceProfile2.SetDeviceName("anything");
    deviceProfile2.SetManufactureName("anything");
    deviceProfile2.SetDeviceModel("anything");
    deviceProfile2.SetSerialNumberId("anything");
    deviceProfile2.SetStorageCapability(1);
    deviceProfile2.SetOsSysCap("anything");
    deviceProfile2.SetOsApiLevel(1);
    deviceProfile2.SetOsVersion("anything");
    deviceProfile2.SetOsType(1);
    DeviceProfileManager::GetInstance().PutDeviceProfile(deviceProfile2);

    string deviceId = "anything2";
    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);

    string outDeviceId = outDeviceProfile.GetDeviceId();
    outDeviceProfile.GetDeviceTypeName();
    outDeviceProfile.GetDeviceTypeId();
    outDeviceProfile.GetDeviceName();
    outDeviceProfile.GetManufactureName();
    outDeviceProfile.GetDeviceModel();
    outDeviceProfile.GetSerialNumberId();
    outDeviceProfile.GetStorageCapability();
    outDeviceProfile.GetOsSysCap();
    outDeviceProfile.GetOsApiLevel();
    outDeviceProfile.GetOsVersion();
    outDeviceProfile.GetOsType();
    EXPECT_EQ(outDeviceId, "anything2");
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
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
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
    string deviceId = "anything13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    DeviceProfile outDeviceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetDeviceProfile(deviceId, outDeviceProfile);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetServiceProfile001
 * @tc.desc: GetServiceProfile succeed, GetServiceProfile in cache.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetServiceProfile001, TestSize.Level1)
{
    ServiceProfile serviceProfile5;
    serviceProfile5.SetDeviceId("deviceId5");
    serviceProfile5.SetServiceName("serviceName5");
    serviceProfile5.SetServiceType("serviceType5");
    DeviceProfileManager::GetInstance().PutServiceProfile(serviceProfile5);

    string deviceId = "deviceId5";
    string serviceName = "serviceName5";
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_EQ(ret, DP_SUCCESS);

    string outDeviceId = outServiceProfile.GetDeviceId();
    string outServiceName = outServiceProfile.GetServiceName();
    string outServiceType = outServiceProfile.GetServiceType();
    EXPECT_EQ(outDeviceId, "deviceId5");
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
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
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
    string deviceId = "deviceId13";
    string serviceName = "serviceName13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    ServiceProfile outServiceProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetServiceProfile(deviceId, serviceName, outServiceProfile);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: GetCharacteristicProfile001
 * @tc.desc: GetCharacteristicProfile succeed, GetCharProfile in cache.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, GetCharacteristicProfile001, TestSize.Level1)
{
    CharacteristicProfile charProfile5;
    charProfile5.SetDeviceId("deviceId5");
    charProfile5.SetServiceName("serviceName5");
    charProfile5.SetCharacteristicKey("characteristicKey5");
    charProfile5.SetCharacteristicValue("characteristicValue5");
    DeviceProfileManager::GetInstance().PutCharacteristicProfile(charProfile5);

    string deviceId = "deviceId5";
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
    EXPECT_EQ(outDeviceId, "deviceId5");
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
    EXPECT_EQ(ret, DP_INVALID_PARAMS);
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
    string deviceId = "deviceId13";
    string serviceName = "serviceName13";
    string characteristicKey = "characteristicKey13";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    CharacteristicProfile outCharProfile;
    int32_t ret = DeviceProfileManager::GetInstance().GetCharacteristicProfile(deviceId, serviceName,
        characteristicKey, outCharProfile);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    string deviceId = "deviceId15";
    string serviceName = "serviceName15";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().DeleteServiceProfile(deviceId, serviceName);
    EXPECT_EQ(ret, DP_DEL_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    string deviceId = "deviceId15";
    string serviceName = "serviceName15";
    string characteristicKey = "characteristicKey15";
    DeviceProfileManager::GetInstance().deviceProfileStore_->UnInit();
    int32_t ret = DeviceProfileManager::GetInstance().DeleteCharacteristicProfile(deviceId, serviceName,
        characteristicKey);
    EXPECT_EQ(ret, DP_DEL_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    vector<DeviceProfile> deviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllDeviceProfile(deviceProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    vector<ServiceProfile> serviceProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllServiceProfile(serviceProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
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
    vector<CharacteristicProfile> charProfiles;
    int32_t ret = DeviceProfileManager::GetInstance().GetAllCharacteristicProfile(charProfiles);
    EXPECT_EQ(ret, DP_GET_KV_DB_FAIL);
    DeviceProfileManager::GetInstance().Init();
}

/**
 * @tc.name: SyncDeviceProfile001
 * @tc.desc: SyncDeviceProfile failed, Params is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DeviceProfileManagerTest, SyncDeviceProfile001, TestSize.Level1)
{
    SyncOptions syncOptions;
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
    SyncOptions syncOptions;
    OHOS::sptr<OHOS::IRemoteObject> syncCb = new(nothrow) SyncCallback();
    
    syncOptions.AddDevice("deviceId1");
    syncOptions.AddDevice("deviceId2");
    syncOptions.SetSyncMode(SyncMode::MAX);
    
    int32_t errCode = DeviceProfileManager::GetInstance().SyncDeviceProfile(syncOptions, syncCb);
    EXPECT_EQ(errCode, DP_INVALID_PARAMS);
}