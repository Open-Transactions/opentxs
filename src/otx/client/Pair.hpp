// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "core/StateMachine.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class Data;
class Section;
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
class Issuer;
}  // namespace client
}  // namespace otx

namespace proto
{
class PeerReply;
class PeerRequest;
}  // namespace proto

class Flag;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::otx::client::implementation
{
class Pair final : virtual public otx::client::Pair,
                   Lockable,
                   opentxs::internal::StateMachine
{
public:
    auto AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const UnallocatedCString& pairingCode) const noexcept -> bool final;
    auto CheckIssuer(
        const identifier::Nym& localNymID,
        const identifier::UnitDefinition& unitDefinitionID) const noexcept
        -> bool final;
    auto IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const noexcept
        -> UnallocatedCString final;
    auto IssuerList(const identifier::Nym& localNymID, const bool onlyTrusted)
        const noexcept -> UnallocatedSet<identifier::Nym> final
    {
        return state_.IssuerList(localNymID, onlyTrusted);
    }
    auto Stop() const noexcept -> std::shared_future<void> final
    {
        return cleanup();
    }
    auto Wait() const noexcept -> std::shared_future<void> final
    {
        return StateMachine::Wait();
    }

    void init() noexcept final;

    Pair(const Flag& running, const api::session::Client& client);
    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    auto operator=(const Pair&) -> Pair& = delete;
    auto operator=(Pair&&) -> Pair& = delete;

    ~Pair() final { cleanup().get(); }

private:
    /// local nym id, issuer nym id
    using IssuerID = std::pair<identifier::Nym, identifier::Nym>;
    enum class Status : std::uint8_t {
        Error = 0,
        Started = 1,
        Registered = 2,
    };

    struct State {
        using OfferedCurrencies = std::size_t;
        using RegisteredAccounts = std::size_t;
        using UnusedBailments = std::size_t;
        using NeedRename = bool;
        using AccountDetails = std::tuple<
            identifier::UnitDefinition,
            identifier::Generic,
            UnusedBailments>;
        using Trusted = bool;
        using Details = std::tuple<
            std::unique_ptr<std::mutex>,
            identifier::Notary,
            identifier::Nym,
            Status,
            Trusted,
            OfferedCurrencies,
            RegisteredAccounts,
            UnallocatedVector<AccountDetails>,
            UnallocatedVector<api::session::OTX::BackgroundTask>,
            NeedRename>;
        using StateMap = UnallocatedMap<IssuerID, Details>;

        static auto count_currencies(
            const UnallocatedVector<AccountDetails>& in) noexcept
            -> std::size_t;
        static auto count_currencies(
            const api::Session& api,
            const identity::wot::claim::Section& in) noexcept -> std::size_t;
        static auto get_account(
            const api::session::Client& client,
            const identifier::UnitDefinition& unit,
            const identifier::Generic& account,
            UnallocatedVector<AccountDetails>& details) noexcept
            -> AccountDetails&;

        auto CheckIssuer(const identifier::Nym& id) const noexcept -> bool;
        auto check_state() const noexcept -> bool;

        void Add(
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID,
            const bool trusted) noexcept;
        void Add(
            const Lock& lock,
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID,
            const bool trusted) noexcept;
        auto begin() noexcept -> StateMap::iterator { return state_.begin(); }
        auto end() noexcept -> StateMap::iterator { return state_.end(); }
        auto GetDetails(
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID) noexcept -> StateMap::iterator;
        auto run(const std::function<void(const IssuerID&)> fn) noexcept
            -> bool;

        auto IssuerList(
            const identifier::Nym& localNymID,
            const bool onlyTrusted) const noexcept
            -> UnallocatedSet<identifier::Nym>;

        State(std::mutex& lock) noexcept;

    private:
        std::mutex& lock_;
        mutable StateMap state_;
        UnallocatedSet<identifier::Nym> issuers_;
    };

    const Flag& running_;
    const api::session::Client& client_;
    mutable State state_;
    std::promise<void> startup_promise_;
    std::shared_future<void> startup_;
    OTZMQListenCallback nym_callback_;
    OTZMQListenCallback peer_reply_callback_;
    OTZMQListenCallback peer_request_callback_;
    OTZMQPublishSocket pair_event_;
    OTZMQPublishSocket pending_bailment_;
    OTZMQSubscribeSocket nym_subscriber_;
    OTZMQSubscribeSocket peer_reply_subscriber_;
    OTZMQSubscribeSocket peer_request_subscriber_;

    void check_accounts(
        const identity::wot::claim::Data& issuerClaims,
        otx::client::Issuer& issuer,
        const identifier::Notary& serverID,
        std::size_t& offered,
        std::size_t& registeredAccounts,
        UnallocatedVector<State::AccountDetails>& accountDetails)
        const noexcept;
    void check_connection_info(
        otx::client::Issuer& issuer,
        const identifier::Notary& serverID) const noexcept;
    void check_rename(
        const otx::client::Issuer& issuer,
        const identifier::Notary& serverID,
        const PasswordPrompt& reason,
        bool& needRename) const noexcept;
    void check_store_secret(
        otx::client::Issuer& issuer,
        const identifier::Notary& serverID) const noexcept;
    auto cleanup() const noexcept -> std::shared_future<void>;
    auto get_connection(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const identifier::Notary& serverID,
        const contract::peer::ConnectionInfoType type) const
        -> std::pair<bool, identifier::Generic>;
    auto initiate_bailment(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::Nym& issuerID,
        const identifier::UnitDefinition& unitID) const
        -> std::pair<bool, identifier::Generic>;
    auto process_connection_info(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const -> bool;
    void process_peer_replies(const Lock& lock, const identifier::Nym& nymID)
        const;
    void process_peer_requests(const Lock& lock, const identifier::Nym& nymID)
        const;
    auto process_pending_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerRequest& request) const -> bool;
    auto process_request_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const -> bool;
    auto process_request_outbailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const -> bool;
    auto process_store_secret(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const -> bool;
    auto queue_nym_download(
        const identifier::Nym& localNymID,
        const identifier::Nym& targetNymID) const
        -> api::session::OTX::BackgroundTask;
    auto queue_nym_registration(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const bool setData) const -> api::session::OTX::BackgroundTask;
    auto queue_server_contract(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID) const
        -> api::session::OTX::BackgroundTask;
    void queue_unit_definition(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID) const;
    auto register_account(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID) const
        -> std::pair<bool, identifier::Generic>;
    auto need_registration(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> bool;
    void state_machine(const IssuerID& id) const;
    auto store_secret(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const identifier::Notary& serverID) const
        -> std::pair<bool, identifier::Generic>;

    void callback_nym(const zmq::Message& in) noexcept;
    void callback_peer_reply(const zmq::Message& in) noexcept;
    void callback_peer_request(const zmq::Message& in) noexcept;
};
}  // namespace opentxs::otx::client::implementation
