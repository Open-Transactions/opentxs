// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string_view>

#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/common/Contract.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/blind/mint/Mint.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class PasswordPrompt;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::mint
{
class Mint : public blind::Mint::Imp
{
public:
    auto AccountID() const -> identifier::Generic override
    {
        return cash_account_id_;
    }
    auto Expired() const -> bool override;
    auto GetDenomination(std::int32_t nIndex) const -> Amount override;
    auto GetDenominationCount() const -> std::int32_t override
    {
        return denomination_count_;
    }

    auto GetExpiration() const -> Time override { return expiration_; }
    auto GetLargestDenomination(const Amount& lAmount) const -> Amount override;
    auto GetPrivate(Armored& theArmor, const Amount& lDenomination) const
        -> bool override;
    auto GetPublic(Armored& theArmor, const Amount& lDenomination) const
        -> bool override;
    auto GetSeries() const -> std::int32_t override { return series_; }
    auto GetValidFrom() const -> Time override { return valid_from_; }
    auto GetValidTo() const -> Time override { return valid_to_; }
    auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& override
    {
        return instrument_definition_id_;
    }
    auto isValid() const noexcept -> bool final { return true; }

    void GenerateNewMint(
        const api::session::Wallet& wallet,
        const std::int32_t nSeries,
        const Time VALID_FROM,
        const Time VALID_TO,
        const Time MINT_EXPIRATION,
        const identifier::UnitDefinition& theInstrumentDefinitionID,
        const identifier::Notary& theNotaryID,
        const identity::Nym& theNotary,
        const Amount& nDenom1,
        const Amount& nDenom2,
        const Amount& nDenom3,
        const Amount& nDenom4,
        const Amount& nDenom5,
        const Amount& nDenom6,
        const Amount& nDenom7,
        const Amount& nDenom8,
        const Amount& nDenom9,
        const Amount& nDenom10,
        const std::size_t keySize,
        const PasswordPrompt& reason) override;
    auto LoadContract() -> bool override;
    auto LoadMint(std::string_view extension) -> bool override;
    void Release() override;
    void Release_Mint() override;
    void ReleaseDenominations() override;
    auto SaveMint(std::string_view extension) -> bool override;
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) override
    {
        instrument_definition_id_ = newID;
    }
    void SetSavePrivateKeys(bool bDoIt = true) override
    {
        save_private_keys_ = bDoIt;
    }
    void UpdateContents(const PasswordPrompt& reason) override;
    auto VerifyContractID() const -> bool override;
    auto VerifyMint(const identity::Nym& theOperator) -> bool override;

    Mint() = delete;

    ~Mint() override;

protected:
    using mapOfArmor = UnallocatedMap<Amount, OTArmored>;

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void InitMint();

    mapOfArmor private_;
    mapOfArmor public_;
    identifier::Notary notary_id_;
    identifier::Nym server_nym_id_;
    identifier::UnitDefinition instrument_definition_id_;
    std::int32_t denomination_count_;
    bool save_private_keys_;
    std::int32_t series_;
    Time valid_from_;
    Time valid_to_;
    Time expiration_;
    identifier::Generic cash_account_id_;

    Mint(const api::Session& api);
    Mint(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit);
    Mint(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit);
};
}  // namespace opentxs::otx::blind::mint
