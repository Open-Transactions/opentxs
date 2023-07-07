// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/Base.hpp"  // IWYU pragma: associated

#include <Context.pb.h>
#include <Signature.pb.h>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NymFile.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/consensus/Client.hpp"
#include "otx/consensus/ClientPrivate.hpp"
#include "otx/consensus/Server.hpp"
#include "otx/consensus/ServerPrivate.hpp"

#ifndef OT_MAX_ACK_NUMS
#define OT_MAX_ACK_NUMS 100
#endif

namespace opentxs::otx::context::implementation
{
template <typename CRTP, typename DataType>
Base<CRTP, DataType>::Base(
    const api::Session& api,
    const VersionNumber targetVersion,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Signable(api, local, 0, {}, {}, calculate_id(api, local, remote), {})
    , server_id_(server)
    , remote_nym_(remote)
    , target_version_(targetVersion)
{
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
template <typename CRTP, typename DataType>
Base<CRTP, DataType>::Base(
    const api::Session& api,
    const VersionNumber targetVersion,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Signable(api, local, 0, {}, {}, calculate_id(api, local, remote), {})
    , server_id_(server)
    , remote_nym_(remote)
    , target_version_(targetVersion)
{
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::AcknowledgedNumbers() const
    -> UnallocatedSet<RequestNumber>
{
    return get_data()->acknowledged_request_numbers_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::add_acknowledged_number(
    Data& data,
    const RequestNumber req) -> bool
{
    auto& numbers = data.acknowledged_request_numbers_;
    auto output = numbers.insert(req);

    while (OT_MAX_ACK_NUMS < numbers.size()) { numbers.erase(numbers.begin()); }

    return output.second;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::AddAcknowledgedNumber(const RequestNumber req)
    -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    return add_acknowledged_number(data, req);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::AvailableNumbers() const -> std::size_t
{
    return get_data()->available_transaction_numbers_.size();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::calculate_id(
    const api::Session& api,
    const Nym_p& client,
    const Nym_p& server) noexcept(false) -> identifier_type
{
    if (!client) { throw std::runtime_error("Invalid client nym"); }

    if (!server) { throw std::runtime_error("Invalid notary nym"); }

    auto preimage = api.Factory().Data();
    preimage.Assign(client->ID());
    preimage += server->ID();

    return api.Factory().IdentifierFromPreimage(preimage.Bytes());
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::clear_signatures(Data& data) noexcept -> void
{
    data.sig_.reset();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::consume_available(
    Data& data,
    const TransactionNumber& number) -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")("Consuming number ")(
        number)
        .Flush();
    auto& available = data.available_transaction_numbers_;

    return 1 == available.erase(number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::consume_issued(
    Data& data,
    const TransactionNumber& number) -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")("Consuming number ")(
        number)
        .Flush();
    auto& available = data.available_transaction_numbers_;
    auto& issued = data.issued_transaction_numbers_;

    if (0 < available.count(number)) {
        LogDetail()(OT_PRETTY_CLASS())(
            "Consuming an issued number that was still available.")
            .Flush();
        available.erase(number);
    }

    return 1 == issued.erase(number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::ConsumeAvailable(const TransactionNumber& number)
    -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    return consume_available(data, number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::ConsumeIssued(const TransactionNumber& number)
    -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    return consume_issued(data, number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::contract(const Data& data) const -> proto::Context
{
    auto output = serialize(data);

    if (data.sig_) {
        output.mutable_signature()->CopyFrom(*data.sig_);
    } else {
        LogError()(OT_PRETTY_CLASS())("missing signature").Flush();
    }

    return output;
}

// This method will remove entries from acknowledged_request_numbers_ if they
// are not on the provided set
template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::finish_acknowledgements(
    Data& data,
    const UnallocatedSet<RequestNumber>& req) -> void
{
    auto toErase = UnallocatedSet<RequestNumber>{};
    auto& numbers = data.acknowledged_request_numbers_;

    for (const auto& number : numbers) {
        if (0 == req.count(number)) { toErase.insert(number); }
    }

    for (const auto& it : toErase) { numbers.erase(it); }
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::calculate_id() const -> identifier_type
{
    try {
        return calculate_id(api_, Signer(), remote_nym_);
    } catch (...) {
        return identifier_type{};
    }
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::HaveLocalNymboxHash() const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    return have_local_nymbox_hash(data);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::have_local_nymbox_hash(const Data& data) const
    -> bool
{
    return false == data.local_nymbox_hash_.empty();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::HaveRemoteNymboxHash() const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    return have_remote_nymbox_hash(data);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::have_remote_nymbox_hash(const Data& data) const
    -> bool
{
    return false == data.remote_nymbox_hash_.empty();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::IncrementRequest() -> RequestNumber
{
    auto handle = get_data();
    auto& data = *handle;

    return ++data.request_number_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::InitializeNymbox(const PasswordPrompt& reason)
    -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    const auto& ownerNymID = client_nym_id();
    auto nymbox{api_.Factory().InternalSession().Ledger(
        ownerNymID, server_nym_id(), server_id_)};

    if (false == bool(nymbox)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate nymbox for ")(
            ownerNymID, api_.Crypto())(".")
            .Flush();

        return false;
    }

    const auto generated = nymbox->GenerateLedger(
        ownerNymID, server_id_, ledgerType::nymbox, true);

    if (false == generated) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to generate nymbox "
            "for ")(ownerNymID, api_.Crypto())(".")
            .Flush();

        return false;
    }

    nymbox->ReleaseSignatures();

    OT_ASSERT(Signer());

    if (false == nymbox->SignContract(*Signer(), reason)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to sign nymbox for ")(ownerNymID, api_.Crypto())(".")
            .Flush();

        return false;
    }

    if (false == nymbox->SaveContract()) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to serialize nymbox "
            "for ")(ownerNymID, api_.Crypto())(".")
            .Flush();

        return false;
    }

    if (false == nymbox->SaveNymbox(data.local_nymbox_hash_)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Unable to save nymbox for ")(ownerNymID, api_.Crypto())
            .Flush();

        return false;
    }

    return true;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::insert_available_number(
    Data& data,
    const TransactionNumber& number) -> bool
{

    return data.available_transaction_numbers_.insert(number).second;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::insert_issued_number(
    Data& data,
    const TransactionNumber& number) -> bool
{

    return data.issued_transaction_numbers_.insert(number).second;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::issue_number(
    Data& data,
    const TransactionNumber& number) -> bool
{
    auto& issued = data.issued_transaction_numbers_;
    auto& available = data.available_transaction_numbers_;
    issued.insert(number);
    available.insert(number);
    const bool isIssued = issued.contains(number);
    const bool isAvailable = available.contains(number);
    const bool output = isIssued && isAvailable;

    if (!output) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Failed to issue number ")(number)(".")
            .Flush();
        issued.erase(number);
        available.erase(number);
    }

    return output;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::IssuedNumbers() const
    -> UnallocatedSet<TransactionNumber>
{
    return get_data()->issued_transaction_numbers_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::LegacyDataFolder() const -> UnallocatedCString
{
    return api_.DataFolder().string();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::LocalNymboxHash() const -> identifier::Generic
{
    return get_data()->local_nymbox_hash_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::mutable_Nymfile(const PasswordPrompt& reason)
    -> Editor<opentxs::NymFile>
{
    OT_ASSERT(Signer());

    return api_.Wallet().Internal().mutable_Nymfile(Signer()->ID(), reason);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::NymboxHashMatch() const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    if (false == have_local_nymbox_hash(data)) { return false; }

    if (false == have_remote_nymbox_hash(data)) { return false; }

    return (data.local_nymbox_hash_ == data.remote_nymbox_hash_);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Nymfile(const PasswordPrompt& reason) const
    -> std::unique_ptr<const opentxs::NymFile>
{
    OT_ASSERT(Signer());

    return api_.Wallet().Internal().Nymfile(Signer()->ID(), reason);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::recover_available_number(
    Data& data,
    const TransactionNumber& number) -> bool
{
    if (0 == number) { return false; }

    const bool issued = data.issued_transaction_numbers_.contains(number);

    if (false == issued) { return false; }

    return data.available_transaction_numbers_.insert(number).second;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::RecoverAvailableNumber(
    const TransactionNumber& number) -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    return recover_available_number(data, number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Refresh(
    proto::Context& out,
    const PasswordPrompt& reason) -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    update_signature(data, reason);
    out = contract(data);

    return true;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::RemoteNym() const -> const identity::Nym&
{
    OT_ASSERT(remote_nym_);

    return *remote_nym_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::RemoteNymboxHash() const -> identifier::Generic
{
    return get_data()->remote_nymbox_hash_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::remove_acknowledged_number(
    Data& data,
    const UnallocatedSet<RequestNumber>& req) -> bool
{
    auto removed = 0_uz;

    for (const auto& number : req) {
        removed += data.acknowledged_request_numbers_.erase(number);
    }

    return (0_uz < removed);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::RemoveAcknowledgedNumber(
    const UnallocatedSet<RequestNumber>& req) -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    return remove_acknowledged_number(data, req);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Request() const -> RequestNumber
{
    return get_data()->request_number_;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Reset() -> void
{
    reset(*get_data());
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::reset(Data& data) -> void
{
    data.available_transaction_numbers_.clear();
    data.issued_transaction_numbers_.clear();
    data.request_number_ = 0;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Save(const PasswordPrompt& reason) noexcept -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    return save(data, reason);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::save(Data& data, const PasswordPrompt& reason)
    -> bool
{
    if (false == update_signature(data, reason)) { return false; }
    if (false == validate(data)) { return false; }

    return api_.Storage().Store(contract(data));
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::serialize(
    const Data& data,
    const otx::ConsensusType type) const -> proto::Context
{
    auto output = proto::Context{};
    output.set_version(version(data));
    output.set_type(translate(type));

    if (Signer()) {
        output.set_localnym(Signer()->ID().asBase58(api_.Crypto()));
    }

    if (remote_nym_) {
        output.set_remotenym(remote_nym_->ID().asBase58(api_.Crypto()));
    }

    output.set_localnymboxhash(data.local_nymbox_hash_.asBase58(api_.Crypto()));
    output.set_remotenymboxhash(
        data.remote_nymbox_hash_.asBase58(api_.Crypto()));
    output.set_requestnumber(data.request_number_);

    for (const auto& it : data.acknowledged_request_numbers_) {
        output.add_acknowledgedrequestnumber(it);
    }

    for (const auto& it : data.available_transaction_numbers_) {
        output.add_availabletransactionnumber(it);
    }

    for (const auto& it : data.issued_transaction_numbers_) {
        output.add_issuedtransactionnumber(it);
    }

    return output;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Serialize(Writer&& out) const noexcept -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    return serialize(
        [&] {
            auto proto = proto::Context{};
            serialize(data, proto);

            return proto;
        }(),
        std::move(out));
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::serialize(const Data& data, proto::Context& out)
    const -> bool
{
    out = contract(data);

    return true;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::set_local_nymbox_hash(
    Data& data,
    const identifier::Generic& value) -> void
{
    auto& hash = data.local_nymbox_hash_;
    hash = value;
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")(
        "Set local nymbox hash to: ")
        .asHex(hash)
        .Flush();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::set_remote_nymbox_hash(
    Data& data,
    const identifier::Generic& value) -> void
{
    auto& hash = data.remote_nymbox_hash_;
    hash = value;
    LogVerbose()(OT_PRETTY_CLASS())("(")(type())(") ")(
        "Set remote nymbox hash to: ")
        .asHex(hash)
        .Flush();
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::SetLocalNymboxHash(const identifier::Generic& hash)
    -> void
{
    auto handle = get_data();
    auto& data = *handle;
    set_local_nymbox_hash(data, hash);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::SetRemoteNymboxHash(const identifier::Generic& hash)
    -> void
{
    auto handle = get_data();
    auto& data = *handle;
    set_remote_nymbox_hash(data, hash);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::SetRequest(const RequestNumber req) -> void
{
    get_data()->request_number_ = req;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::signatures() const noexcept
    -> std::span<const contract::Signature>
{
    OT_FAIL;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::signatures(const Data& data) const noexcept
    -> std::span<const contract::Signature>
{
    return {std::addressof(data.sig_), 1_uz};
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::sig_version(const Data& data) const -> proto::Context
{
    auto output = serialize(data, Type());
    output.clear_signature();

    return output;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::update_signature(const PasswordPrompt& reason)
    -> bool
{
    OT_FAIL;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::update_signature(
    Data& data,
    const PasswordPrompt& reason) -> bool
{
    update_version(data, target_version_);

    if (!Signable::update_signature(reason)) { return false; }

    auto success{false};
    auto serialized = sig_version(data);
    auto& signature = *serialized.mutable_signature();
    success = Signer()->Internal().Sign(
        serialized, crypto::SignatureRole::Context, signature, reason);

    if (success) {
        data.sig_ = std::make_shared<proto::Signature>(signature);
    } else {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Failed to create signature.")
            .Flush();
    }

    return success;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::update_version(
    Data& data,
    const VersionNumber version) noexcept -> void
{
    data.current_version_ = version;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Validate() const noexcept -> bool
{
    return validate(*get_data());
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::validate() const -> bool
{
    OT_FAIL;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::validate(const Data& data) const -> bool
{
    if (data.sig_) {
        return verify_signature(data, *data.sig_);
    } else {
        LogError()(OT_PRETTY_CLASS())("contract is not signed").Flush();

        return false;
    }
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::VerifyAcknowledgedNumber(
    const RequestNumber& req) const -> bool
{
    return verify_acknowledged_number(*get_data(), req);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::verify_acknowledged_number(
    const Data& data,
    const RequestNumber& req) const -> bool
{
    return data.acknowledged_request_numbers_.contains(req);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::VerifyAvailableNumber(
    const TransactionNumber& number) const -> bool
{
    return verify_available_number(*get_data(), number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::verify_available_number(
    const Data& data,
    const TransactionNumber& number) const -> bool
{
    return data.available_transaction_numbers_.contains(number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::VerifyIssuedNumber(
    const TransactionNumber& number) const -> bool
{
    return verify_issued_number(*get_data(), number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::verify_issued_number(
    const Data& data,
    const TransactionNumber& number) const -> bool
{
    return data.issued_transaction_numbers_.contains(number);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::verify_signature(
    const proto::Signature& signature) const -> bool
{
    OT_FAIL;
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::verify_signature(
    const Data& data,
    const proto::Signature& signature) const -> bool
{
    if (!Signable::verify_signature(signature)) {
        LogError()(OT_PRETTY_CLASS())("(")(type())(") ")(
            "Error: invalid signature.")
            .Flush();

        return false;
    }

    auto serialized = sig_version(data);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::Version() const noexcept -> VersionNumber
{
    return version(*get_data());
}

template <typename CRTP, typename DataType>
auto Base<CRTP, DataType>::version(const Data& data) const noexcept
    -> VersionNumber
{
    return data.current_version_;
}
}  // namespace opentxs::otx::context::implementation

namespace opentxs::otx::context::implementation
{
template class Base<ClientContext, ClientPrivate>;
template class Base<Server, ServerPrivate>;
}  // namespace opentxs::otx::context::implementation
