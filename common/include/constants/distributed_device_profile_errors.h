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

#ifndef OHOS_DP_DISTRIBUTED_DEVICE_PROFILE_ERRORS_H
#define OHOS_DP_DISTRIBUTED_DEVICE_PROFILE_ERRORS_H

namespace OHOS {
namespace DistributedDeviceProfile {
// DEVICE_PROFILE_ERR_OFFSET(98566143)
constexpr int32_t DP_SUCCESS = 0;
constexpr int32_t DP_INVALID_PARAMS = 98566144;
constexpr int32_t DP_INTERFACE_CHECK_FAILED = 98566145;
constexpr int32_t DP_GET_LOCAL_UDID_FAILED = 98566146;
constexpr int32_t DP_GET_SERVICE_FAILED = 98566147;
constexpr int32_t DP_INIT_DB_FAILED = 98566148;
constexpr int32_t DP_NOT_SUBSCRIBED = 98566149;
constexpr int32_t DP_UNSUBSCRIBE_FAILED = 98566150;
constexpr int32_t DP_SUBSCRIBE_FAILED = 98566151;
constexpr int32_t DP_SUBSCRIBE_LIMIT_EXCEEDED = 98566152;
constexpr int32_t DP_POST_TASK_FAILED = 98566153;
constexpr int32_t DP_DEVICE_SYNC_BUSY = 98566154;
constexpr int32_t DP_PERMISSION_DENIED = 98566155;
constexpr int32_t DP_UNTRUSTED_GROUP = 98566156;
constexpr int32_t DP_NOT_INIT_DB = 98566157;
constexpr int32_t DP_GET_NETWORKID_FAILED = 98566158;
constexpr int32_t DP_FILE_FAILED_ERR = 98566159;
constexpr int32_t DP_LOAD_SERVICE_ERR = 98566160;
constexpr int32_t DP_PUT_DEVICE_CACHE_ERR = 98566161;
constexpr int32_t DP_PUT_SERVICE_CACHE_ERR = 98566162;
constexpr int32_t DP_PUT_CHARACTERISTIC_CACHE_ERR = 98566163;
constexpr int32_t DP_CACHE_EXIST = 98566164;
constexpr int32_t DP_CACHE_NOT_EXIST = 98566165;
constexpr int32_t DP_IPC_REMOTE_OBJECT_NULLPTR = 98566166;
constexpr int32_t DP_GET_ACL_PROFILE_FAIL = 98566167;
constexpr int32_t DP_GET_TRUST_DEVICE_PROFILE_FAIL = 98566168;
constexpr int32_t DP_GET_ALL_TRUST_DEVICE_PROFILE_FAIL = 98566169;
constexpr int32_t DP_PUT_ACL_PROFILE_FAIL = 98566170;
constexpr int32_t DP_UPDATE_ACL_PROFILE_FAIL = 98566171;
constexpr int32_t DP_UNLOAD_HANDLER_NULLPTR = 98566172;
constexpr int32_t DP_CACHE_INIT_FAIL = 98566173;
constexpr int32_t DP_DEVICE_MANAGER_INIT_FAIL = 98566174;
constexpr int32_t DP_TRUST_PROFILE_MANAGER_INIT_FAIL = 98566175;
constexpr int32_t DP_DEVICE_PROFILE_MANAGER_INIT_FAIL = 98566176;
constexpr int32_t DP_SUBSCRIBE_PROFILE_MANAGER_INIT_FAIL = 98566177;
constexpr int32_t DP_CONTENT_SENSOR_MANAGER_INIT_FAIL = 98566179;
constexpr int32_t DP_CACHE_UNINIT_FAIL = 98566180;
constexpr int32_t DP_DEVICE_MANAGER_UNINIT_FAIL = 98566181;
constexpr int32_t DP_TRUST_PROFILE_MANAGER_UNINIT_FAIL = 98566182;
constexpr int32_t DP_DEVICE_PROFILE_MANAGER_UNINIT_FAIL = 98566183;
constexpr int32_t DP_SUBSCRIBE_TRUST_PROFILE_MANAGER_UNINIT_FAIL = 98566184;
constexpr int32_t DP_SUBSCRIBE_DEVICE_PROFILE_MANAGER_UNINIT_FAIL = 98566185;
constexpr int32_t DP_CONTENT_SENSOR_MANAGER_UNINIT_FAIL = 98566186;
constexpr int32_t DP_CS_COLLECTOR_HANDLER_INIT_FAIL = 98566187;
constexpr int32_t DP_CS_POST_TASK_FAIL = 98566188;
constexpr int32_t DP_KV_DB_PTR_NULL = 98566189;
constexpr int32_t DP_KV_DB_INIT_FAIL = 98566190;
constexpr int32_t DP_REGISTER_KV_DATA_LISTENER_FAILED = 98566191;
constexpr int32_t DP_UNREGISTER_KV_DATA_LISTENER_FAILED = 98566192;
constexpr int32_t DP_REGISTER_KV_SYNC_LISTENER_FAILED = 98566193;
constexpr int32_t DP_UNREGISTER_KV_SYNC_LISTENER_FAILED = 98566194;
constexpr int32_t DP_REGISTER_KV_DEATH_LISTENER_FAILED = 98566195;
constexpr int32_t DP_UNREGISTER_KV_DEATH_LISTENER_FAILED = 98566196;
constexpr int32_t DP_PUT_KV_DB_FAIL = 98566197;
constexpr int32_t DP_DEL_KV_DB_FAIL = 98566198;
constexpr int32_t DP_GET_KV_DB_FAIL = 98566199;
constexpr int32_t DP_MANAGER_INIT_FAIL = 98566200;
constexpr int32_t DP_EXCEED_MAX_SIZE_FAIL = 98566201;
constexpr int32_t DP_NOT_FOUND_FAIL = 98566202;
constexpr int32_t DP_SYNC_DEVICE_FAIL = 98566203;
constexpr int32_t DP_KV_SYNC_FAIL = 98566204;
constexpr int32_t DP_GET_SA_MANAGER_FAIL = 98566205;
constexpr int32_t DP_GET_SA_PROXY_FAIL = 98566206;
constexpr int32_t DP_PARSE_PERMISSION_JSON_FAIL = 98566207;
constexpr int32_t DP_RDBADAPTER_PUT_FAIL = 98566209;
constexpr int32_t DP_RDBADAPTER_DELETE_FAIL = 98566210;
constexpr int32_t DP_RDBADAPTER_UPDATE_FAIL = 98566211;
constexpr int32_t DP_RDBADAPTER_CREATE_TABLE_FAIL = 98566212;
constexpr int32_t DP_RDBADAPTER_INIT_FAIL = 98566213;
constexpr int32_t DP_GET_RDBSTORE_FAIL = 98566214;
constexpr int32_t DP_RDBADAPTER_TABLE_NOT_EXIST = 98566215;
constexpr int32_t DP_UNINIT_FAIL = 98566216;
constexpr int32_t DP_PUT_SUBSCRIBE_TRUST_INFO_TABLE_FAIL = 98566217;
constexpr int32_t DP_PUT_TRUST_DEVICE_PROFILE_FAIL = 98566218;
constexpr int32_t DP_NOTIFY_TRUST_DEVICE_FAIL = 98566219;
constexpr int32_t DP_GET_RESULTSET_FAIL = 98566220;
constexpr int32_t DP_NOT_FIND_DATA = 98566221;
constexpr int32_t DP_NOTIFY_STATUS_UPDATE_FAIL = 98566222;
constexpr int32_t DP_RESULTSET_TO_VALUES_FAIL = 98566223;
constexpr int32_t DP_DELETE_TRUST_DEVICE_PROFILE_FAIL = 98566224;
constexpr int32_t DP_DELETE_ACCESS_CONTROL_PROFILE_FAIL = 98566225;
constexpr int32_t DP_CREATE_TABLE_FAIL = 98566226;
constexpr int32_t DP_CREATE_UNIQUE_INDEX_FAIL = 98566227;
constexpr int32_t DP_PUT_ACCESSER_PROFILE_FAIL = 98566228;
constexpr int32_t DP_PUT_ACCESSEE_PROFILE_FAIL = 98566229;
constexpr int32_t DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL = 98566230;
constexpr int32_t DP_DELETE_ACCESSER_PROFILE_FAIL = 98566231;
constexpr int32_t DP_DELETE_ACCESSEE_PROFILE_FAIL = 98566232;
constexpr int32_t DP_GET_SYSTEM_ABILITY_FAIL = 98566233;
constexpr int32_t DP_GET_PROXY_FAIL = 98566234;
constexpr int32_t DP_WRITE_PARCEL_FAIL = 98566235;
constexpr int32_t DP_READ_PARCEL_FAIL = 98566236;
constexpr int32_t DP_UNLOAD_SA_FAIL = 98566237;
constexpr int32_t DP_UPDATE_ACCESSER_PROFILE_FAIL = 98566238;
constexpr int32_t DP_UPDATE_ACCESSEE_PROFILE_FAIL = 98566239;
constexpr int32_t DP_SYNC_INIT_FAILED = 98566240;
constexpr int32_t DP_SYNC_PROFILE_FAILED = 98566241;
constexpr int32_t DP_UNSUPPORTED_METHOD = 98566242;
constexpr int32_t DP_NO_MEMORY = 98566243;
constexpr int32_t DP_SESSION_ERROR = 98566244;
constexpr int32_t DP_INVALID_PARAM = 98566245;
constexpr int32_t DP_CREATE_SESSION_FAILED = 98566246;
constexpr int32_t DP_RUN_LOADED_FUNCTION_FAILED = 98566247;
constexpr int32_t DP_LOAD_SYNC_ADAPTER_FAILED = 98566248;
constexpr int32_t DP_UPDATE_ACL_NOT_ALLOW = 98566249;
constexpr int32_t DP_RDB_DB_PTR_NULL = 98566250;
constexpr int32_t DP_NOTIFY_ACCESS_CONTROL_FAIL = 98566251;
constexpr int32_t DP_DM_ADAPTER_INIT_FAIL = 98566252;
constexpr int32_t DP_DATA_EXISTS = 98566253;
constexpr int32_t DP_STATIC_COLLECT_FAIL = 98566254;
constexpr int32_t DP_LOAD_STATIC_CAP_FAIL = 98566255;
constexpr int32_t DP_LOAD_STATIC_INFO_FAIL = 98566256;
constexpr int32_t DP_GET_STATIC_CAP_FAIL = 98566257;
constexpr int32_t DP_GET_STATIC_INFO_FAIL = 98566258;
constexpr int32_t DP_PARSE_STATIC_CAP_FAIL = 98566259;
constexpr int32_t DP_PARSE_STATIC_INFO_FAIL = 98566260;
constexpr int32_t DP_LOAD_JSON_FILE_FAIL = 98566261;
constexpr int32_t DP_SET_SWITCH_CHARPROFILE_FAIL = 98566262;
constexpr int32_t DP_NOTIFYSWITCHUPDATE_FAIL = 98566263;
constexpr int32_t DP_PUT_CHAR_BATCH_FAIL = 98566264;
constexpr int32_t DP_GET_SWITCH_FAIL = 98566265;
constexpr int32_t DP_GET_NETWORKID_BY_UDID_FAIL = 98566266;
constexpr int32_t DP_GET_UDID_BY_NETWORKID_FAIL = 98566267;
constexpr int32_t DP_GET_SERVICENAME_BY_POS_FAIL = 98566268;
constexpr int32_t DP_GENERATE_SWITCH_NOTIFY_FAIL = 98566269;
constexpr int32_t DP_LOAD_SWITCH_JSON_FAIL = 98566270;
constexpr int32_t DP_GET_SWITCH_INFO_FAIL = 98566271;
constexpr int32_t DP_INIT_SWITCH_PROFILE_MANAGER_FAIL = 98566272;
constexpr int32_t DP_NULLPTR = 98566273;
constexpr int32_t DP_SUBSCRIBE_INITED_FALI = 98566274;
constexpr int32_t DP_GET_DEVICE_ENTRIES_FAIL = 98566275;
constexpr int32_t DP_RDB_DATABASE_RESTORE_FAIL = 98566276;
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif // OHOS_DP_DISTRIBUTED_DEVICE_PROFILE_ERRORS_H
