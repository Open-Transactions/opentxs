// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class EcdsaProvider : virtual public crypto::EcdsaProvider
{
public:
    bool SignDER(
        [[maybe_unused]] const ReadView plaintext,
        [[maybe_unused]] const ReadView key,
        [[maybe_unused]] const crypto::HashType hash,
        [[maybe_unused]] Space& signature) const noexcept override
    {
        return false;
    }

    ~EcdsaProvider() override = default;

protected:
    const api::Crypto& crypto_;

    EcdsaProvider(const api::Crypto& crypto);

private:
    EcdsaProvider() = delete;
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    auto operator=(const EcdsaProvider&) -> EcdsaProvider& = delete;
    auto operator=(EcdsaProvider&&) -> EcdsaProvider& = delete;
};
}  // namespace opentxs::crypto::implementation
