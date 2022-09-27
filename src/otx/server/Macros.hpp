// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Log.hpp"
#include "otx/server/ServerSettings.hpp"

namespace opentxs::server
{
// For NYM_IS_ALLOWED() to evaluate to TRUE, either the boolean value itself is
// set to true (meaning, "YES any Nym is allowed..") OR (it only continues if
// that part fails) if the override Nym's ID matches to the Nym ID passed in (as
// a const char *).
#define NYM_IS_ALLOWED(SZ_NYM_ID, BOOL_VAR_NAME)                               \
    ((BOOL_VAR_NAME) ||                                                        \
     ((ServerSettings::GetOverrideNymID().size() > 0) &&                       \
      (0 == ServerSettings::GetOverrideNymID().compare((SZ_NYM_ID)))))

#define OT_ENFORCE_PERMISSION_MSG(BOOL_VAR_NAME)                               \
    {                                                                          \
        const char* pNymAllowedIDStr = msgIn.nym_id_->Get();                   \
        const char* pActionNameStr = msgIn.command_->Get();                    \
                                                                               \
        if (false == NYM_IS_ALLOWED(pNymAllowedIDStr, BOOL_VAR_NAME)) {        \
            LogConsole()(OT_PRETTY_CLASS())("Nym ")(                           \
                pNymAllowedIDStr)(" attempted an action (")(                   \
                pActionNameStr)("), but based on server configuration, he's "  \
                                "not allowed.")                                \
                .Flush();                                                      \
            return false;                                                      \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt
}  // namespace opentxs::server
