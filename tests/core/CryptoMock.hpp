// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#pragma once

#include <gmock/gmock.h>

#include "opentxs/api/session/Crypto.hpp"

namespace opentxs
{

namespace api::session::internal
{
class Crypto : virtual public opentxs::api::session::Crypto
{
};
}  // namespace api::session::internal

namespace api::internal
{
class Crypto : virtual public opentxs::api::session::Crypto
{
};
}  // namespace api::internal

class CryptoMock : virtual public api::session::internal::Crypto,
                   virtual public api::internal::Crypto
{
public:
    CryptoMock() = default;

    MOCK_METHOD(
        const api::crypto::Asymmetric&,
        Asymmetric,
        (),
        (const, noexcept, final));

    MOCK_METHOD(
        const api::crypto::Blockchain&,
        Blockchain,
        (),
        (const, noexcept, final));

    MOCK_METHOD(const api::crypto::Seed&, Seed, (), (const, noexcept, final));

    MOCK_METHOD(
        const api::crypto::Symmetric&,
        Symmetric,
        (),
        (const, noexcept, final));

    MOCK_METHOD(crypto::Bip32&, BIP32, (), (const, noexcept, final));

    MOCK_METHOD(

        crypto::Bip39&,
        BIP39,
        (),
        (const, noexcept, final));

    MOCK_METHOD(api::crypto::Config&, Config, (), (const, noexcept, final));

    MOCK_METHOD(api::crypto::Encode&, Encode, (), (const, noexcept, final));

    MOCK_METHOD(api::crypto::Hash&, Hash, (), (const, noexcept, final));

    MOCK_METHOD(api::crypto::Util&, Util, (), (const, noexcept, final));

    auto Internal() const noexcept -> const api::internal::Crypto& final
    {
        return *this;
    }

    auto Internal() noexcept -> api::internal::Crypto& final { return *this; }

    auto InternalSession() const noexcept
        -> const api::session::internal::Crypto& final
    {
        return *this;
    }

    auto InternalSession() noexcept -> api::session::internal::Crypto& final
    {
        return *this;
    }
};

}  // namespace opentxs