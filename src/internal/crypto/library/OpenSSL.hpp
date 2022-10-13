// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"

namespace opentxs::crypto
{
class OpenSSL : virtual public HashingProvider,
                virtual public Pbkdf2,
                virtual public Ripemd160,
                virtual public AsymmetricProvider
{
public:
    OpenSSL(const OpenSSL&) = delete;
    OpenSSL(OpenSSL&&) = delete;
    auto operator=(const OpenSSL&) -> OpenSSL& = delete;
    auto operator=(OpenSSL&&) -> OpenSSL& = delete;

    ~OpenSSL() override = default;

protected:
    OpenSSL() = default;
};
}  // namespace opentxs::crypto
