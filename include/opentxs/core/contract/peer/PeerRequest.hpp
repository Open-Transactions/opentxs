/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"

#include <string>

namespace opentxs
{

class PeerRequest : public Signable
{
private:
    typedef Signable ot_super;

    Identifier initiator_;
    Identifier recipient_;
    Identifier server_;
    Identifier cookie_;
    proto::PeerRequestType type_{proto::PEERREQUEST_ERROR};

    static std::unique_ptr<PeerRequest> Finish(
        std::unique_ptr<PeerRequest>& contract);
    static Identifier GetID(const proto::PeerRequest& contract);
    static bool FinalizeContract(PeerRequest& contract);

    proto::PeerRequest contract(const Lock& lock) const;
    Identifier GetID(const Lock& lock) const override;
    proto::PeerRequest SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock) override;

    PeerRequest() = delete;

protected:
    virtual proto::PeerRequest IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const override;

    PeerRequest(
        const ConstNym& nym,
        const proto::PeerRequest& serialized);
    PeerRequest(
        const ConstNym& nym,
        const proto::PeerRequest& serialized,
        const std::string& conditions);
    PeerRequest(
        const ConstNym& nym,
        const Identifier& recipient,
        const Identifier& serverID,
        const proto::PeerRequestType& type);
    PeerRequest(
        const ConstNym& nym,
        const Identifier& recipient,
        const Identifier& serverID,
        const std::string& conditions,
        const proto::PeerRequestType& type);

public:
    static std::unique_ptr<PeerRequest> Create(
        const ConstNym& nym,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Create(
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID,
        const Identifier& recipient,
        const std::string& txid);
    static std::unique_ptr<PeerRequest> Create(
        const ConstNym& nym,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID,
        const std::uint64_t& amount,
        const std::string& terms);
    static std::unique_ptr<PeerRequest> Create(
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const proto::ConnectionInfoType connectionType,
        const Identifier& recipient,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Create(
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const proto::SecretType secretType,
        const Identifier& recipient,
        const std::string& primary,
        const std::string& secondary,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Factory(
        const ConstNym& nym,
        const proto::PeerRequest& serialized);

    std::string Alias() const override { return Name(); }
    proto::PeerRequest Contract() const;
    const Identifier& Initiator() const { return initiator_; }
    std::string Name() const override;
    const Identifier& Recipient() const { return recipient_; }
    Data Serialize() const override;
    const proto::PeerRequestType& Type() const { return type_; }
    void SetAlias(const std::string&) override {}

    ~PeerRequest() = default;
};
} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP
