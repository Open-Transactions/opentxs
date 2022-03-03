// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto
{
class OPENTXS_EXPORT EcdsaProvider : virtual public AsymmetricProvider
{
public:
    virtual auto PubkeyAdd(
        const ReadView pubkey,
        const ReadView scalar,
        const AllocateOutput result) const noexcept -> bool = 0;
    virtual auto ScalarAdd(
        const ReadView lhs,
        const ReadView rhs,
        const AllocateOutput result) const noexcept -> bool = 0;
    virtual auto ScalarMultiplyBase(
        const ReadView scalar,
        const AllocateOutput result) const noexcept -> bool = 0;
    virtual auto SignDER(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        Space& signature) const noexcept -> bool = 0;

    ~EcdsaProvider() override = default;

protected:
    EcdsaProvider() = default;

private:
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    auto operator=(const EcdsaProvider&) -> EcdsaProvider& = delete;
    auto operator=(EcdsaProvider&&) -> EcdsaProvider& = delete;
};
}  // namespace opentxs::crypto
