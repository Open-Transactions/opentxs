// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "internal/otx/blind/Mint.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Token;
}  // namespace blind
}  // namespace otx

class Armored;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
class Mint::Imp : public otx::blind::internal::Mint
{
public:
    auto AccountID() const -> identifier::Account override;
    auto Expired() const -> bool override { return {}; }
    auto GetDenomination(std::int32_t) const -> Amount override { return {}; }
    auto GetDenominationCount() const -> std::int32_t override { return {}; }

    auto GetExpiration() const -> Time override { return {}; }
    auto GetLargestDenomination(const Amount&) const -> Amount override
    {
        return {};
    }
    auto GetPrivate(Armored&, const Amount&) const -> bool override
    {
        return {};
    }
    auto GetPublic(Armored&, const Amount&) const -> bool override
    {
        return {};
    }
    auto GetSeries() const -> std::int32_t override { return {}; }
    auto GetValidFrom() const -> Time override { return {}; }
    auto GetValidTo() const -> Time override { return {}; }
    auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& override;
    virtual auto isValid() const noexcept -> bool { return false; }

    auto AddDenomination(
        const identity::Nym&,
        const Amount&,
        const std::size_t,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }
    auto GenerateNewMint(
        const api::session::Wallet&,
        const std::int32_t,
        const Time,
        const Time,
        const Time,
        const identifier::UnitDefinition&,
        const identifier::Notary&,
        const identity::Nym&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const Amount&,
        const std::size_t,
        const PasswordPrompt&) -> void override
    {
    }
    auto LoadContract() -> bool override { return {}; }
    auto LoadMint(std::string_view) -> bool override { return {}; }
    auto Release() -> void override {}
    auto Release_Mint() -> void override;
    auto ReleaseDenominations() -> void override {}
    auto SaveMint(std::string_view extension) -> bool override { return {}; }
    auto SetInstrumentDefinitionID(const identifier::UnitDefinition&)
        -> void override
    {
    }
    auto SetSavePrivateKeys(bool) -> void override {}
    auto SignToken(
        const identity::Nym&,
        opentxs::otx::blind::Token&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }
    auto UpdateContents(const PasswordPrompt& reason) -> void override {}
    auto VerifyContractID() const -> bool override { return {}; }
    auto VerifyMint(const identity::Nym& theOperator) -> bool override
    {
        return {};
    }
    auto VerifyToken(
        const identity::Nym&,
        const opentxs::otx::blind::Token&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }

    Imp(const api::Session& api) noexcept;
    Imp(const api::Session& api,
        const identifier::UnitDefinition& unit) noexcept;
    Imp() = delete;

    ~Imp() override;

private:
    bool need_release_;
};
}  // namespace opentxs::otx::blind
