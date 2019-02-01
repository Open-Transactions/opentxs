// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Pair : virtual public opentxs::api::client::Pair, Lockable
{
public:
    bool AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const std::string& pairingCode) const override;
    bool CheckIssuer(
        const identifier::Nym& localNymID,
        const Identifier& unitDefinitionID) const override;
    std::string IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const override;
    std::set<OTIdentifier> IssuerList(
        const identifier::Nym& localNymID,
        const bool onlyTrusted) const override;
    void Update() const override;

    ~Pair();

    friend class opentxs::api::client::Pair;

private:
    class Cleanup
    {
    private:
        Flag& run_;
        Cleanup() = delete;

    public:
        Cleanup(Flag& run);
        ~Cleanup();
    };

    enum class Status : std::uint8_t {
        Error = 0,
        Started = 1,
        Registered = 2,
    };

    friend opentxs::Factory;
    /// local nym id, issuer nym id
    typedef std::pair<OTNymID, OTNymID> IssuerID;

    const Flag& running_;
    const api::client::Manager& client_;
    mutable std::mutex peer_lock_{};
    mutable std::mutex status_lock_{};
    mutable OTFlag pairing_;
    mutable std::atomic<std::uint64_t> last_refresh_{0};
    mutable std::unique_ptr<std::thread> pairing_thread_{nullptr};
    mutable std::unique_ptr<std::thread> refresh_thread_{nullptr};
    mutable std::map<IssuerID, std::pair<Status, bool>> pair_status_{};
    mutable UniqueQueue<bool> update_;
    OTZMQPublishSocket pair_event_;
    OTZMQPublishSocket pending_bailment_;
    mutable std::atomic<int> next_task_id_;

    void check_pairing() const;
    void check_refresh() const;
    std::map<OTNymID, std::set<OTIdentifier>> create_issuer_map() const;
    std::pair<bool, OTIdentifier> get_connection(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const Identifier& serverID,
        const proto::ConnectionInfoType type) const;
    std::pair<bool, OTIdentifier> initiate_bailment(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& issuerID,
        const Identifier& unitID) const;
    void process_connection_info(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    void process_peer_replies(const Lock& lock, const identifier::Nym& nymID)
        const;
    void process_peer_requests(const Lock& lock, const identifier::Nym& nymID)
        const;
    void process_pending_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerRequest& request) const;
    void process_request_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    void process_request_outbailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    void process_store_secret(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    void queue_nym_download(
        const identifier::Nym& localNymID,
        const identifier::Nym& targetNymID) const;
    void queue_nym_registration(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const bool setData) const;
    void queue_server_contract(
        const identifier::Nym& nymID,
        const Identifier& serverID) const;
    void queue_unit_definition(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    void refresh() const;
    std::pair<bool, OTIdentifier> register_account(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    bool need_registration(
        const identifier::Nym& localNymID,
        const Identifier& serverID) const;
    void state_machine(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const;
    std::pair<bool, OTIdentifier> store_secret(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const Identifier& serverID) const;
    void update_pairing() const;
    void update_peer() const;

    Pair(const Flag& running, const api::client::Manager& client);
    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace opentxs::api::client::implementation
