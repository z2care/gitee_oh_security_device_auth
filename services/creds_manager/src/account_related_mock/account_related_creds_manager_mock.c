/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "account_related_creds_manager.h"

#include "device_auth_defines.h"

int32_t GetAccountRelatedCredInfo(int32_t osAccountId, const char *groupId, const char *deviceId,
    bool isUdid, IdentityInfo *info)
{
    (void)osAccountId;
    (void)groupId;
    (void)deviceId;
    (void)isUdid;
    (void)info;
    return HC_ERR_NOT_SUPPORT;
}

int32_t GetAccountAsymSharedSecret(int32_t osAccountId, const CertInfo *peerCertInfo, Uint8Buff *sharedSecret)
{
    (void)osAccountId;
    (void)peerCertInfo;
    (void)sharedSecret;
    return HC_ERR_NOT_SUPPORT;
}

int32_t GetAccountSymSharedSecret(const CJson *in, const CJson *urlJson, Uint8Buff *sharedSecret)
{
    (void)in;
    (void)urlJson;
    (void)sharedSecret;
    return HC_ERR_NOT_SUPPORT;
}

int32_t GetAccountAsymCredInfo(int32_t osAccountId, const CertInfo *certInfo, IdentityInfo **returnInfo)
{
    (void)osAccountId;
    (void)certInfo;
    (void)returnInfo;
    return HC_ERR_NOT_SUPPORT;
}

int32_t GetAccountSymCredInfoByPeerUrl(const CJson *in, const CJson *urlJson, IdentityInfo *info)
{
    (void)in;
    (void)urlJson;
    (void)info;
    return HC_ERR_NOT_SUPPORT;
}