// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/consensus/Context.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#ifndef OT_MAX_ACK_NUMS
#define OT_MAX_ACK_NUMS 100
#endif

#define SIGNATURE_VERSION 2

#define OT_METHOD "Context::"

namespace opentxs
{
Context::Context(
    const api::Core& api,
    const std::uint32_t targetVersion,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server)
    : ot_super(local, targetVersion)
    , api_(api)
    , server_id_(Identifier::Factory(server))
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(0)
    , acknowledged_request_numbers_()
    , local_nymbox_hash_(Identifier::Factory())
    , remote_nymbox_hash_(Identifier::Factory())
    , target_version_(targetVersion)
{
}

Context::Context(
    const api::Core& api,
    const std::uint32_t targetVersion,
    const proto::Context& serialized,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server)
    : ot_super(local, serialized.version())
    , api_(api)
    , server_id_(Identifier::Factory(server))
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(serialized.requestnumber())
    , acknowledged_request_numbers_()
    , local_nymbox_hash_(Identifier::Factory(serialized.localnymboxhash()))
    , remote_nymbox_hash_(Identifier::Factory(serialized.remotenymboxhash()))
    , target_version_(targetVersion)
{
    for (const auto& it : serialized.acknowledgedrequestnumber()) {
        acknowledged_request_numbers_.insert(it);
    }

    for (const auto& it : serialized.availabletransactionnumber()) {
        available_transaction_numbers_.insert(it);
    }

    for (const auto& it : serialized.issuedtransactionnumber()) {
        issued_transaction_numbers_.insert(it);
    }

    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

std::set<RequestNumber> Context::AcknowledgedNumbers() const
{
    Lock lock(lock_);

    return acknowledged_request_numbers_;
}

bool Context::add_acknowledged_number(const Lock& lock, const RequestNumber req)
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = acknowledged_request_numbers_.insert(req);

    while (OT_MAX_ACK_NUMS < acknowledged_request_numbers_.size()) {
        acknowledged_request_numbers_.erase(
            acknowledged_request_numbers_.begin());
    }

    return output.second;
}

bool Context::AddAcknowledgedNumber(const RequestNumber req)
{
    Lock lock(lock_);

    return add_acknowledged_number(lock, req);
}

std::size_t Context::AvailableNumbers() const
{
    return available_transaction_numbers_.size();
}

bool Context::ConsumeAvailable(const TransactionNumber& number)
{
    Lock lock(lock_);

    return 1 == available_transaction_numbers_.erase(number);
}

bool Context::ConsumeIssued(const TransactionNumber& number)
{
    Lock lock(lock_);

    if (0 < available_transaction_numbers_.count(number)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Consuming an issued number that was still available.")
            .Flush();

        available_transaction_numbers_.erase(number);
    }

    return 1 == issued_transaction_numbers_.erase(number);
}

proto::Context Context::contract(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock);

    if (0 < signatures_.size()) {
        auto& sigProto = *output.mutable_signature();
        sigProto.CopyFrom(*signatures_.front());
    }

    return output;
}

// This method will remove entries from acknowledged_request_numbers_ if they
// are not on the provided set
void Context::finish_acknowledgements(
    const Lock& lock,
    const std::set<RequestNumber>& req)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<RequestNumber> toErase;

    for (const auto& number : acknowledged_request_numbers_) {
        if (0 == req.count(number)) { toErase.insert(number); }
    }

    for (const auto& it : toErase) { acknowledged_request_numbers_.erase(it); }
}

OTIdentifier Context::GetID(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto contract = IDVersion(lock);
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData(contract));

    return id;
}

bool Context::HaveLocalNymboxHash() const
{
    return String::Factory(local_nymbox_hash_)->Exists();
}

bool Context::HaveRemoteNymboxHash() const
{
    return String::Factory(remote_nymbox_hash_)->Exists();
}

proto::Context Context::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::Context output;
    output.set_version(version_);

    switch (Type()) {
        case proto::CONSENSUSTYPE_SERVER: {
            if (nym_) {
                output.set_localnym(String::Factory(nym_->ID())->Get());
            }

            if (remote_nym_) {
                output.set_remotenym(String::Factory(remote_nym_->ID())->Get());
            }

            output.set_localnymboxhash(
                String::Factory(local_nymbox_hash_)->Get());
            output.set_remotenymboxhash(
                String::Factory(remote_nymbox_hash_)->Get());
        } break;
        case proto::CONSENSUSTYPE_CLIENT: {
            if (nym_) {
                output.set_remotenym(String::Factory(nym_->ID())->Get());
            }

            if (remote_nym_) {
                output.set_localnym(String::Factory(remote_nym_->ID())->Get());
            }

            output.set_remotenymboxhash(
                String::Factory(local_nymbox_hash_)->Get());
            output.set_localnymboxhash(
                String::Factory(remote_nymbox_hash_)->Get());
        } break;
        default: {
            OT_FAIL;
        }
    }

    output.set_requestnumber(request_number_.load());

    for (const auto& it : available_transaction_numbers_) {
        output.add_availabletransactionnumber(it);
    }

    for (const auto& it : issued_transaction_numbers_) {
        output.add_issuedtransactionnumber(it);
    }

    return output;
}

RequestNumber Context::IncrementRequest()
{
    Lock lock(lock_);

    return ++request_number_;
}

bool Context::InitializeNymbox()
{
    Lock lock(lock_);
    const auto& ownerNymID = client_nym_id(lock);
    auto nymbox{
        api_.Factory().Ledger(ownerNymID, server_nym_id(lock), server_id_)};

    if (false == bool(nymbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate nymbox for ")(ownerNymID)(".")
            .Flush();

        return false;
    }

    const auto generated = nymbox->GenerateLedger(
        ownerNymID, server_id_, ledgerType::nymbox, true);

    if (false == generated) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to generate nymbox for ")(
            ownerNymID)(".")
            .Flush();

        return false;
    }

    nymbox->ReleaseSignatures();

    OT_ASSERT(nym_)

    if (false == nymbox->SignContract(*nym_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to sign nymbox for ")(
            ownerNymID)(".")
            .Flush();

        return false;
    }

    if (false == nymbox->SaveContract()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to serialize nymbox for ")(
            ownerNymID)(".")
            .Flush();

        return false;
    }

    if (false == nymbox->SaveNymbox(local_nymbox_hash_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to save nymbox for ")(
            ownerNymID)
            .Flush();

        return false;
    }

    return true;
}

bool Context::insert_available_number(const TransactionNumber& number)
{
    Lock lock(lock_);

    return available_transaction_numbers_.insert(number).second;
}

bool Context::insert_issued_number(const TransactionNumber& number)
{
    Lock lock(lock_);

    return issued_transaction_numbers_.insert(number).second;
}

bool Context::issue_number(const Lock& lock, const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    issued_transaction_numbers_.insert(number);
    available_transaction_numbers_.insert(number);
    const bool issued = (1 == issued_transaction_numbers_.count(number));
    const bool available = (1 == available_transaction_numbers_.count(number));
    const bool output = issued && available;

    if (!output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to issue number ")(number)(
            ".")
            .Flush();
        issued_transaction_numbers_.erase(number);
        available_transaction_numbers_.erase(number);
    }

    return output;
}

std::set<TransactionNumber> Context::IssuedNumbers() const
{
    Lock lock(lock_);

    return issued_transaction_numbers_;
}

std::string Context::LegacyDataFolder() const { return api_.DataFolder(); }

OTIdentifier Context::LocalNymboxHash() const
{
    Lock lock(lock_);

    return local_nymbox_hash_;
}

Editor<class NymFile> Context::mutable_Nymfile(const OTPasswordData& reason)
{
    OT_ASSERT(nym_)

    return api_.Wallet().mutable_Nymfile(nym_->ID(), reason);
}

std::string Context::Name() const
{
    Lock lock(lock_);

    return String::Factory(id(lock))->Get();
}

bool Context::NymboxHashMatch() const
{
    Lock lock(lock_);

    if (!HaveLocalNymboxHash()) { return false; }

    if (!HaveRemoteNymboxHash()) { return false; }

    return (local_nymbox_hash_ == remote_nymbox_hash_);
}

std::unique_ptr<const opentxs::NymFile> Context::Nymfile(
    const OTPasswordData& reason) const
{
    OT_ASSERT(nym_);

    return api_.Wallet().Nymfile(nym_->ID(), reason);
}

bool Context::RecoverAvailableNumber(const TransactionNumber& number)
{
    if (0 == number) { return false; }

    Lock lock(lock_);

    const bool issued = 1 == issued_transaction_numbers_.count(number);

    if (!issued) { return false; }

    return available_transaction_numbers_.insert(number).second;
}

const class Nym& Context::RemoteNym() const
{
    OT_ASSERT(remote_nym_);

    return *remote_nym_;
}

OTIdentifier Context::RemoteNymboxHash() const
{
    Lock lock(lock_);

    return remote_nymbox_hash_;
}

bool Context::remove_acknowledged_number(
    const Lock& lock,
    const std::set<RequestNumber>& req)
{
    OT_ASSERT(verify_write_lock(lock));

    std::size_t removed = 0;

    for (const auto& number : req) {
        removed += acknowledged_request_numbers_.erase(number);
    }

    return (0 < removed);
}

bool Context::RemoveAcknowledgedNumber(const std::set<RequestNumber>& req)
{
    Lock lock(lock_);

    return remove_acknowledged_number(lock, req);
}

RequestNumber Context::Request() const { return request_number_.load(); }

void Context::Reset()
{
    Lock lock(lock_);
    available_transaction_numbers_.clear();
    issued_transaction_numbers_.clear();
    request_number_.store(0);
}

proto::Context Context::serialize(
    const Lock& lock,
    const proto::ConsensusType type) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::Context output;
    output.set_version(version_);
    output.set_type(type);

    if (nym_) { output.set_localnym(String::Factory(nym_->ID())->Get()); }

    if (remote_nym_) {
        output.set_remotenym(String::Factory(remote_nym_->ID())->Get());
    }

    output.set_localnymboxhash(String::Factory(local_nymbox_hash_)->Get());
    output.set_remotenymboxhash(String::Factory(remote_nymbox_hash_)->Get());
    output.set_requestnumber(request_number_.load());

    for (const auto& it : acknowledged_request_numbers_) {
        output.add_acknowledgedrequestnumber(it);
    }

    for (const auto& it : available_transaction_numbers_) {
        output.add_availabletransactionnumber(it);
    }

    for (const auto& it : issued_transaction_numbers_) {
        output.add_issuedtransactionnumber(it);
    }

    return output;
}

OTData Context::Serialize() const { return proto::ProtoAsData(Serialized()); }

proto::Context Context::Serialized() const
{
    Lock lock(lock_);

    return contract(lock);
}

const Identifier& Context::Server() const { return server_id_; }

void Context::SetLocalNymboxHash(const Identifier& hash)
{
    Lock lock(lock_);
    local_nymbox_hash_ = Identifier::Factory(hash);
    CalculateID(lock);
}

void Context::SetRemoteNymboxHash(const Identifier& hash)
{
    Lock lock(lock_);
    remote_nymbox_hash_ = Identifier::Factory(hash);
    CalculateID(lock);
}

void Context::SetRequest(const RequestNumber req)
{
    Lock lock(lock_);

    request_number_.store(req);
}

proto::Context Context::SigVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    output.clear_signature();

    return output;
}

bool Context::update_signature(const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    if (!ot_super::update_signature(lock)) { return false; }

    if (version_ < target_version_) { version_ = target_version_; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_version(SIGNATURE_VERSION);
    signature.set_role(proto::SIGROLE_CONTEXT);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool Context::validate(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (1 != signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: This context is not signed.")
            .Flush();

        return false;
    }

    return verify_signature(lock, *signatures_.front());
}

bool Context::VerifyAcknowledgedNumber(const RequestNumber& req) const
{
    Lock lock(lock_);

    return (0 < acknowledged_request_numbers_.count(req));
}

bool Context::VerifyAvailableNumber(const TransactionNumber& number) const
{
    Lock lock(lock_);

    return (0 < available_transaction_numbers_.count(number));
}

bool Context::VerifyIssuedNumber(const TransactionNumber& number) const
{
    Lock lock(lock_);

    return (0 < issued_transaction_numbers_.count(number));
}

bool Context::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (!ot_super::verify_signature(lock, signature)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: invalid signature.")
            .Flush();

        return false;
    }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
}
}  // namespace opentxs
