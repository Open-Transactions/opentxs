// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "opentxs/core/Amount.hpp"
#include "otx/blind/mint/Imp.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Mint;
class Token;
}  // namespace blind
}  // namespace otx

class PasswordPrompt;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::mint
{
class Lucre final : public mint::Mint
{
public:
    auto AddDenomination(
        const identity::Nym& theNotary,
        const Amount& denomination,
        const std::size_t keySize,
        const PasswordPrompt& reason) -> bool final;

    auto SignToken(
        const identity::Nym& notary,
        opentxs::otx::blind::Token& token,
        const PasswordPrompt& reason) -> bool final;
    auto VerifyToken(
        const identity::Nym& notary,
        const opentxs::otx::blind::Token& token,
        const PasswordPrompt& reason) -> bool final;

    Lucre(const api::Session& api);
    Lucre(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit);
    Lucre(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit);

    ~Lucre() final = default;
};
}  // namespace opentxs::otx::blind::mint
