// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/PeerRequest.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <Signature.pb.h>
#include <compare>
#include <memory>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto PeerRequest(const api::Session& api) noexcept
    -> std::unique_ptr<contract::peer::Request>
{
    return std::make_unique<contract::peer::blank::Request>(api);
}
}  // namespace opentxs::factory

namespace opentxs::contract::peer
{
auto operator<(const OTPeerRequest& lhs, const OTPeerRequest& rhs) noexcept
    -> bool
{
    return lhs->ID() > rhs->ID();
}
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer::implementation
{
Request::Request(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const identifier::Nym& recipient,
    const identifier::Notary& server,
    const PeerRequestType& type,
    const UnallocatedCString& conditions)
    : Signable(api, nym, version, conditions, "")
    , initiator_(nym->ID())
    , recipient_(recipient)
    , server_(server)
    , cookie_(api_.Factory().IdentifierFromRandom())
    , type_(type)
{
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Request::Request(
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
          api.Factory().IdentifierFromBase58(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , initiator_(api.Factory().NymIDFromBase58(serialized.initiator()))
    , recipient_(api.Factory().NymIDFromBase58(serialized.recipient()))
    , server_(api.Factory().NotaryIDFromBase58(serialized.server()))
    , cookie_(api.Factory().IdentifierFromBase58(serialized.cookie()))
    , type_(translate(serialized.type()))
{
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Request::Request(const Request& rhs) noexcept
    : Signable(rhs)
    , initiator_(rhs.initiator_)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , cookie_(rhs.cookie_)
    , type_(rhs.type_)
{
}

auto Request::asBailment() const noexcept -> const request::Bailment&
{
    static const auto blank = peer::request::blank::Bailment{api_};

    return blank;
}

auto Request::asBailmentNotice() const noexcept
    -> const request::BailmentNotice&
{
    static const auto blank = peer::request::blank::BailmentNotice{api_};

    return blank;
}

auto Request::asConnection() const noexcept -> const request::Connection&
{
    static const auto blank = peer::request::blank::Connection{api_};

    return blank;
}

auto Request::asOutbailment() const noexcept -> const request::Outbailment&
{
    static const auto blank = peer::request::blank::Outbailment{api_};

    return blank;
}

auto Request::asStoreSecret() const noexcept -> const request::StoreSecret&
{
    static const auto blank = peer::request::blank::StoreSecret{api_};

    return blank;
}

auto Request::contract(const Lock& lock) const -> SerializedType
{
    auto contract = SigVersion(lock);
    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

auto Request::FinalizeContract(Request& contract, const PasswordPrompt& reason)
    -> bool
{
    Lock lock(contract.lock_);

    if (!contract.update_signature(lock, reason)) { return false; }

    return contract.validate(lock);
}

auto Request::Finish(Request& contract, const PasswordPrompt& reason) -> bool
{
    if (FinalizeContract(contract, reason)) {

        return true;
    } else {
        LogError()(OT_PRETTY_STATIC(Request))("Failed to finalize contract.")
            .Flush();

        return false;
    }
}

auto Request::GetID(const Lock& lock) const -> identifier::Generic
{
    return GetID(api_, IDVersion(lock));
}

auto Request::GetID(const api::Session& api, const SerializedType& contract)
    -> identifier::Generic
{
    return api.Factory().InternalSession().IdentifierFromPreimage(contract);
}

auto Request::IDVersion(const Lock& lock) const -> SerializedType
{
    OT_ASSERT(verify_write_lock(lock));

    SerializedType contract;

    if (version_ < 2) {
        contract.set_version(2);
    } else {
        contract.set_version(version_);
    }

    contract.clear_id();  // reinforcing that this field must be blank.
    contract.set_initiator(String::Factory(initiator_)->Get());
    contract.set_recipient(String::Factory(recipient_)->Get());
    contract.set_type(translate(type_));
    contract.set_cookie(String::Factory(cookie_)->Get());
    contract.set_server(String::Factory(server_)->Get());
    contract.clear_signature();  // reinforcing that this field must be blank.

    return contract;
}

auto Request::Serialize() const noexcept -> ByteArray
{
    Lock lock(lock_);

    return api_.Factory().Internal().Data(contract(lock));
}

auto Request::Serialize(SerializedType& output) const -> bool
{
    Lock lock(lock_);

    output = contract(lock);

    return true;
}

auto Request::SigVersion(const Lock& lock) const -> SerializedType
{
    auto contract = IDVersion(lock);
    contract.set_id(id(lock).asBase58(api_.Crypto()));

    return contract;
}

auto Request::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    if (!Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Internal().Sign(
        serialized, crypto::SignatureRole::PeerRequest, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Request::validate(const Lock& lock) const -> bool
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();
    }

    if (1 > signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    const auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (!validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();
    }

    return (validNym && validSyntax && validSig);
}

auto Request::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const -> bool
{
    if (!Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::contract::peer::implementation
