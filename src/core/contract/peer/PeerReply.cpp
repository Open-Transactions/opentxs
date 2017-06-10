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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
PeerReply::PeerReply(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
        : ot_super(nym)
        , initiator_(serialized.initiator())
        , recipient_(serialized.recipient())
        , server_(serialized.server())
        , cookie_(serialized.cookie())
        , type_(serialized.type())
{
    id_ = Identifier(serialized.id());
    signatures_.push_front(
        SerializedSignature(
            std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
}

PeerReply::PeerReply(
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& server,
    const proto::PeerRequestType& type,
    const Identifier& request)
        : ot_super(nym, 2)
        , initiator_(initiator)
        , recipient_(nym->ID())
        , server_(server)
        , cookie_(request)
        , type_(type)
{
}

proto::PeerReply PeerReply::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

proto::PeerReply PeerReply::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& requestID,
    const Identifier& server,
    const std::string& terms)
{
    auto peerRequest = LoadRequest(nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(
                nym,
                Identifier(peerRequest->initiator()),
                requestID,
                server,
                terms));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(
                nym,
                Identifier(peerRequest->initiator()),
                requestID,
                server,
                terms));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const Identifier& requestID,
    const Identifier& server,
    const bool& ack)
{
    auto peerRequest = LoadRequest(nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_PENDINGBAILMENT) :
        case (proto::PEERREQUEST_STORESECRET) : {
            contract.reset(new NoticeAcknowledgement(
                nym,
                Identifier(peerRequest->initiator()),
                requestID,
                server,
                type,
                ack));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const Identifier& request,
    const Identifier& server,
    const bool& ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
{
    auto peerRequest = LoadRequest(nym, request);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_CONNECTIONINFO) : {
            contract.reset(new ConnectionReply(
                nym,
                Identifier(peerRequest->initiator()),
                request,
                server,
                ack,
                url,
                login,
                password,
                key));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Factory(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
{
    if (!proto::Check(serialized, 0, 0xFFFFFFFF)) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(nym, serialized));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(nym, serialized));
        } break;
        case (proto::PEERREQUEST_PENDINGBAILMENT) :
        case (proto::PEERREQUEST_STORESECRET) : {
            contract.reset(new NoticeAcknowledgement(nym, serialized));
        } break;
        case (proto::PEERREQUEST_CONNECTIONINFO) : {
            contract.reset(new ConnectionReply(nym, serialized));
        } break;
        default : {
            otErr << __FUNCTION__ << ": invalid reply type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate reply." << std::endl;

        return nullptr;
    }

    Lock lock(contract->lock_);

    if (!contract->validate(lock)) {
        otErr << __FUNCTION__ << ": invalid reply." << std::endl;

        return nullptr;
    }

    const Identifier purportedID(serialized.id());

    if (!contract->CalculateID(lock)) {
        otErr << __FUNCTION__ << ": failed to calculate ID." << std::endl;

        return nullptr;
    }

    const auto& actualID = contract->id_;

    if (purportedID != actualID) {
        otErr << __FUNCTION__ << ": invalid ID." << std::endl;

        return nullptr;
    }

    return contract;
}

bool PeerReply::FinalizeContract(PeerReply& contract)
{
    Lock lock(contract.lock_);

    if (!contract.CalculateID(lock)) { return false; }

    if (!contract.update_signature(lock)) { return false; }

    return contract.validate(lock);
}

std::unique_ptr<PeerReply> PeerReply::Finish(
    std::unique_ptr<PeerReply>& contract)
{
    std::unique_ptr<PeerReply> output(contract.release());

    if (!output) {
        otErr << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*output)) {

        return output;
    } else {
        otErr << __FUNCTION__ << ": failed to finalize contract." << std::endl;

        return nullptr;
    }
}

Identifier PeerReply::GetID(const Lock& lock) const
{
    return GetID(IDVersion(lock));
}

Identifier PeerReply::GetID(const proto::PeerReply& contract)
{
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData(contract));
    return id;
}

proto::PeerReply PeerReply::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::PeerReply contract;

    if (version_ < 2) {
        contract.set_version(2);
    } else {
        contract.set_version(version_);
    }

    contract.clear_id();         // reinforcing that this field must be blank.
    contract.set_initiator(String(initiator_).Get());
    contract.set_recipient(String(recipient_).Get());
    contract.set_type(type_);
    contract.set_cookie(String(cookie_).Get());
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.set_server(String(server_).Get());

    return contract;
}

std::shared_ptr<proto::PeerRequest> PeerReply::LoadRequest(
    const ConstNym& nym,
    const Identifier& requestID)
{
    std::shared_ptr<proto::PeerRequest> output;
    std::time_t notUsed = 0;

    output = OT::App().Contract().PeerRequest(
            nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, notUsed);

    if (!output) {
        output = OT::App().Contract().PeerRequest(
            nym->ID(), requestID, StorageBox::PROCESSEDPEERREQUEST, notUsed);

        if (output) {
            otErr << __FUNCTION__ << ": request has already been processed."
                  << std::endl;
        } else {
            otErr << __FUNCTION__ << ": request does not exist."
                << std::endl;
        }
    }

    return output;
}

std::string PeerReply::Name() const
{
    return String(id_).Get();
}

Data PeerReply::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(contract(lock));
}

proto::PeerReply PeerReply::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String(id(lock)).Get());

    return contract;
}

bool PeerReply::update_signature(const Lock& lock)
{
    if (!ot_super::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_PEERREPLY);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        otErr << __FUNCTION__ << ": failed to create signature."
                << std::endl;
    }

    return success;
}

bool PeerReply::validate(const Lock& lock) const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        otErr << __FUNCTION__ << ": invalid nym." << std::endl;
    }

    const bool validSyntax = proto::Check(contract(lock), version_, version_);

    if (!validSyntax) {
        otErr << __FUNCTION__ << ": invalid syntax." << std::endl;
    }

    if (1 > signatures_.size()) {
        otErr << __FUNCTION__ << ": Missing signature." << std::endl;

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) {
        validSig = verify_signature(lock, *signature);
    }

    if (!validSig) {
        otErr << __FUNCTION__ << ": invalid signature." << std::endl;
    }

    return (validNym && validSyntax && validSig);
}

bool PeerReply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (!ot_super::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);;
}
} // namespace opentxs
