// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"

#include "internal/consensus/Internal.hpp"

#include <map>
#include <tuple>

namespace opentxs::api::implementation
{
class Wallet : virtual public opentxs::api::Wallet, public Lockable
{
public:
    SharedAccount Account(const Identifier& accountID) const override;
    OTIdentifier AccountPartialMatch(const std::string& hint) const override;
    ExclusiveAccount CreateAccount(
        const identifier::Nym& ownerNymID,
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        const opentxs::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash) const override;
    bool DeleteAccount(const Identifier& accountID) const override;
    SharedAccount IssuerAccount(const Identifier& unitID) const override;
    ExclusiveAccount mutable_Account(
        const Identifier& accountID,
        const AccountCallback callback) const override;
    bool UpdateAccount(
        const Identifier& accountID,
        const opentxs::ServerContext& context,
        const String& serialized) const override;
    bool UpdateAccount(
        const Identifier& accountID,
        const opentxs::ServerContext& context,
        const String& serialized,
        const std::string& label) const override;
    bool ImportAccount(
        std::unique_ptr<opentxs::Account>& imported) const override;
    std::shared_ptr<const opentxs::ClientContext> ClientContext(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const override;
    std::shared_ptr<const opentxs::ServerContext> ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID) const override;
    Editor<opentxs::ClientContext> mutable_ClientContext(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const override;
    Editor<opentxs::ServerContext> mutable_ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID) const override;
    std::set<OTIdentifier> IssuerList(
        const identifier::Nym& nymID) const override;
    std::shared_ptr<const api::client::Issuer> Issuer(
        const identifier::Nym& nymID,
        const Identifier& issuerID) const override;
    Editor<api::client::Issuer> mutable_Issuer(
        const identifier::Nym& nymID,
        const Identifier& issuerID) const override;
    bool IsLocalNym(const std::string& id) const override;
    std::size_t LocalNymCount() const override;
    std::set<OTNymID> LocalNyms() const override;
    ConstNym Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstNym Nym(const proto::CredentialIndex& nym) const override;
    ConstNym Nym(
        const NymParameters& nymParameters,
        const proto::ContactItemType type = proto::CITEMTYPE_ERROR,
        const std::string name = "") const override;
    NymData mutable_Nym(const identifier::Nym& id) const override;
    std::unique_ptr<const opentxs::NymFile> Nymfile(
        const identifier::Nym& id,
        const OTPasswordData& reason) const override;
    Editor<opentxs::NymFile> mutable_Nymfile(
        const identifier::Nym& id,
        const OTPasswordData& reason) const override;
    ConstNym NymByIDPartialMatch(const std::string& partialId) const override;
    ObjectList NymList() const override;
    bool NymNameByIndex(const std::size_t index, String& name) const override;
    std::shared_ptr<proto::PeerReply> PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box) const override;
    bool PeerReplyComplete(
        const identifier::Nym& nym,
        const Identifier& replyOrRequest) const override;
    bool PeerReplyCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const override;
    bool PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request,
        const Identifier& reply) const override;
    ObjectList PeerReplySent(const identifier::Nym& nym) const override;
    ObjectList PeerReplyIncoming(const identifier::Nym& nym) const override;
    ObjectList PeerReplyFinished(const identifier::Nym& nym) const override;
    ObjectList PeerReplyProcessed(const identifier::Nym& nym) const override;
    bool PeerReplyReceive(const identifier::Nym& nym, const PeerObject& reply)
        const override;
    std::shared_ptr<proto::PeerRequest> PeerRequest(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time) const override;
    bool PeerRequestComplete(
        const identifier::Nym& nym,
        const Identifier& reply) const override;
    bool PeerRequestCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request) const override;
    bool PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request) const override;
    bool PeerRequestDelete(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const override;
    ObjectList PeerRequestSent(const identifier::Nym& nym) const override;
    ObjectList PeerRequestIncoming(const identifier::Nym& nym) const override;
    ObjectList PeerRequestFinished(const identifier::Nym& nym) const override;
    ObjectList PeerRequestProcessed(const identifier::Nym& nym) const override;
    bool PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const override;
    bool PeerRequestUpdate(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const override;
#if OT_CASH
    std::unique_ptr<const blind::Purse> Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const bool checking) const override;
    Editor<blind::Purse> mutable_Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type) const override;
#endif
    bool RemoveServer(const Identifier& id) const override;
    bool RemoveUnitDefinition(const Identifier& id) const override;
    ConstServerContract Server(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstServerContract Server(
        const proto::ServerContract& contract) const override;
    ConstServerContract Server(
        const std::string& nymid,
        const std::string& name,
        const std::string& terms,
        const std::list<ServerContract::Endpoint>& endpoints,
        const std::uint32_t version) const override;
    ObjectList ServerList() const override;
    bool SetNymAlias(const Identifier& id, const std::string& alias)
        const override;
    bool SetServerAlias(const Identifier& id, const std::string& alias)
        const override;
    bool SetUnitDefinitionAlias(const Identifier& id, const std::string& alias)
        const override;
    ObjectList UnitDefinitionList() const override;
    const ConstUnitDefinition UnitDefinition(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstUnitDefinition UnitDefinition(
        const proto::UnitDefinition& contract) const override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t& power,
        const std::string& fraction) const override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms) const override;
    proto::ContactItemType CurrencyTypeBasedOnUnitType(
        const Identifier& contractID) const override;

    bool LoadCredential(
        const std::string& id,
        std::shared_ptr<proto::Credential>& credential) const override;
    bool SaveCredential(const proto::Credential& credential) const override;

    virtual ~Wallet() = default;

protected:
    using AccountLock =
        std::pair<std::shared_mutex, std::unique_ptr<opentxs::Account>>;
    using ContextID = std::pair<std::string, std::string>;
    using ContextMap =
        std::map<ContextID, std::shared_ptr<opentxs::internal::Context>>;

    const api::Core& api_;
    mutable ContextMap context_map_;
    mutable std::mutex context_map_lock_;

    std::shared_ptr<opentxs::Context> context(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const;
    proto::ContactItemType extract_unit(const Identifier& contractID) const;
    proto::ContactItemType extract_unit(
        const opentxs::UnitDefinition& contract) const;

    void save(opentxs::internal::Context* context) const;
    OTNymID server_to_nym(OTIdentifier& serverID) const;
    Wallet(const api::Core& core);

private:
    using AccountMap = std::map<OTIdentifier, AccountLock>;
    using NymLock = std::pair<std::mutex, std::shared_ptr<opentxs::Nym>>;
    using NymMap = std::map<std::string, NymLock>;
    using ServerMap =
        std::map<std::string, std::shared_ptr<opentxs::ServerContract>>;
    using UnitMap =
        std::map<std::string, std::shared_ptr<opentxs::UnitDefinition>>;
    using IssuerID = std::pair<OTIdentifier, OTIdentifier>;
    using IssuerLock =
        std::pair<std::mutex, std::shared_ptr<api::client::Issuer>>;
    using IssuerMap = std::map<IssuerID, IssuerLock>;
    using PurseID = std::tuple<OTNymID, OTServerID, OTUnitID>;

    friend opentxs::Factory;

    static const std::map<std::string, proto::ContactItemType> unit_of_account_;

    mutable AccountMap account_map_;
    mutable NymMap nym_map_;
    mutable ServerMap server_map_;
    mutable UnitMap unit_map_;
    mutable IssuerMap issuer_map_;
    mutable std::mutex account_map_lock_;
    mutable std::mutex nym_map_lock_;
    mutable std::mutex server_map_lock_;
    mutable std::mutex unit_map_lock_;
    mutable std::mutex issuer_map_lock_;
    mutable std::mutex peer_map_lock_;
    mutable std::map<std::string, std::mutex> peer_lock_;
    mutable std::mutex nymfile_map_lock_;
    mutable std::map<OTIdentifier, std::mutex> nymfile_lock_;
#if OT_CASH
    mutable std::mutex purse_lock_;
    mutable std::map<PurseID, std::mutex> purse_id_lock_;
#endif
    OTZMQPublishSocket account_publisher_;
    OTZMQPublishSocket issuer_publisher_;
    OTZMQPublishSocket nym_publisher_;
    OTZMQPublishSocket server_publisher_;
    OTZMQPublishSocket peer_reply_publisher_;
    OTZMQPublishSocket peer_request_publisher_;
    OTZMQRequestSocket dht_nym_requester_;
    OTZMQRequestSocket dht_server_requester_;
    OTZMQRequestSocket dht_unit_requester_;

    std::string account_alias(
        const std::string& accountID,
        const std::string& hint) const;
    opentxs::Account* account_factory(
        const Identifier& accountID,
        const std::string& alias,
        const std::string& serialized) const;
#if OT_CASH
    std::mutex& get_purse_lock(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit) const;
#endif
    virtual void instantiate_client_context(
        const proto::Context& serialized,
        const std::shared_ptr<const opentxs::Nym>& localNym,
        const std::shared_ptr<const opentxs::Nym>& remoteNym,
        std::shared_ptr<opentxs::internal::Context>& output) const
    {
    }
    virtual void instantiate_server_context(
        const proto::Context& serialized,
        const std::shared_ptr<const opentxs::Nym>& localNym,
        const std::shared_ptr<const opentxs::Nym>& remoteNym,
        std::shared_ptr<opentxs::internal::Context>& output) const
    {
    }
    virtual bool load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const
    {
        return false;
    }
    Editor<opentxs::NymFile> mutable_nymfile(
        const std::shared_ptr<const opentxs::Nym>& targetNym,
        const std::shared_ptr<const opentxs::Nym>& signerNym,
        const identifier::Nym& id,
        const OTPasswordData& reason) const;
    std::mutex& nymfile_lock(const identifier::Nym& nymID) const;
    std::mutex& peer_lock(const std::string& nymID) const;
    void publish_server(const Identifier& id) const;
#if OT_CASH
    std::unique_ptr<blind::Purse> purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const bool checking) const;
#endif
    void save(
        const std::string id,
        std::unique_ptr<opentxs::Account>& in,
        eLock& lock,
        bool success) const;
    void save(const Lock& lock, api::client::Issuer* in) const;
#if OT_CASH
    void save(const Lock& lock, const OTNymID nym, blind::Purse* in) const;
#endif
    void save(NymData* nymData, const Lock& lock) const;
    void save(opentxs::NymFile* nym, const Lock& lock) const;
    bool SaveCredentialIDs(const opentxs::Nym& nym) const;
    virtual std::shared_ptr<const opentxs::Nym> signer_nym(
        const identifier::Nym& id) const = 0;

    /* Throws std::out_of_range for missing accounts */
    AccountLock& account(
        const Lock& lock,
        const Identifier& accountID,
        const bool create) const;
    IssuerLock& issuer(
        const identifier::Nym& nymID,
        const Identifier& issuerID,
        const bool create) const;

    /**   Save an instantiated server contract to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated ServerContract object
     */
    ConstServerContract Server(std::unique_ptr<ServerContract>& contract) const;

    /**   Save an instantiated unit definition to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated UnitDefinition object
     */
    ConstUnitDefinition UnitDefinition(
        std::unique_ptr<opentxs::UnitDefinition>& contract) const;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace opentxs::api::implementation
