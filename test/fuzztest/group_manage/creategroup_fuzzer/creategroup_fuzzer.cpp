/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "creategroup_fuzzer.h"

namespace OHOS {
    bool FuzzDoCreateGroup(const uint8_t* data, size_t size)
    {
        const DeviceGroupManager *gmInstance = GetGmInstance();
        if (gmInstance == nullptr) {
            return false;
        }
        if (data == nullptr) {
            return false;
        }
        if (size < sizeof(int64_t)) {
            return false;
        }
        const int32_t *osAccountId = reinterpret_cast<const int32_t *>(data);
        const int64_t *requestId = reinterpret_cast<const int64_t *>(data);
        std::string appId(reinterpret_cast<const char *>(data), size);
        std::string createParams(reinterpret_cast<const char *>(data), size);
        gmInstance->createGroup(*osAccountId, *requestId, appId.c_str(), createParams.c_str());
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzDoCreateGroup(data, size);
    return 0;
}

