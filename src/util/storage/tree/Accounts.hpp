// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageAccounts.pb.h>
#include <functional>
#include <string_view>
#include <tuple>

#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Trunk;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Accounts final : public Node
{
public:
    auto AccountContract(const identifier::Account& accountID) const
        -> identifier::UnitDefinition;
    auto AccountIssuer(const identifier::Account& accountID) const
        -> identifier::Nym;
    auto AccountOwner(const identifier::Account& accountID) const
        -> identifier::Nym;
    auto AccountServer(const identifier::Account& accountID) const
        -> identifier::Notary;
    auto AccountSigner(const identifier::Account& accountID) const
        -> identifier::Nym;
    auto AccountUnit(const identifier::Account& accountID) const -> UnitType;
    auto AccountsByContract(const identifier::UnitDefinition& unit) const
        -> UnallocatedSet<identifier::Account>;
    auto AccountsByIssuer(const identifier::Nym& issuerNym) const
        -> UnallocatedSet<identifier::Account>;
    auto AccountsByOwner(const identifier::Nym& ownerNym) const
        -> UnallocatedSet<identifier::Account>;
    auto AccountsByServer(const identifier::Notary& server) const
        -> UnallocatedSet<identifier::Account>;
    auto AccountsByUnit(const UnitType unit) const
        -> UnallocatedSet<identifier::Account>;
    auto Alias(const identifier::Account& id) const -> UnallocatedCString;
    auto Load(
        const identifier::Account& id,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;

    auto Delete(const identifier::Account& id) -> bool;
    auto SetAlias(const identifier::Account& id, std::string_view alias)
        -> bool;
    auto Store(
        const identifier::Account& id,
        const UnallocatedCString& data,
        std::string_view alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) -> bool;

    Accounts() = delete;
    Accounts(const Accounts&) = delete;
    Accounts(Accounts&&) = delete;
    auto operator=(const Accounts&) -> Accounts = delete;
    auto operator=(Accounts&&) -> Accounts = delete;

    ~Accounts() final = default;

private:
    friend Trunk;

    using NymIndex =
        UnallocatedMap<identifier::Nym, UnallocatedSet<identifier::Account>>;
    using ServerIndex =
        UnallocatedMap<identifier::Notary, UnallocatedSet<identifier::Account>>;
    using ContractIndex = UnallocatedMap<
        identifier::UnitDefinition,
        UnallocatedSet<identifier::Account>>;
    using UnitIndex =
        UnallocatedMap<UnitType, UnallocatedSet<identifier::Account>>;
    /** owner, signer, issuer, server, contract, unit */
    using AccountData = std::tuple<
        identifier::Nym,
        identifier::Nym,
        identifier::Nym,
        identifier::Notary,
        identifier::UnitDefinition,
        UnitType>;
    using ReverseIndex = UnallocatedMap<identifier::Account, AccountData>;

    NymIndex owner_index_{};
    NymIndex signer_index_{};
    NymIndex issuer_index_{};
    ServerIndex server_index_{};
    ContractIndex contract_index_{};
    UnitIndex unit_index_{};
    mutable ReverseIndex account_data_{};

    template <typename A, typename M, typename I>
    static auto add_set_index(
        const api::Crypto& crypto,
        const identifier::Account& accountID,
        const A& argID,
        M& mapID,
        I& index) -> bool;

    template <typename K, typename I>
    static void erase(
        const identifier::Account& accountID,
        const K& key,
        I& index)
    {
        try {
            auto& set = index.at(key);
            set.erase(accountID);

            if (0 == set.size()) { index.erase(key); }
        } catch (...) {
        }
    }

    auto get_account_data(
        const Lock& lock,
        const identifier::Account& accountID) const -> AccountData&;
    auto serialize() const -> proto::StorageAccounts;

    auto check_update_account(
        const Lock& lock,
        const identifier::Account& accountID,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& contract,
        const UnitType unit) -> bool;
    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const Lock& lock) const -> bool final;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Accounts(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& key);
};
}  // namespace opentxs::storage::tree
