// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"
// IWYU pragma: no_include "opentxs/crypto/Language.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#pragma once

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>

#include "integration/Helpers.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
class Notary;
}  // namespace session

class Context;
class Session;
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Amount;
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct AccountActivityData;
struct AccountListData;
struct Counter;
struct User;
}  // namespace ottest

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
auto check_account_activity_rpc(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool;
auto check_account_list_rpc(
    const User& user,
    const AccountListData& expected) noexcept -> bool;

class RPCPushCounter
{
public:
    auto at(std::size_t index) const -> const zmq::Message&;
    auto size() const noexcept -> std::size_t;
    auto wait(std::size_t index) const noexcept -> bool;

    auto shutdown() noexcept -> void;

    RPCPushCounter(const ot::api::Context& ot) noexcept;

    ~RPCPushCounter();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    RPCPushCounter(const RPCPushCounter&) = delete;
    RPCPushCounter(RPCPushCounter&&) = delete;
    auto operator=(const RPCPushCounter&) -> RPCPushCounter& = delete;
    auto operator=(RPCPushCounter&&) -> RPCPushCounter& = delete;
};

class RPC_fixture : virtual public ::testing::Test
{
protected:
    using AccountMap = ot::UnallocatedMap<
        ot::UnallocatedCString,
        ot::UnallocatedVector<ot::UnallocatedCString>>;
    using Amount = ot::Amount;
    using IssuedUnits = ot::UnallocatedVector<ot::UnallocatedCString>;
    using LocalNymMap =
        ot::UnallocatedMap<int, ot::UnallocatedSet<ot::UnallocatedCString>>;
    using SeedMap =
        ot::UnallocatedMap<int, ot::UnallocatedSet<ot::UnallocatedCString>>;
    using UserIndex = ot::UnallocatedMap<int, User>;

    static SeedMap seed_map_;
    static LocalNymMap local_nym_map_;
    static IssuedUnits created_units_;
    static AccountMap registered_accounts_;
    static UserIndex users_;

    const ot::api::Context& ot_;
    RPCPushCounter& push_;

    auto CreateNym(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& name,
        const ot::UnallocatedCString& seed,
        int index) const noexcept -> const User&;
    auto DepositCheques(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& nym) const noexcept -> std::size_t;
    auto DepositCheques(const ot::api::session::Notary& server, const User& nym)
        const noexcept -> std::size_t;
    auto DepositCheques(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& nym) const noexcept -> std::size_t;
    auto ImportBip39(
        const ot::api::Session& api,
        const ot::UnallocatedCString& words) const noexcept
        -> ot::UnallocatedCString;
    auto ImportServerContract(
        const ot::api::session::Notary& from,
        const ot::api::session::Client& to) const noexcept -> bool;
    auto InitAccountActivityCounter(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& nym,
        const ot::UnallocatedCString& account,
        Counter& counter) const noexcept -> void;
    auto InitAccountActivityCounter(
        const User& nym,
        const ot::UnallocatedCString& account,
        Counter& counter) const noexcept -> void;
    auto InitAccountActivityCounter(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nym,
        const ot::UnallocatedCString& account,
        Counter& counter) const noexcept -> void;
    auto InitAccountTreeCounter(const User& nym, Counter& counter)
        const noexcept -> void;
    auto InitAccountTreeCounter(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nym,
        Counter& counter) const noexcept -> void;
    auto IssueUnit(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount,
        const ot::display::Definition& displayDefinition) const noexcept
        -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Notary& server,
        const User& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount,
        const ot::display::Definition& displayDefinition) const noexcept
        -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Notary& server,
        const User& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount,
        const ot::display::Definition& displayDefinition) const noexcept
        -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString;
    auto RefreshAccount(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nym,
        const ot::identifier::Notary& server) const noexcept -> void;
    auto RefreshAccount(
        const ot::api::session::Client& api,
        const ot::UnallocatedVector<ot::UnallocatedCString> nyms,
        const ot::identifier::Notary& server) const noexcept -> void;
    auto RefreshAccount(
        const ot::api::session::Client& api,
        const ot::UnallocatedVector<const User*> nyms,
        const ot::identifier::Notary& server) const noexcept -> void;
    auto RegisterAccount(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& nym,
        const ot::UnallocatedCString& unit,
        const ot::UnallocatedCString& label) const noexcept
        -> ot::UnallocatedCString;
    auto RegisterAccount(
        const ot::api::session::Notary& server,
        const User& nym,
        const ot::UnallocatedCString& unit,
        const ot::UnallocatedCString& label) const noexcept
        -> ot::UnallocatedCString;
    auto RegisterAccount(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& nym,
        const ot::UnallocatedCString& unit,
        const ot::UnallocatedCString& label) const noexcept
        -> ot::UnallocatedCString;
    auto RegisterNym(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& nymID) const noexcept -> bool;
    auto RegisterNym(const ot::api::session::Notary& server, const User& nym)
        const noexcept -> bool;
    auto RegisterNym(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& nym) const noexcept -> bool;
    auto SendCheque(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& nym,
        const ot::UnallocatedCString& account,
        const ot::UnallocatedCString& contact,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SendCheque(
        const ot::api::session::Notary& server,
        const User& nym,
        const ot::UnallocatedCString& account,
        const ot::UnallocatedCString& contact,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SendCheque(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& nym,
        const ot::UnallocatedCString& account,
        const ot::UnallocatedCString& contact,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SendTransfer(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& sender,
        const ot::UnallocatedCString& fromAccount,
        const ot::UnallocatedCString& toAccount,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SendTransfer(
        const ot::api::session::Notary& server,
        const User& sender,
        const ot::UnallocatedCString& fromAccount,
        const ot::UnallocatedCString& toAccount,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SendTransfer(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& sender,
        const ot::UnallocatedCString& fromAccount,
        const ot::UnallocatedCString& toAccount,
        const ot::UnallocatedCString& memo,
        Amount amount) const noexcept -> bool;
    auto SetIntroductionServer(
        const ot::api::session::Client& on,
        const ot::api::session::Notary& to) const noexcept -> bool;
    auto StartClient(int index) const noexcept
        -> const ot::api::session::Client&;
    auto StartNotarySession(int index) const noexcept
        -> const ot::api::session::Notary&;

    virtual auto Cleanup() noexcept -> void;

    RPC_fixture() noexcept;

private:
    auto init_maps(int instance) const noexcept -> void;
};
}  // namespace ottest
