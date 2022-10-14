// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class EcdsaProvider : virtual public AsymmetricProvider
{
public:
    virtual auto PubkeyAdd(ReadView pubkey, ReadView scalar, Writer&& result)
        const noexcept -> bool = 0;
    virtual auto ScalarAdd(ReadView lhs, ReadView rhs, Writer&& result)
        const noexcept -> bool = 0;
    virtual auto ScalarMultiplyBase(ReadView scalar, Writer&& result)
        const noexcept -> bool = 0;
    virtual auto SignDER(
        ReadView plaintext,
        ReadView key,
        crypto::HashType hash,
        Writer&& signature) const noexcept -> bool = 0;

    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    auto operator=(const EcdsaProvider&) -> EcdsaProvider& = delete;
    auto operator=(EcdsaProvider&&) -> EcdsaProvider& = delete;

    ~EcdsaProvider() override = default;

protected:
    EcdsaProvider() = default;
};
}  // namespace opentxs::crypto
