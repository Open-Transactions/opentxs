// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "core/contract/peer/PeerReply.hpp"  // IWYU pragma: associated

#include <ctime>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/PeerReply.pb.h"
#include "serialization/protobuf/Signature.pb.h"

namespace opentxs::factory
{
auto PeerReply(const api::Session& api) noexcept
    -> std::unique_ptr<contract::peer::Reply>
{
    return std::make_unique<contract::peer::blank::Reply>(api);
}
}  // namespace opentxs::factory

namespace opentxs::contract::peer::implementation
{
Reply::Reply(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const identifier::Nym& initiator,
    const identifier::Notary& server,
    const PeerRequestType& type,
    const Identifier& request,
    const UnallocatedCString& conditions)
    : Signable(api, nym, version, conditions, "")
    , initiator_(initiator)
    , recipient_(nym->ID())
    , server_(server)
    , cookie_(Identifier::Factory(request))
    , type_(type)
{
}

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
          api.Factory().Identifier(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , initiator_(identifier::Nym::Factory(serialized.initiator()))
    , recipient_(identifier::Nym::Factory(serialized.recipient()))
    , server_(identifier::Notary::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(translate(serialized.type()))
{
}

Reply::Reply(const Reply& rhs) noexcept
    : Signable(rhs)
    , initiator_(rhs.initiator_)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , cookie_(rhs.cookie_)
    , type_(rhs.type_)
{
}

auto Reply::asAcknowledgement() const noexcept -> const reply::Acknowledgement&
{
    static const auto blank = peer::reply::blank::Acknowledgement{api_};

    return blank;
}

auto Reply::asBailment() const noexcept -> const reply::Bailment&
{
    static const auto blank = peer::reply::blank::Bailment{api_};

    return blank;
}

auto Reply::asConnection() const noexcept -> const reply::Connection&
{
    static const auto blank = peer::reply::blank::Connection{api_};

    return blank;
}

auto Reply::asOutbailment() const noexcept -> const reply::Outbailment&
{
    static const auto blank = peer::reply::blank::Outbailment{api_};

    return blank;
}

auto Reply::contract(const Lock& lock) const -> SerializedType
{
    auto contract = SigVersion(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

auto Reply::FinalizeContract(Reply& contract, const PasswordPrompt& reason)
    -> bool
{
    Lock lock(contract.lock_);

    if (!contract.update_signature(lock, reason)) { return false; }

    return contract.validate(lock);
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

auto Reply::GetID(const Lock& lock) const -> OTIdentifier
{
    return GetID(api_, IDVersion(lock));
}

auto Reply::GetID(const api::Session& api, const SerializedType& contract)
    -> OTIdentifier
{
    return api.Factory().InternalSession().Identifier(contract);
}

auto Reply::IDVersion(const Lock& lock) const -> SerializedType
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
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.set_server(String::Factory(server_)->Get());

    return contract;
}

auto Reply::LoadRequest(
    const api::Session& api,
    const Nym_p& nym,
    const Identifier& requestID,
    proto::PeerRequest& output) -> bool
{
    std::time_t notUsed = 0;

    auto loaded = api.Wallet().Internal().PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, notUsed, output);

    if (false == loaded) {
        loaded = api.Wallet().Internal().PeerRequest(
            nym->ID(),
            requestID,
            StorageBox::PROCESSEDPEERREQUEST,
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

auto Reply::Serialize() const noexcept -> OTData
{
    Lock lock(lock_);

    return api_.Factory().InternalSession().Data(contract(lock));
}

auto Reply::Serialize(SerializedType& output) const -> bool
{
    Lock lock(lock_);

    output = contract(lock);

    return true;
}

auto Reply::SigVersion(const Lock& lock) const -> SerializedType
{
    auto contract = IDVersion(lock);
    contract.set_id(String::Factory(id(lock))->Get());

    return contract;
}

auto Reply::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    if (!Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Sign(
        serialized, crypto::SignatureRole::PeerReply, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Reply::validate(const Lock& lock) const -> bool
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        LogError()(OT_PRETTY_CLASS())("Missing nym.").Flush();

        return false;
    }

    if (false == validNym) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();

        return false;
    }

    if (1 > signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (!validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();
    }

    return (validNym && validSyntax && validSig);
}

auto Reply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const -> bool
{
    if (!Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Verify(serialized, sigProto);
}
}  // namespace opentxs::contract::peer::implementation
