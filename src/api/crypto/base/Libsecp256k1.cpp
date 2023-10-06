// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/base/Crypto.hpp"  // IWYU pragma: associated

#include "internal/crypto/library/Secp256k1.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::imp
{
auto Crypto::Init_Libsecp256k1() noexcept -> void
{
    assert_false(nullptr == secp256k1_);

    secp256k1_->Init();
}

auto Crypto::Libsecp256k1() const noexcept -> const opentxs::crypto::Secp256k1&
{
    return *secp256k1_;
}
}  // namespace opentxs::api::imp
