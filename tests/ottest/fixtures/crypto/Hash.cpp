// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/Hash.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "internal/crypto/symmetric/Key.hpp"
#include "internal/util/LogMacros.hpp"

namespace ottest
{
Test_Hash::Test_Hash()
    : crypto_(ot::Context().Crypto())
{
}

auto Test_Hash::GetHash(
    const ot::api::Session& api,
    const ot::crypto::symmetric::Key& key,
    const ot::PasswordPrompt& reason,
    std::size_t bytes) const noexcept -> ot::ByteArray
{
    auto secret = api.Factory().Secret(bytes);
    const auto rc = key.Internal().RawKey(secret, reason);

    OT_ASSERT(rc);

    return api.Factory().DataFromBytes(secret.Bytes());
}
}  // namespace ottest
