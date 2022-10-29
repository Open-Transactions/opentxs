// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Test_Nym : public ::testing::Test
{
public:
    static const bool have_hd_;
    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;

    const ot::api::session::Client& client_;
#if OT_STORAGE_FS
    const ot::api::session::Client& client_fs_;
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
    const ot::api::session::Client& client_sqlite_;
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
    const ot::api::session::Client& client_lmdb_;
#endif  // OT_STORAGE_LMDB
    const ot::PasswordPrompt reason_;

    auto test_nym(
        const ot::crypto::ParameterType type,
        const ot::identity::CredentialType cred,
        const ot::identity::SourceType source,
        const ot::UnallocatedCString& name = "Nym") -> bool;
    auto test_storage(const ot::api::session::Client& api) -> bool;

    Test_Nym();
};
}  // namespace ottest
