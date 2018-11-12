// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/OT.hpp"

#define OT_METHOD "opentxs::PeerRequest::"

namespace opentxs
{
PeerRequest::PeerRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
    : ot_super(nym, serialized.version())
    , initiator_(Identifier::Factory(serialized.initiator()))
    , recipient_(Identifier::Factory(serialized.recipient()))
    , server_(Identifier::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(serialized.type())
    , wallet_{wallet}
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

PeerRequest::PeerRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequest& serialized,
    const std::string& conditions)
    : ot_super(nym, serialized.version(), conditions)
    , initiator_(Identifier::Factory(serialized.initiator()))
    , recipient_(Identifier::Factory(serialized.recipient()))
    , server_(Identifier::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(serialized.type())
    , wallet_{wallet}
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

PeerRequest::PeerRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::uint32_t version,
    const Identifier& recipient,
    const Identifier& server,
    const proto::PeerRequestType& type)
    : ot_super(nym, version)
    , initiator_(Identifier::Factory(nym->ID()))
    , recipient_(Identifier::Factory(recipient))
    , server_(Identifier::Factory(server))
    , cookie_(Identifier::Factory())
    , type_(type)
    , wallet_{wallet}
{
    auto random = OT::App().Crypto().AES().InstantiateBinarySecretSP();
    random->randomizeMemory(32);
    cookie_->CalculateDigest(
        Data::Factory(random->getMemory(), random->getMemorySize()));
}

PeerRequest::PeerRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::uint32_t version,
    const Identifier& recipient,
    const Identifier& server,
    const std::string& conditions,
    const proto::PeerRequestType& type)
    : ot_super(nym, version, conditions)
    , initiator_(Identifier::Factory(nym->ID()))
    , recipient_(Identifier::Factory(recipient))
    , server_(Identifier::Factory(server))
    , cookie_(Identifier::Factory())
    , type_(type)
    , wallet_{wallet}
{
    auto random = OT::App().Crypto().AES().InstantiateBinarySecretSP();
    random->randomizeMemory(32);
    cookie_->CalculateDigest(
        Data::Factory(random->getMemory(), random->getMemorySize()));
}

proto::PeerRequest PeerRequest::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

proto::PeerRequest PeerRequest::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::Wallet& wallet,
    const ConstNym& sender,
    const proto::PeerRequestType& type,
    const Identifier& unitID,
    const Identifier& serverID,
    const Identifier& recipient,
    const Identifier& requestID,
    const std::string& txid,
    const Amount& amount)
{
    auto unit = wallet.UnitDefinition(unitID);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_PENDINGBAILMENT): {
            contract.reset(new BailmentNotice(
                wallet,
                sender,
                recipient,
                unitID,
                serverID,
                requestID,
                txid,
                amount));
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& unitID,
    const Identifier& serverID)
{
    auto unit = wallet.UnitDefinition(unitID);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentRequest(
                wallet, nym, unit->Nym()->ID(), unitID, serverID));
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& unitID,
    const Identifier& serverID,
    const std::uint64_t& amount,
    const std::string& terms)
{
    auto unit = wallet.UnitDefinition(unitID);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentRequest(
                wallet,
                nym,
                unit->Nym()->ID(),
                unitID,
                serverID,
                amount,
                terms));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::Wallet& wallet,
    const ConstNym& sender,
    const proto::PeerRequestType& type,
    const proto::ConnectionInfoType connectionType,
    const Identifier& recipient,
    const Identifier& serverID)
{
    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionRequest(
                wallet, sender, recipient, connectionType, serverID));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::Wallet& wallet,
    const ConstNym& sender,
    const proto::PeerRequestType& type,
    const proto::SecretType secretType,
    const Identifier& recipient,
    const std::string& primary,
    const std::string& secondary,
    const Identifier& serverID)
{
    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new StoreSecret(
                wallet,
                sender,
                recipient,
                secretType,
                primary,
                secondary,
                serverID));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerRequest> PeerRequest::Factory(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
{
    if (!proto::Validate(serialized, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid protobuf.").Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentRequest(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentRequest(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_PENDINGBAILMENT): {
            contract.reset(new BailmentNotice(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionRequest(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new StoreSecret(wallet, nym, serialized));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    if (!contract) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return nullptr;
    }

    Lock lock(contract->lock_);

    if (!contract->validate(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request.").Flush();

        return nullptr;
    }

    const auto purportedID = Identifier::Factory(serialized.id());

    if (!contract->CalculateID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate ID.").Flush();

        return nullptr;
    }

    const auto& actualID = contract->id_;

    if (purportedID != actualID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid ID.").Flush();

        return nullptr;
    }

    return contract;
}

bool PeerRequest::FinalizeContract(PeerRequest& contract)
{
    Lock lock(contract.lock_);

    if (!contract.CalculateID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate ID.").Flush();

        return false;
    }

    if (!contract.update_signature(lock)) { return false; }

    return contract.validate(lock);
}

std::unique_ptr<PeerRequest> PeerRequest::Finish(
    std::unique_ptr<PeerRequest>& contract)
{
    std::unique_ptr<PeerRequest> output(contract.release());

    if (!output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return nullptr;
    }

    if (FinalizeContract(*output)) {

        return output;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to finalize contract.")
            .Flush();

        return nullptr;
    }
}

OTIdentifier PeerRequest::GetID(const Lock& lock) const
{
    return GetID(IDVersion(lock));
}

OTIdentifier PeerRequest::GetID(const proto::PeerRequest& contract)
{
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData(contract));
    return id;
}

proto::PeerRequest PeerRequest::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::PeerRequest contract;

    if (version_ < 2) {
        contract.set_version(2);
    } else {
        contract.set_version(version_);
    }

    contract.clear_id();  // reinforcing that this field must be blank.
    contract.set_initiator(String::Factory(initiator_)->Get());
    contract.set_recipient(String::Factory(recipient_)->Get());
    contract.set_type(type_);
    contract.set_cookie(String::Factory(cookie_)->Get());
    contract.set_server(String::Factory(server_)->Get());
    contract.clear_signature();  // reinforcing that this field must be blank.

    return contract;
}

std::string PeerRequest::Name() const { return String::Factory(id_)->Get(); }

OTData PeerRequest::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(contract(lock));
}

proto::PeerRequest PeerRequest::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String::Factory(id(lock))->Get());

    return contract;
}

bool PeerRequest::update_signature(const Lock& lock)
{
    if (!ot_super::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_PEERREQUEST);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool PeerRequest::validate(const Lock& lock) const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid syntax.").Flush();
    }

    if (1 > signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (!validSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature.").Flush();
    }

    return (validNym && validSyntax && validSig);
}

bool PeerRequest::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (!ot_super::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
    ;
}
}  // namespace opentxs
