// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <tuple>

#include "Basic.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

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
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
enum class Widget : int {
    AccountActivityUSD,
    AccountList,
    AccountSummaryBTC,
    AccountSummaryBCH,
    AccountSummaryUSD,
    ActivitySummary,
    ContactList,
    MessagableList,
    Profile,
    PayableListBTC,
    PayableListBCH,
    ActivityThreadAlice,
    ActivityThreadBob,
    ActivityThreadIssuer,
    ContactIssuer,
    AccountSummary,
};

using WidgetCallback = std::function<bool()>;
// target counter value, callback
using WidgetCallbackData = std::tuple<int, WidgetCallback, std::promise<bool>>;
// name, counter
using WidgetData = std::tuple<Widget, int, WidgetCallbackData>;
using WidgetMap = ot::UnallocatedMap<ot::OTIdentifier, WidgetData>;
using WidgetTypeMap = ot::UnallocatedMap<Widget, ot::OTIdentifier>;
using StateMap = std::map<
    ot::UnallocatedCString,
    ot::UnallocatedMap<Widget, ot::UnallocatedMap<int, WidgetCallback>>>;

struct Server {
    const ot::api::session::Notary* api_{nullptr};
    bool init_{false};
    const ot::OTNotaryID id_{ot::identifier::Notary::Factory()};
    const ot::UnallocatedCString password_;

    auto Contract() const noexcept -> ot::OTServerContract;
    auto Reason() const noexcept -> ot::OTPasswordPrompt;

    auto init(const ot::api::session::Notary& api) noexcept -> void;
};

struct User {
    const ot::UnallocatedCString words_;
    const ot::UnallocatedCString passphrase_;
    const ot::UnallocatedCString name_;
    const ot::UnallocatedCString name_lower_;
    const ot::api::session::Client* api_;
    bool init_;
    ot::UnallocatedCString seed_id_;
    std::uint32_t index_;
    ot::Nym_p nym_;
    ot::OTNymID nym_id_;
    ot::UnallocatedCString payment_code_;

    auto Account(const ot::UnallocatedCString& type) const noexcept
        -> const ot::Identifier&;
    auto Contact(const ot::UnallocatedCString& contact) const noexcept
        -> const ot::Identifier&;
    auto PaymentCode() const -> ot::PaymentCode;
    auto Reason() const noexcept -> ot::OTPasswordPrompt;
    auto SetAccount(
        const ot::UnallocatedCString& type,
        const ot::UnallocatedCString& id) const noexcept -> bool;
    auto SetAccount(
        const ot::UnallocatedCString& type,
        const ot::Identifier& id) const noexcept -> bool;
    auto SetContact(
        const ot::UnallocatedCString& contact,
        const ot::UnallocatedCString& id) const noexcept -> bool;
    auto SetContact(
        const ot::UnallocatedCString& contact,
        const ot::Identifier& id) const noexcept -> bool;

    auto init(
        const ot::api::session::Client& api,
        const ot::identity::Type type = ot::identity::Type::individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> bool;
    auto init(
        const ot::api::session::Client& api,
        const Server& server,
        const ot::identity::Type type = ot::identity::Type::individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> bool;
    auto init_custom(
        const ot::api::session::Client& api,
        const Server& server,
        const std::function<void(User&)> custom,
        const ot::identity::Type type = ot::identity::Type::individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> void;
    auto init_custom(
        const ot::api::session::Client& api,
        const std::function<void(User&)> custom,
        const ot::identity::Type type = ot::identity::Type::individual,
        const std::uint32_t index = 0,
        const ot::crypto::SeedStyle seed =
            ot::crypto::SeedStyle::BIP39) noexcept -> void;

    User(
        const ot::UnallocatedCString words,
        const ot::UnallocatedCString name,
        const ot::UnallocatedCString passphrase = "") noexcept;

private:
    mutable std::mutex lock_;
    mutable ot::UnallocatedMap<ot::UnallocatedCString, ot::OTIdentifier>
        contacts_;
    mutable ot::UnallocatedMap<ot::UnallocatedCString, ot::OTIdentifier>
        accounts_;

    auto init_basic(
        const ot::api::session::Client& api,
        const ot::identity::Type type,
        const std::uint32_t index,
        const ot::crypto::SeedStyle seed) noexcept -> bool;

    User(const User&) = delete;
    User(User&&) = delete;
    User& operator=(const User&) = delete;
    User& operator=(User&&) = delete;
};

struct Callbacks {
    mutable std::mutex callback_lock_;
    ot::OTZMQListenCallback callback_;

    auto Count() const noexcept -> std::size_t;

    auto RegisterWidget(
        const ot::Lock& callbackLock,
        const Widget type,
        const ot::Identifier& id,
        int counter = 0,
        WidgetCallback callback = {}) noexcept -> std::future<bool>;

    auto SetCallback(
        const Widget type,
        int limit,
        WidgetCallback callback) noexcept -> std::future<bool>;

    Callbacks(const ot::UnallocatedCString& name) noexcept;

private:
    mutable std::mutex map_lock_;
    const ot::UnallocatedCString name_;
    WidgetMap widget_map_;
    WidgetTypeMap ui_names_;

    auto callback(ot::network::zeromq::Message&& incoming) noexcept -> void;

    Callbacks() = delete;
};

struct Issuer {
    static const int expected_bailments_{3};
    static const ot::UnallocatedCString new_notary_name_;

    int bailment_counter_;
    std::promise<bool> bailment_promise_;
    std::shared_future<bool> bailment_;
    std::promise<bool> store_secret_promise_;
    std::shared_future<bool> store_secret_;

    Issuer() noexcept;
};

class IntegrationFixture : public ::testing::Test
{
public:
    static const User alex_;
    static const User bob_;
    static const User issuer_;
    static const User chris_;
    static const Server server_1_;
};

auto set_introduction_server(
    const ot::api::session::Client& api,
    const Server& server) noexcept -> void;
auto test_future(
    std::future<bool>& future,
    const unsigned int seconds = 60) noexcept -> bool;
}  // namespace ottest
