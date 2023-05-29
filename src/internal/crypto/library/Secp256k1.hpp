// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"

namespace opentxs::crypto
{
class Secp256k1 : virtual public EcdsaProvider
{
public:
    using AsymmetricProvider::Init;
    virtual void Init() = 0;

    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;

    ~Secp256k1() override = default;

protected:
    Secp256k1() = default;
};
}  // namespace opentxs::crypto
