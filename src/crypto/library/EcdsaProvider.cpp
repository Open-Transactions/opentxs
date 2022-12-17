// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/EcdsaProvider.hpp"  // IWYU pragma: associated

#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::implementation
{
EcdsaProvider::EcdsaProvider(const api::Crypto& crypto)
    : crypto_(crypto)
{
}

auto EcdsaProvider::SignDER(
    ReadView plaintext,
    ReadView key,
    crypto::HashType,
    Writer&&) const noexcept -> bool
{
    return false;
}

EcdsaProvider::~EcdsaProvider() = default;
}  // namespace opentxs::crypto::implementation
