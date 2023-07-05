// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/Base.hpp"  // IWYU pragma: associated

#include <PeerReply.pb.h>
#include <Signature.pb.h>
#include <compare>
#include <ctime>
#include <memory>
#include <span>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/contract/peer/Types.hpp"
#include "internal/core/contract/peer/reply/Base.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto operator<(const OTPeerReply& lhs, const OTPeerReply& rhs) noexcept -> bool
{
    return lhs->ID() > rhs->ID();
}
}  // namespace opentxs

namespace opentxs::contract::peer::reply::implementation
{
Reply::Reply(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const identifier::Nym& initiator,
    const identifier::Notary& server,
    const RequestType& type,
    const identifier::Generic& request,
    const UnallocatedCString& conditions)
    : Signable(api, nym, version, conditions, "")
    , initiator_(initiator)
    , recipient_(nym->ID())
    , server_(server)
    , cookie_(request)
    , type_(type)
{
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Reply::Reply(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized,
    const UnallocatedCString& conditions)
    : Signable(
          api,
          nym,
          serialized.version(),
          conditions,
          "",
          serialized.id(),
          api.Factory().IdentifierFromBase58(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , initiator_(api_.Factory().NymIDFromBase58(serialized.initiator()))
    , recipient_(api_.Factory().NymIDFromBase58(serialized.recipient()))
    , server_(api_.Factory().NotaryIDFromBase58(serialized.server()))
    , cookie_(api_.Factory().IdentifierFromBase58(serialized.cookie()))
    , type_(translate(serialized.type()))
{
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Reply::Reply(const Reply& rhs) noexcept
    : Signable(rhs)
    , initiator_(rhs.initiator_)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , cookie_(rhs.cookie_)
    , type_(rhs.type_)
{
}

auto Reply::Alias() const noexcept -> UnallocatedCString
{
    return UnallocatedCString{Name()};
}

auto Reply::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return CString{Name(), alloc.result_};
}

auto Reply::asAcknowledgement() const noexcept
    -> const internal::Acknowledgement&
{
    static const auto blank = peer::reply::blank::Acknowledgement{api_};

    return blank;
}

auto Reply::asBailment() const noexcept -> const internal::Bailment&
{
    static const auto blank = peer::reply::blank::Bailment{api_};

    return blank;
}

auto Reply::asConnection() const noexcept -> const internal::Connection&
{
    static const auto blank = peer::reply::blank::Connection{api_};

    return blank;
}

auto Reply::asFaucet() const noexcept -> const internal::Faucet&
{
    static const auto blank = peer::reply::blank::Faucet{api_};

    return blank;
}

auto Reply::asOutbailment() const noexcept -> const internal::Outbailment&
{
    static const auto blank = peer::reply::blank::Outbailment{api_};

    return blank;
}

auto Reply::calculate_id() const -> identifier_type
{
    return GetID(api_, IDVersion());
}

auto Reply::contract() const -> SerializedType
{
    auto contract = sig_version();

    if (const auto sigs = signatures(); false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    return contract;
}

auto Reply::FinalizeContract(Reply& contract, const PasswordPrompt& reason)
    -> bool
{
    if (!contract.update_signature(reason)) { return false; }

    return contract.validate();
}

auto Reply::Finish(Reply& contract, const PasswordPrompt& reason) -> bool
{
    if (FinalizeContract(contract, reason)) {

        return true;
    } else {
        LogError()(OT_PRETTY_STATIC(Reply))("Failed to finalize contract.")
            .Flush();

        return false;
    }
}

auto Reply::GetID(const api::Session& api, const SerializedType& contract)
    -> identifier_type
{
    return api.Factory().InternalSession().IdentifierFromPreimage(contract);
}

auto Reply::IDVersion() const -> SerializedType
{
    SerializedType contract;

    if (Version() < 2) {
        contract.set_version(2);
    } else {
        contract.set_version(Version());
    }

    contract.clear_id();  // reinforcing that this field must be blank.
    contract.set_initiator(initiator_.asBase58(api_.Crypto()));
    contract.set_recipient(recipient_.asBase58(api_.Crypto()));
    contract.set_type(translate(type_));
    contract.set_cookie(cookie_.asBase58(api_.Crypto()));
    contract.clear_signature();  // reinforcing that this field must be blank.

    if (false == server_.empty()) {
        contract.set_server(server_.asBase58(api_.Crypto()));
    }

    return contract;
}

auto Reply::LoadRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Generic& requestID,
    proto::PeerRequest& output) -> bool
{
    std::time_t notUsed = 0;

    auto loaded = api.Wallet().Internal().PeerRequest(
        nym->ID(),
        requestID,
        otx::client::StorageBox::INCOMINGPEERREQUEST,
        notUsed,
        output);

    if (false == loaded) {
        loaded = api.Wallet().Internal().PeerRequest(
            nym->ID(),
            requestID,
            otx::client::StorageBox::PROCESSEDPEERREQUEST,
            notUsed,
            output);

        if (loaded) {
            LogError()(OT_PRETTY_STATIC(Reply))(
                "Request has already been processed.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_STATIC(Reply))("Request does not exist.")
                .Flush();
        }
    }

    return true;
}

auto Reply::Serialize(Writer&& out) const noexcept -> bool
{
    return serialize(contract(), std::move(out));
}

auto Reply::Serialize(SerializedType& output) const -> bool
{
    output = contract();

    return true;
}

auto Reply::sig_version() const -> SerializedType
{
    auto contract = IDVersion();
    contract.set_id(ID().asBase58(api_.Crypto()));

    return contract;
}

auto Reply::update_signature(const PasswordPrompt& reason) -> bool
{
    if (!Signable::update_signature(reason)) { return false; }

    auto success = false;
    auto sigs = Signatures{};
    auto serialized = sig_version();
    auto& signature = *serialized.mutable_signature();
    success = Nym()->Internal().Sign(
        serialized, crypto::SignatureRole::PeerReply, signature, reason);

    if (success) {
        sigs.emplace_back(new proto::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Reply::validate() const -> bool
{
    auto validNym = false;

    if (Nym()) {
        validNym = Nym()->VerifyPseudonym();
    } else {
        LogError()(OT_PRETTY_CLASS())("Missing nym.").Flush();

        return false;
    }

    if (false == validNym) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();

        return false;
    }

    const auto validSyntax = proto::Validate(contract(), VERBOSE);

    if (!validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature.").Flush();

        return false;
    }

    auto validSig = false;
    const auto& signature = sigs.front();

    if (signature) { validSig = verify_signature(*signature); }

    if (!validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();
    }

    return (validNym && validSyntax && validSig);
}

auto Reply::verify_signature(const proto::Signature& signature) const -> bool
{
    if (!Signable::verify_signature(signature)) { return false; }

    auto serialized = sig_version();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Nym()->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::contract::peer::reply::implementation
