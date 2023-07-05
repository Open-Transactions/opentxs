// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <locale>
#include <optional>
#include <string_view>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class AccountVisitor;
class Factory;
class PasswordPrompt;
class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::implementation
{
class Unit : virtual public contract::Unit,
             public opentxs::contract::implementation::Signable<
                 identifier::UnitDefinition>
{
public:
    static const UnallocatedMap<VersionNumber, VersionNumber>
        unit_of_account_version_map_;

    static auto GetID(const api::Session& api, const SerializedType& contract)
        -> identifier_type;

    auto AddAccountRecord(
        const UnallocatedCString& dataFolder,
        const Account& theAccount) const -> bool override;
    auto DisplayStatistics(String& strContents) const -> bool override;
    auto EraseAccountRecord(
        const UnallocatedCString& dataFolder,
        const identifier::Account& theAcctID) const -> bool override;
    auto Name() const noexcept -> std::string_view override
    {
        return short_name_;
    }
    auto Serialize(Writer&& out) const noexcept -> bool override;
    auto Serialize(Writer&& destination, bool includeNym = false) const
        -> bool override;
    auto Serialize(SerializedType&, bool includeNym = false) const
        -> bool override;
    auto Type() const -> contract::UnitType override = 0;
    auto UnitOfAccount() const -> opentxs::UnitType override
    {
        return unit_of_account_;
    }
    auto VisitAccountRecords(
        const UnallocatedCString& dataFolder,
        AccountVisitor& visitor,
        const PasswordPrompt& reason) const -> bool override;

    auto InitAlias(std::string_view alias) -> void final
    {
        Signable::SetAlias(alias);
    }
    auto SetAlias(std::string_view alias) noexcept -> bool override;

    Unit(Unit&&) = delete;
    auto operator=(const Unit&) -> Unit& = delete;
    auto operator=(Unit&&) -> Unit& = delete;

    ~Unit() override = default;

protected:
    const opentxs::UnitType unit_of_account_;

    virtual auto IDVersion() const -> SerializedType;
    virtual auto SigVersion() const -> SerializedType;
    auto validate() const -> bool override;

    auto update_signature(const PasswordPrompt& reason) -> bool override;

    Unit(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const opentxs::UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement);
    Unit(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType serialized);
    Unit(const Unit&);

private:
    friend opentxs::Factory;

    struct Locale : std::numpunct<char> {
    };

    static const Locale locale_;

    const std::optional<display::Definition> display_definition_;
    const Amount redemption_increment_;
    const UnallocatedCString short_name_;

    auto calculate_id() const -> identifier_type final;
    auto contract() const -> SerializedType;
    auto get_displayscales(const SerializedType&) const
        -> std::optional<display::Definition>;
    auto get_unitofaccount(const SerializedType&) const -> opentxs::UnitType;
    auto verify_signature(const proto::Signature& signature) const
        -> bool override;
};
}  // namespace opentxs::contract::implementation
