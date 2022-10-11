// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/crypto/library/Scrypt.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "opentxs/api/crypto/Util.hpp"

namespace opentxs::crypto
{
class Sodium : virtual public api::crypto::Util,
               virtual public EcdsaProvider,
               virtual public HashingProvider,
               virtual public SymmetricProvider,
               virtual public Scrypt
{
public:
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    auto operator=(const Sodium&) -> Sodium& = delete;
    auto operator=(Sodium&&) -> Sodium& = delete;

    ~Sodium() override = default;

protected:
    Sodium() = default;
};
}  // namespace opentxs::crypto
