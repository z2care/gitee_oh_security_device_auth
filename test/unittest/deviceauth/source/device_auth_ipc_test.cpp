/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cinttypes>
#include <cstring>
#include <unistd.h>
#include <gtest/gtest.h>
#include "device_auth.h"
#include "device_auth_defines.h"
#include "json_utils.h"
#include "securec.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace std;
using namespace testing::ext;

namespace {
#define TEST_REQ_ID 123
#define TEST_REQ_ID2 321
#define TEST_REQ_ID3 322
#define TEST_REQ_ID4 323
#define TEST_APP_ID "TestAppId"
#define TEST_APP_ID2 "TestAppId2"
#define TEST_GROUP_NAME "TestGroup"
#define TEST_AUTH_ID "TestAuthId"
#define TEST_AUTH_ID2 "TestAuthId2"
#define TEST_AUTH_ID3 "TestAuthId3"
#define TEST_UDID "TestUdid"
#define TEST_UDID2 "TestUdid2"
#define TEST_UDID_CLIENT "5420459D93FE773F9945FD64277FBA2CAB8FB996DDC1D0B97676FBB1242B3930"
#define TEST_UDID_SERVER "52E2706717D5C39D736E134CC1E3BE1BAA2AA52DB7C76A37C749558BD2E6492C"
#define TEST_USER_ID_AUTH "4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4"
#define TEST_USER_ID "1234ABCD"
#define TEST_USER_ID2 "DCBA4321"
#define TEST_GROUP_ID "E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21"
#define TEST_GROUP_ID2 "F2AA208B1E010542B20A34B03B4B6289EA7C7F6DFE97DA2E370348B826682D3D"
#define TEST_GROUP_ID3 "1234ABCD"
#define TEST_GROUP_ID4 "6B7B805962B8EB8275D73128BFDAA7ECD755A2EC304E36543941874A277FA75F"
#define TEST_AUTH_CODE "37364761534f454d33567a73424e794f33573330507069434b31676f7254706b"
#define TEST_AUTH_CODE2 "2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335"
#define TEST_AUTH_CODE3 "1234567812345678123456781234567812345678123456781234567812345678"
#define TEST_AUTH_TOKEN3 "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92"
#define TEST_QUERY_PARAMS "bac"
#define TEST_PIN_CODE "123456"
#define TEST_DEV_AUTH_TEMP_KEY_PAIR_LEN 32
#define TEST_DEV_AUTH_SLEEP_TIME 50000
static const char *g_createParams = "{\"groupName\":\"TestGroup\",\"deviceId\":\"TestAuthId\",\"groupType\":256,\"group"
    "Visibility\":-1,\"userType\":0,\"expireTime\":-1}";
static const char *g_createParams2 = "{\"groupName\":\"TestPrivateGroup\",\"deviceId\":\"TestAuth"
    "Id\",\"groupType\":256,\"groupVisibility\":0,\"userType\":0,\"expireTime\":-1}";
static const char *g_disbandParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\"}";
static const char *g_disbandParams2 =
    "{\"groupId\":\"F2AA208B1E010542B20A34B03B4B6289EA7C7F6DFE97DA2E370348B826682D3D\"}";
static const char *g_addParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\","
    "\"groupType\":256,\"pinCode\":\"123456\"}";
static const char *g_addParams2 =
    "{\"groupType\":1,\"groupId\":\"1234ABCD\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\",\"udid\":\"TestUdid\",\"userId\":\"1234ABCD\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"37364761534f454d33567a73424e794f33573330507069434b31676f7254706b\"}},"
    "{\"deviceId\":\"TestAuthId3\",\"udid\":\"TestUdid2\",\"userId\":\"1234ABCD\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335\"}}]}";
static const char *g_deleteParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\",\"deleteId\":\"TestAuthId2\"}";
static const char *g_deleteParams2 =
    "{\"groupType\":1,\"groupId\":\"1234ABCD\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\"},{\"deviceId\":\"TestAuthId3\"}]}";
static const char *g_processData = "{\"appId\":\"TestAppId\"}";
static const char *g_authParams = "{\"peerConnDeviceId\":\"52E2706717D5C39D736E134CC1E3BE1BAA2AA52DB7C76A37C"
    "749558BD2E6492C\",\"servicePkgName\":\"TestAppId\",\"isClient\":true}";
static const char *g_getRegisterInfoParams =
    "{\"version\":\"1.0.0\",\"deviceId\":\"TestAuthId\",\"userId\":\"1234ABCD\"}";

enum AsyncStatus {
    ASYNC_STATUS_WAITING = 0,
    ASYNC_STATUS_TRANSMIT = 1,
    ASYNC_STATUS_FINISH = 2,
    ASYNC_STATUS_ERROR = 3
};

static AsyncStatus volatile g_asyncStatus;
static uint32_t g_transmitDataMaxLen = 2048;
static uint8_t g_transmitData[2048] = { 0 };
static uint32_t g_transmitDataLen = 0;

static void SetAccessToken()
{
    const char *acls[] = {"ACCESS_IDS"};
    const char *perms[] = {
        "ohos.permission.PLACE_CALL",
        "ohos.permission.ACCESS_IDS"
    };
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 1,
        .dcaps = NULL,
        .perms = perms,
        .acls = acls,
        .processName = "dslm_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
}

static bool OnTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    if (memcpy_s(g_transmitData, g_transmitDataMaxLen, data, dataLen) != EOK) {
        return false;
    }
    g_transmitDataLen = dataLen;
    g_asyncStatus = ASYNC_STATUS_TRANSMIT;
    return true;
}

static void OnSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    (void)requestId;
    (void)sessionKey;
    (void)sessionKeyLen;
    return;
}

static void OnFinish(int64_t requestId, int operationCode, const char *authReturn)
{
    g_asyncStatus = ASYNC_STATUS_FINISH;
}

static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    g_asyncStatus = ASYNC_STATUS_ERROR;
}

static char *OnBindRequest(int64_t requestId, int operationCode, const char* reqParam)
{
    return nullptr;
}

static char *OnAuthRequest(int64_t requestId, int operationCode, const char* reqParam)
{
    return nullptr;
}

static DeviceAuthCallback g_gmCallback = {
    .onTransmit = OnTransmit,
    .onSessionKeyReturned = OnSessionKeyReturned,
    .onFinish = OnFinish,
    .onError = OnError,
    .onRequest = OnBindRequest
};

static DeviceAuthCallback g_gaCallback = {
    .onTransmit = OnTransmit,
    .onSessionKeyReturned = OnSessionKeyReturned,
    .onFinish = OnFinish,
    .onError = OnError,
    .onRequest = OnAuthRequest
};

static void OnGroupCreated(const char *groupInfo)
{
    (void)groupInfo;
    return;
}

static void OnGroupDeleted(const char *groupInfo)
{
    (void)groupInfo;
    return;
}

static void OnDeviceBound(const char *peerUdid, const char *groupInfo)
{
    (void)peerUdid;
    (void)groupInfo;
    return;
}

static void OnDeviceUnBound(const char *peerUdid, const char *groupInfo)
{
    (void)peerUdid;
    (void)groupInfo;
    return;
}

static void OnDeviceNotTrusted(const char *peerUdid)
{
    (void)peerUdid;
    return;
}

static void OnLastGroupDeleted(const char *peerUdid, int groupType)
{
    (void)peerUdid;
    (void)groupType;
    return;
}

static void OnTrustedDeviceNumChanged(int curTrustedDeviceNum)
{
    (void)curTrustedDeviceNum;
    return;
}

static DataChangeListener g_listener = {
    .onGroupCreated = OnGroupCreated,
    .onGroupDeleted = OnGroupDeleted,
    .onDeviceBound = OnDeviceBound,
    .onDeviceUnBound = OnDeviceUnBound,
    .onDeviceNotTrusted = OnDeviceNotTrusted,
    .onLastGroupDeleted = OnLastGroupDeleted,
    .onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged,
};

static void CreateDemoGroup(int32_t osAccountId, int64_t reqId, const char *appId, const char *createParams)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    int32_t ret = gm->createGroup(osAccountId, reqId, appId, createParams);
    if (ret != HC_SUCCESS) {
        g_asyncStatus = ASYNC_STATUS_ERROR;
        return;
    }
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    usleep(TEST_DEV_AUTH_SLEEP_TIME);
}

static void DeleteDemoGroup(int32_t osAccountId, int64_t reqId, const char *appId, const char *disbandParams)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(osAccountId, reqId, appId, disbandParams);
    if (ret != HC_SUCCESS) {
        g_asyncStatus = ASYNC_STATUS_ERROR;
        return;
    }
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    usleep(TEST_DEV_AUTH_SLEEP_TIME);
}

class InitDeviceAuthServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void InitDeviceAuthServiceTest::SetUpTestCase() {}
void InitDeviceAuthServiceTest::TearDownTestCase() {}
void InitDeviceAuthServiceTest::SetUp() {}
void InitDeviceAuthServiceTest::TearDown() {}

HWTEST_F(InitDeviceAuthServiceTest, InitDeviceAuthServiceTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
    DestroyDeviceAuthService();
}

class DestroyDeviceAuthServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DestroyDeviceAuthServiceTest::SetUpTestCase() {}
void DestroyDeviceAuthServiceTest::TearDownTestCase() {}
void DestroyDeviceAuthServiceTest::SetUp() {}
void DestroyDeviceAuthServiceTest::TearDown() {}

HWTEST_F(DestroyDeviceAuthServiceTest, DestroyDeviceAuthServiceTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
    DestroyDeviceAuthService();
}

class GetGmInstanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GetGmInstanceTest::SetUpTestCase() {}
void GetGmInstanceTest::TearDownTestCase() {}

void GetGmInstanceTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GetGmInstanceTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GetGmInstanceTest, GetGmInstanceTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
}

class GetGaInstanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GetGaInstanceTest::SetUpTestCase() {}
void GetGaInstanceTest::TearDownTestCase() {}

void GetGaInstanceTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GetGaInstanceTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GetGaInstanceTest, GetGaInstanceTest001, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    EXPECT_NE(ga, nullptr);
}

class GmRegCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmRegCallbackTest::SetUpTestCase() {}
void GmRegCallbackTest::TearDownTestCase() {}

void GmRegCallbackTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmRegCallbackTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
    DeviceAuthCallback callback2;
    ret = gm->regCallback(TEST_APP_ID, &callback2);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(nullptr, &callback);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmRegDataChangeListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmRegDataChangeListenerTest::SetUpTestCase() {}
void GmRegDataChangeListenerTest::TearDownTestCase() {}

void GmRegDataChangeListenerTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmRegDataChangeListenerTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &g_listener);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &g_listener);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->regDataChangeListener(TEST_APP_ID, &g_listener);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(nullptr, &g_listener);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmCreateGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmCreateGroupTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmCreateGroupTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmCreateGroupTest::SetUp() {}
void GmCreateGroupTest::TearDown() {}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, g_createParams);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_createParams);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
    CreateDemoGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_createParams2);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

class GmDestroyInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDestroyInfoTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmDestroyInfoTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmDestroyInfoTest::SetUp() {}
void GmDestroyInfoTest::TearDown() {}

HWTEST_F(GmDestroyInfoTest, GmDestroyInfoTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    gm->destroyInfo(nullptr);
}

HWTEST_F(GmDestroyInfoTest, GmDestroyInfoTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *str = nullptr;
    gm->destroyInfo(&str);
}

class GmCheckAccessToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmCheckAccessToGroupTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmCheckAccessToGroupTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmCheckAccessToGroupTest::SetUp() {}

void GmCheckAccessToGroupTest::TearDown() {}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, TEST_GROUP_ID2);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID3);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmGetPkInfoListTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetPkInfoListTest::SetUpTestCase() {}
void GmGetPkInfoListTest::TearDownTestCase() {}

void GmGetPkInfoListTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetPkInfoListTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    const char *queryParams = "{\"udid\":\"5420459D93FE773F9945FD64277FBA2CAB8FB996DDC1D0B97676FBB1242"
        "B3930\",\"isSelfPk\":true}";
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, queryParams, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    ASSERT_EQ(returnNum, 0);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    const char *queryParams = "{\"udid\":\"ABCD\",\"isSelfPk\":true}";
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, queryParams, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetGroupInfoByIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetGroupInfoByIdTest::SetUpTestCase() {}
void GmGetGroupInfoByIdTest::TearDownTestCase() {}

void GmGetGroupInfoByIdTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetGroupInfoByIdTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, TEST_GROUP_ID2, &returnData);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID3, &returnData);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmGetGroupInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetGroupInfoTest::SetUpTestCase() {}
void GmGetGroupInfoTest::TearDownTestCase() {}

void GmGetGroupInfoTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetGroupInfoTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    const char *queryParams = "{\"groupOwner\":\"TestAppId\"}";
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, queryParams, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    const char *queryParams = "{\"groupOwner\":\"TestAppId2\"}";
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, queryParams, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetJoinedGroupsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetJoinedGroupsTest::SetUpTestCase() {}
void GmGetJoinedGroupsTest::TearDownTestCase() {}

void GmGetJoinedGroupsTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetJoinedGroupsTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, nullptr, PEER_TO_PEER_GROUP, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, PEER_TO_PEER_GROUP, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, nullptr, PEER_TO_PEER_GROUP, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, PEER_TO_PEER_GROUP, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, PEER_TO_PEER_GROUP, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetRelatedGroupsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetRelatedGroupsTest::SetUpTestCase() {}
void GmGetRelatedGroupsTest::TearDownTestCase() {}

void GmGetRelatedGroupsTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetRelatedGroupsTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, nullptr, TEST_AUTH_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID2, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetDeviceInfoByIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetDeviceInfoByIdTest::SetUpTestCase() {}
void GmGetDeviceInfoByIdTest::TearDownTestCase() {}

void GmGetDeviceInfoByIdTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetDeviceInfoByIdTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, nullptr, TEST_AUTH_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, TEST_AUTH_ID, TEST_GROUP_ID2, &returnData);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest007, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, TEST_GROUP_ID3, &returnData);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmGetTrustedDevicesTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetTrustedDevicesTest::SetUpTestCase() {}
void GmGetTrustedDevicesTest::TearDownTestCase() {}

void GmGetTrustedDevicesTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetTrustedDevicesTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, TEST_GROUP_ID2, &returnData, &returnNum);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest007, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID3, &returnData, &returnNum);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmIsDeviceInGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmIsDeviceInGroupTest::SetUpTestCase() {}
void GmIsDeviceInGroupTest::TearDownTestCase() {}

void GmIsDeviceInGroupTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmIsDeviceInGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, TEST_AUTH_ID);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, TEST_AUTH_ID);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, TEST_AUTH_ID);
    ASSERT_EQ(ret, true);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID2, TEST_GROUP_ID2, TEST_AUTH_ID);
    ASSERT_NE(ret, true);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest006, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID3, TEST_AUTH_ID);
    ASSERT_EQ(ret, false);
}

class GmAddMemberToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmAddMemberToGroupTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmAddMemberToGroupTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmAddMemberToGroupTest::SetUp() {}
void GmAddMemberToGroupTest::TearDown() {}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID2, TEST_APP_ID, g_addParams);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GmDeleteMemberFromGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDeleteMemberFromGroupTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmDeleteMemberFromGroupTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmDeleteMemberFromGroupTest::SetUp() {}
void GmDeleteMemberFromGroupTest::TearDown() {}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID3, TEST_APP_ID, g_deleteParams);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GmProcessDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmProcessDataTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmProcessDataTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmProcessDataTest::SetUp() {}
void GmProcessDataTest::TearDown() {}

HWTEST_F(GmProcessDataTest, GmProcessDataTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->processData(TEST_REQ_ID, nullptr, 0);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmProcessDataTest, GmProcessDataTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->processData(TEST_REQ_ID, (const uint8_t *)g_processData, 0);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmProcessDataTest, GmProcessDataTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->processData(TEST_REQ_ID, (const uint8_t *)g_processData, 4097);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmProcessDataTest, GmProcessDataTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->processData(TEST_REQ_ID, (const uint8_t *)g_processData, strlen(g_processData) + 1);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmAddMultiMembersToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmAddMultiMembersToGroupTest::SetUpTestCase() {}
void GmAddMultiMembersToGroupTest::TearDownTestCase() {}

void GmAddMultiMembersToGroupTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmAddMultiMembersToGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams2);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmDelMultiMembersFromGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDelMultiMembersFromGroupTest::SetUpTestCase() {}
void GmDelMultiMembersFromGroupTest::TearDownTestCase() {}

void GmDelMultiMembersFromGroupTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmDelMultiMembersFromGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_deleteParams2);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmGetRegisterInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetRegisterInfoTest::SetUpTestCase() {}
void GmGetRegisterInfoTest::TearDownTestCase() {}

void GmGetRegisterInfoTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetRegisterInfoTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRegisterInfo(nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getRegisterInfo(TEST_QUERY_PARAMS, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRegisterInfo(g_getRegisterInfoParams, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GaAuthDeviceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GaAuthDeviceTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GaAuthDeviceTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GaAuthDeviceTest::SetUp() {}
void GaAuthDeviceTest::TearDown() {}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest001, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, &g_gmCallback);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest002, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, g_authParams, nullptr);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest003, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID4, g_authParams, &g_gaCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GaProcessDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GaProcessDataTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GaProcessDataTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GaProcessDataTest::SetUp() {}
void GaProcessDataTest::TearDown() {}

HWTEST_F(GaProcessDataTest, GaProcessDataTest001, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, nullptr, 0, &g_gaCallback);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GaProcessDataTest, GaProcessDataTest002, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, (const uint8_t *)g_processData, 0, &g_gaCallback);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GaProcessDataTest, GaProcessDataTest003, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, (const uint8_t *)g_processData, 4097, &g_gaCallback);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GaProcessDataTest, GaProcessDataTest004, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, (const uint8_t *)g_processData, strlen(g_processData) + 1, nullptr);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GaProcessDataTest, GaProcessDataTest005, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, (const uint8_t *)g_processData,
        strlen(g_processData) + 1, &g_gaCallback);
    ASSERT_NE(ret, HC_SUCCESS);
}

class GmDeleteGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDeleteGroupTest::SetUpTestCase() {}
void GmDeleteGroupTest::TearDownTestCase() {}

void GmDeleteGroupTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmDeleteGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, g_disbandParams);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    DeleteDemoGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_disbandParams);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
    DeleteDemoGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_disbandParams2);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

class GmUnRegCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmUnRegCallbackTest::SetUpTestCase() {}
void GmUnRegCallbackTest::TearDownTestCase() {}

void GmUnRegCallbackTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmUnRegCallbackTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegCallback(nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmUnRegDataChangeListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmUnRegDataChangeListenerTest::SetUpTestCase() {}
void GmUnRegDataChangeListenerTest::TearDownTestCase() {}

void GmUnRegDataChangeListenerTest::SetUp()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmUnRegDataChangeListenerTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &g_listener);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->unRegDataChangeListener(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegDataChangeListener(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegDataChangeListener(nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmCancelRequestTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmCancelRequestTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmCancelRequestTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GmCancelRequestTest::SetUp() {}
void GmCancelRequestTest::TearDown() {}

HWTEST_F(GmCancelRequestTest, GmCancelRequestTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    gm->cancelRequest(TEST_REQ_ID, nullptr);
}

HWTEST_F(GmCancelRequestTest, GmCancelRequestTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    gm->cancelRequest(TEST_REQ_ID, TEST_APP_ID);
}

class GaCancelRequestTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GaCancelRequestTest::SetUpTestCase()
{
    SetAccessToken();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GaCancelRequestTest::TearDownTestCase()
{
    DestroyDeviceAuthService();
}

void GaCancelRequestTest::SetUp() {}
void GaCancelRequestTest::TearDown() {}

HWTEST_F(GaCancelRequestTest, GaCancelRequestTest001, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    ga->cancelRequest(TEST_REQ_ID, nullptr);
}

HWTEST_F(GaCancelRequestTest, GaCancelRequestTest002, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    ga->cancelRequest(TEST_REQ_ID, TEST_APP_ID);
}
}
