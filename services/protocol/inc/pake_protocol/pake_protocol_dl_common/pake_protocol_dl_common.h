/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef PAKE_PROTOCOL_DL_COMMON_H
#define PAKE_PROTOCOL_DL_COMMON_H

#include "hc_types.h"
#include "pake_defs.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t GetPakeDlAlg(void);
int32_t GenerateDlPakeParams(PakeBaseParams *params, const Uint8Buff *secret);
int32_t AgreeDlSharedSecret(PakeBaseParams *params, Uint8Buff *sharedSecret);

#ifdef __cplusplus
}
#endif
#endif
