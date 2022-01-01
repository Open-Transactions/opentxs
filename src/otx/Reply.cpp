// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "otx/Reply.hpp"   // IWYU pragma: associated

#include <list>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "core/contract/Signable.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/ServerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "serialization/protobuf/OTXPush.pb.h"
#include "serialization/protobuf/ServerReply.pb.h"
#include "serialization/protobuf/Signature.pb.h"

template class opentxs::Pimpl<opentxs::otx::Reply>;

namespace opentxs::otx
{
const VersionNumber Reply::DefaultVersion{1};
const VersionNumber Reply::MaxVersion{1};

static auto construct_push(OTXPushType pushtype, const std::string& payload)
    -> std::shared_ptr<proto::OTXPush>
{
    auto pPush = std::make_shared<proto::OTXPush>();
    auto& push = *pPush;
    push.set_version(1);
    push.set_type(translate(pushtype));
    push.set_item(payload);

    return pPush;
}

auto Reply::Factory(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    const PasswordPrompt& reason,
    std::shared_ptr<const proto::OTXPush>&& push) -> OTXReply
{
    OT_ASSERT(signer);

    std::unique_ptr<implementation::Reply> output{new implementation::Reply(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        std::move(push))};

    OT_ASSERT(output);

    Lock lock(output->lock_);
    output->update_signature(lock, reason);

    OT_ASSERT(false == output->id(lock)->empty());

    return OTXReply{output.release()};
}

auto Reply::Factory(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    const PasswordPrompt& reason,
    opentxs::otx::OTXPushType pushtype,
    const std::string& payload) -> OTXReply
{
    return Factory(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        reason,
        construct_push(pushtype, payload));
}

auto Reply::Factory(
    const api::Session& api,
    const proto::ServerReply serialized) -> OTXReply
{
    return OTXReply{new implementation::Reply(api, serialized)};
}

auto Reply::Factory(const api::Session& api, const ReadView& view) -> OTXReply
{
    return OTXReply{new implementation::Reply(
        api, proto::Factory<proto::ServerReply>(view))};
}
}  // namespace opentxs::otx

namespace opentxs::otx::implementation
{
Reply::Reply(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    std::shared_ptr<const proto::OTXPush>&& push)
    : Signable(api, signer, DefaultVersion, "", "")
    , recipient_(recipient)
    , server_(server)
    , type_(type)
    , success_(success)
    , number_(number)
    , payload_(std::move(push))
{
    Lock lock(lock_);
    first_time_init(lock);
}

Reply::Reply(const api::Session& api, const proto::ServerReply serialized)
    : Signable(
          api,
          extract_nym(api, serialized),
          serialized.version(),
          "",
          "",
          api.Factory().IdentifierFromBase58(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , recipient_(identifier::Nym::Factory(serialized.nym()))
    , server_(identifier::Server::Factory(serialized.server()))
    , type_(translate(serialized.type()))
    , success_(serialized.success())
    , number_(serialized.request())
    , payload_(
          serialized.has_push()
              ? std::make_shared<proto::OTXPush>(serialized.push())
              : std::shared_ptr<proto::OTXPush>{})
{
    Lock lock(lock_);
    init_serialized(lock);
}

Reply::Reply(const Reply& rhs)
    : Signable(rhs)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , success_(rhs.success_)
    , number_(rhs.number_)
    , payload_(rhs.payload_)
{
}

auto Reply::extract_nym(
    const api::Session& api,
    const proto::ServerReply serialized) -> Nym_p
{
    const auto serverID = identifier::Server::Factory(serialized.server());

    try {
        return api.Wallet().Server(serverID)->Nym();
    } catch (...) {
        LogError()(OT_PRETTY_STATIC(Reply))("Invalid server id.").Flush();

        return nullptr;
    }
}

auto Reply::full_version(const Lock& lock) const -> proto::ServerReply
{
    auto contract = signature_version(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

auto Reply::GetID(const Lock& lock) const -> OTIdentifier
{
    return api_.Factory().InternalSession().Identifier(id_version(lock));
}

auto Reply::id_version(const Lock& lock) const -> proto::ServerReply
{
    proto::ServerReply output{};
    output.set_version(version_);
    output.clear_id();  // Must be blank
    output.set_type(translate(type_));
    output.set_nym(recipient_->str());
    output.set_server(server_->str());
    output.set_request(number_);
    output.set_success(success_);

    if (payload_) { *output.mutable_push() = *payload_; }

    output.clear_signature();  // Must be blank

    return output;
}

auto Reply::Serialize() const noexcept -> OTData
{
    Lock lock(lock_);

    return api_.Factory().InternalSession().Data(full_version(lock));
}

auto Reply::Serialize(AllocateOutput destination) const -> bool
{
    Lock lock(lock_);

    auto serialized = proto::ServerReply{};
    if (false == serialize(lock, serialized)) { return false; }

    return write(serialized, destination);
}

auto Reply::serialize(const Lock& lock, proto::ServerReply& output) const
    -> bool
{
    output = full_version(lock);

    return true;
}

auto Reply::Serialize(proto::ServerReply& output) const -> bool
{
    Lock lock(lock_);

    return serialize(lock, output);
}

auto Reply::signature_version(const Lock& lock) const -> proto::ServerReply
{
    auto contract = id_version(lock);
    contract.set_id(id_->str());

    return contract;
}

auto Reply::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    if (false == Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = signature_version(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Sign(
        serialized, crypto::SignatureRole::ServerReply, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Reply::validate(const Lock& lock) const -> bool
{
    bool validNym{false};

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    if (false == validNym) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(lock), VERBOSE);

    if (false == validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();

        return false;
    }

    if (1 != signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number signatures.").Flush();

        return false;
    }

    bool validSig{false};
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (false == validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Reply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const -> bool
{
    if (false == Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = signature_version(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Verify(serialized, sigProto);
}
}  // namespace opentxs::otx::implementation
