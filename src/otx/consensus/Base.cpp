// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "otx/consensus/Base.hpp"  // IWYU pragma: associated

#include <Context.pb.h>
#include <Signature.pb.h>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NymFile.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/util/Log.hpp"

#ifndef OT_MAX_ACK_NUMS
#define OT_MAX_ACK_NUMS 100
#endif

namespace opentxs::otx::context::implementation
{
Base::Base(
    const api::Session& api,
    const VersionNumber targetVersion,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Signable(
          api,
          local,
          targetVersion,
          {},
          {},
          calculate_id(api, local, remote),
          {})
    , server_id_(server)
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(0)
    , acknowledged_request_numbers_()
    , local_nymbox_hash_()
    , remote_nymbox_hash_()
    , target_version_(targetVersion)
{
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Base::Base(
    const api::Session& api,
    const VersionNumber targetVersion,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Signable(
          api,
          local,
          targetVersion,
          {},
          {},
          calculate_id(api, local, remote),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , server_id_(server)
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(serialized.requestnumber())
    , acknowledged_request_numbers_()
    , local_nymbox_hash_(
          api_.Factory().IdentifierFromBase58(serialized.localnymboxhash()))
    , remote_nymbox_hash_(
          api_.Factory().IdentifierFromBase58(serialized.remotenymboxhash()))
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
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

auto Base::AcknowledgedNumbers() const -> UnallocatedSet<RequestNumber>
{
    auto lock = Lock{lock_};

    return acknowledged_request_numbers_;
}

auto Base::add_acknowledged_number(const Lock& lock, const RequestNumber req)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    auto output = acknowledged_request_numbers_.insert(req);

    while (OT_MAX_ACK_NUMS < acknowledged_request_numbers_.size()) {
        acknowledged_request_numbers_.erase(
            acknowledged_request_numbers_.begin());
    }

    return output.second;
}

auto Base::AddAcknowledgedNumber(const RequestNumber req) -> bool
{
    auto lock = Lock{lock_};

    return add_acknowledged_number(lock, req);
}

auto Base::AvailableNumbers() const -> std::size_t
{
    return available_transaction_numbers_.size();
}

auto Base::calculate_id(
    const api::Session& api,
    const Nym_p& client,
    const Nym_p& server) noexcept(false) -> identifier::Generic
{
    if (!client) { throw std::runtime_error("Invalid client nym"); }

    if (!server) { throw std::runtime_error("Invalid notary nym"); }

    auto preimage = api.Factory().Data();
    preimage.Assign(client->ID());
    preimage += server->ID();

    return api.Factory().IdentifierFromPreimage(preimage.Bytes());
}

auto Base::consume_available(const Lock& lock, const TransactionNumber& number)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")("Consuming number ")(
        number)
        .Flush();
    clear_signatures(lock);

    return 1 == available_transaction_numbers_.erase(number);
}

auto Base::consume_issued(const Lock& lock, const TransactionNumber& number)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")("Consuming number ")(
        number)
        .Flush();
    clear_signatures(lock);

    if (0 < available_transaction_numbers_.count(number)) {
        LogDetail()(OT_PRETTY_CLASS())(
            "Consuming an issued number that was still available.")
            .Flush();

        available_transaction_numbers_.erase(number);
    }

    return 1 == issued_transaction_numbers_.erase(number);
}

auto Base::ConsumeAvailable(const TransactionNumber& number) -> bool
{
    auto lock = Lock{lock_};

    return consume_available(lock, number);
}

auto Base::ConsumeIssued(const TransactionNumber& number) -> bool
{
    auto lock = Lock{lock_};

    return consume_issued(lock, number);
}

auto Base::contract(const Lock& lock) const -> proto::Context
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock);

    if (0 < signatures_.size()) {
        auto& sigProto = *output.mutable_signature();
        sigProto.CopyFrom(*signatures_.front());
    } else {
        LogError()(OT_PRETTY_CLASS())("warning: no signatures on context")
            .Flush();
    }

    return output;
}

// This method will remove entries from acknowledged_request_numbers_ if they
// are not on the provided set
auto Base::finish_acknowledgements(
    const Lock& lock,
    const UnallocatedSet<RequestNumber>& req) -> void
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    auto toErase = UnallocatedSet<RequestNumber>{};

    for (const auto& number : acknowledged_request_numbers_) {
        if (0 == req.count(number)) { toErase.insert(number); }
    }

    for (const auto& it : toErase) { acknowledged_request_numbers_.erase(it); }
}

auto Base::GetID(const Lock& lock) const -> identifier::Generic
{
    OT_ASSERT(verify_write_lock(lock));

    try {
        return calculate_id(api_, nym_, remote_nym_);
    } catch (...) {
        return identifier::Generic{};
    }
}

auto Base::HaveLocalNymboxHash() const -> bool
{
    return false == local_nymbox_hash_.empty();
}

auto Base::HaveRemoteNymboxHash() const -> bool
{
    return false == remote_nymbox_hash_.empty();
}

auto Base::IDVersion(const Lock& lock) const -> proto::Context
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = proto::Context{};
    output.set_version(version_);

    switch (Type()) {
        case otx::ConsensusType::Server: {
            if (nym_) {
                output.set_localnym(nym_->ID().asBase58(api_.Crypto()));
            }

            if (remote_nym_) {
                output.set_remotenym(remote_nym_->ID().asBase58(api_.Crypto()));
            }

            output.set_localnymboxhash(
                local_nymbox_hash_.asBase58(api_.Crypto()));
            output.set_remotenymboxhash(
                remote_nymbox_hash_.asBase58(api_.Crypto()));
        } break;
        case otx::ConsensusType::Client: {
            if (nym_) {
                output.set_remotenym(nym_->ID().asBase58(api_.Crypto()));
            }

            if (remote_nym_) {
                output.set_localnym(remote_nym_->ID().asBase58(api_.Crypto()));
            }

            output.set_remotenymboxhash(
                local_nymbox_hash_.asBase58(api_.Crypto()));
            output.set_localnymboxhash(
                remote_nymbox_hash_.asBase58(api_.Crypto()));
        } break;
        case otx::ConsensusType::Error:
        case otx::ConsensusType::Peer:
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

auto Base::IncrementRequest() -> RequestNumber
{
    auto lock = Lock{lock_};
    clear_signatures(lock);

    return ++request_number_;
}

auto Base::InitializeNymbox(const PasswordPrompt& reason) -> bool
{
    auto lock = Lock{lock_};
    const auto& ownerNymID = client_nym_id(lock);
    auto nymbox{api_.Factory().InternalSession().Ledger(
        ownerNymID, server_nym_id(lock), server_id_)};

    if (false == bool(nymbox)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate nymbox for ")(
            ownerNymID)(".")
            .Flush();

        return false;
    }

    const auto generated = nymbox->GenerateLedger(
        ownerNymID, server_id_, ledgerType::nymbox, true);

    if (false == generated) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to generate nymbox "
            "for ")(ownerNymID)(".")
            .Flush();

        return false;
    }

    nymbox->ReleaseSignatures();

    OT_ASSERT(nym_);

    if (false == nymbox->SignContract(*nym_, reason)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to sign nymbox for ")(ownerNymID)(".")
            .Flush();

        return false;
    }

    if (false == nymbox->SaveContract()) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to serialize nymbox "
            "for ")(ownerNymID)(".")
            .Flush();

        return false;
    }

    clear_signatures(lock);

    if (false == nymbox->SaveNymbox(local_nymbox_hash_)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to save nymbox for ")(ownerNymID)
            .Flush();

        return false;
    }

    return true;
}

auto Base::insert_available_number(const TransactionNumber& number) -> bool
{
    auto lock = Lock{lock_};
    clear_signatures(lock);

    return available_transaction_numbers_.insert(number).second;
}

auto Base::insert_issued_number(const TransactionNumber& number) -> bool
{
    auto lock = Lock{lock_};
    clear_signatures(lock);

    return issued_transaction_numbers_.insert(number).second;
}

auto Base::issue_number(const Lock& lock, const TransactionNumber& number)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    issued_transaction_numbers_.insert(number);
    available_transaction_numbers_.insert(number);
    const bool issued = (1 == issued_transaction_numbers_.count(number));
    const bool available = (1 == available_transaction_numbers_.count(number));
    const bool output = issued && available;

    if (!output) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Failed to issue number ")(number)(".")
            .Flush();
        issued_transaction_numbers_.erase(number);
        available_transaction_numbers_.erase(number);
    }

    return output;
}

auto Base::IssuedNumbers() const -> UnallocatedSet<TransactionNumber>
{
    auto lock = Lock{lock_};

    return issued_transaction_numbers_;
}

auto Base::LegacyDataFolder() const -> UnallocatedCString
{
    return api_.DataFolder().string();
}

auto Base::LocalNymboxHash() const -> identifier::Generic
{
    auto lock = Lock{lock_};

    return local_nymbox_hash_;
}

auto Base::mutable_Nymfile(const PasswordPrompt& reason)
    -> Editor<opentxs::NymFile>
{
    OT_ASSERT(nym_);

    return api_.Wallet().Internal().mutable_Nymfile(nym_->ID(), reason);
}

auto Base::Name() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return String::Factory(id(lock))->Get();
}

auto Base::NymboxHashMatch() const -> bool
{
    auto lock = Lock{lock_};

    if (!HaveLocalNymboxHash()) { return false; }

    if (!HaveRemoteNymboxHash()) { return false; }

    return (local_nymbox_hash_ == remote_nymbox_hash_);
}

auto Base::Nymfile(const PasswordPrompt& reason) const
    -> std::unique_ptr<const opentxs::NymFile>
{
    OT_ASSERT(nym_);

    return api_.Wallet().Internal().Nymfile(nym_->ID(), reason);
}

auto Base::recover_available_number(
    const Lock& lock,
    const TransactionNumber& number) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (0 == number) { return false; }

    const bool issued = 1 == issued_transaction_numbers_.count(number);

    if (!issued) { return false; }

    clear_signatures(lock);

    return available_transaction_numbers_.insert(number).second;
}

auto Base::RecoverAvailableNumber(const TransactionNumber& number) -> bool
{
    auto lock = Lock{lock_};

    return recover_available_number(lock, number);
}

auto Base::Refresh(proto::Context& out, const PasswordPrompt& reason) -> bool
{
    auto lock = Lock{lock_};
    update_signature(lock, reason);
    out = contract(lock);

    return true;
}

auto Base::RemoteNym() const -> const identity::Nym&
{
    OT_ASSERT(remote_nym_);

    return *remote_nym_;
}

auto Base::RemoteNymboxHash() const -> identifier::Generic
{
    auto lock = Lock{lock_};

    return remote_nymbox_hash_;
}

auto Base::remove_acknowledged_number(
    const Lock& lock,
    const UnallocatedSet<RequestNumber>& req) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    auto removed = 0_uz;

    for (const auto& number : req) {
        removed += acknowledged_request_numbers_.erase(number);
    }

    return (0 < removed);
}

auto Base::RemoveAcknowledgedNumber(const UnallocatedSet<RequestNumber>& req)
    -> bool
{
    auto lock = Lock{lock_};

    return remove_acknowledged_number(lock, req);
}

auto Base::Request() const -> RequestNumber { return request_number_.load(); }

auto Base::Reset() -> void
{
    auto lock = Lock{lock_};
    clear_signatures(lock);
    available_transaction_numbers_.clear();
    issued_transaction_numbers_.clear();
    request_number_.store(0);
}

auto Base::save(const Lock& lock, const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == UpdateSignature(lock, reason)) { return false; }
    if (false == ValidateContext(lock)) { return false; }

    return api_.Storage().Store(GetContract(lock));
}

auto Base::serialize(const Lock& lock, const otx::ConsensusType type) const
    -> proto::Context
{
    OT_ASSERT(verify_write_lock(lock));

    proto::Context output;
    output.set_version(version_);
    output.set_type(translate(type));

    if (nym_) { output.set_localnym(nym_->ID().asBase58(api_.Crypto())); }

    if (remote_nym_) {
        output.set_remotenym(remote_nym_->ID().asBase58(api_.Crypto()));
    }

    output.set_localnymboxhash(local_nymbox_hash_.asBase58(api_.Crypto()));
    output.set_remotenymboxhash(remote_nymbox_hash_.asBase58(api_.Crypto()));
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

auto Base::Serialize() const noexcept -> ByteArray
{
    return api_.Factory().InternalSession().Data([&] {
        auto proto = proto::Context{};
        Serialize(proto);

        return proto;
    }());
}

auto Base::Serialize(proto::Context& out) const -> bool
{
    auto lock = Lock{lock_};
    out = contract(lock);

    return true;
}

auto Base::set_local_nymbox_hash(
    const Lock& lock,
    const identifier::Generic& hash) -> void
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    local_nymbox_hash_ = hash;
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")(
        "Set local nymbox hash to: ")(local_nymbox_hash_.asHex())
        .Flush();
}

auto Base::set_remote_nymbox_hash(
    const Lock& lock,
    const identifier::Generic& hash) -> void
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    remote_nymbox_hash_ = hash;
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")(
        "Set remote nymbox hash to: ")(remote_nymbox_hash_.asHex())
        .Flush();
}

auto Base::SetLocalNymboxHash(const identifier::Generic& hash) -> void
{
    auto lock = Lock{lock_};
    set_local_nymbox_hash(lock, hash);
}

auto Base::SetRemoteNymboxHash(const identifier::Generic& hash) -> void
{
    auto lock = Lock{lock_};
    set_remote_nymbox_hash(lock, hash);
}

auto Base::SetRequest(const RequestNumber req) -> void
{
    auto lock = Lock{lock_};
    clear_signatures(lock);
    request_number_.store(req);
}

auto Base::SigVersion(const Lock& lock) const -> proto::Context
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    output.clear_signature();

    return output;
}

auto Base::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    clear_signatures(lock);
    update_version(lock, target_version_);

    if (!Signable::update_signature(lock, reason)) { return false; }

    auto success{false};
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Internal().Sign(
        serialized, crypto::SignatureRole::Context, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Failed to create signature.")
            .Flush();
    }

    return success;
}

auto Base::validate(const Lock& lock) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (1 != signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Error: This context is not signed.")
            .Flush();

        return false;
    }

    return verify_signature(lock, *signatures_.front());
}

auto Base::verify_acknowledged_number(
    const Lock& lock,
    const RequestNumber& req) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    return (0 < acknowledged_request_numbers_.count(req));
}

auto Base::verify_available_number(
    const Lock& lock,
    const TransactionNumber& number) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    return (0 < available_transaction_numbers_.count(number));
}

auto Base::verify_issued_number(
    const Lock& lock,
    const TransactionNumber& number) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    return (0 < issued_transaction_numbers_.count(number));
}

auto Base::verify_signature(const Lock& lock, const proto::Signature& signature)
    const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (!Signable::verify_signature(lock, signature)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Error: invalid signature.")
            .Flush();

        return false;
    }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Internal().Verify(serialized, sigProto);
}

auto Base::VerifyAcknowledgedNumber(const RequestNumber& req) const -> bool
{
    auto lock = Lock{lock_};

    return verify_acknowledged_number(lock, req);
}

auto Base::VerifyAvailableNumber(const TransactionNumber& number) const -> bool
{
    auto lock = Lock{lock_};

    return verify_available_number(lock, number);
}

auto Base::VerifyIssuedNumber(const TransactionNumber& number) const -> bool
{
    auto lock = Lock{lock_};

    return verify_issued_number(lock, number);
}
}  // namespace opentxs::otx::context::implementation
