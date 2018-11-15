// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/otx/Reply.hpp"

#include "Reply.hpp"

template class opentxs::Pimpl<opentxs::otx::Reply>;

#define OTX_REPLY_CREATE_VERSION 1
#define OTX_REPLY_SIGNATURE_VERSION 3

#define OT_METHOD "opentxs::otx::implementation::Reply::"

namespace opentxs::otx
{
OTXReply Reply::Factory(
    const std::shared_ptr<const opentxs::Nym> signer,
    const Identifier& recipient,
    const Identifier& server,
    const proto::ServerReplyType type,
    const bool success)
{
    OT_ASSERT(signer);

    std::unique_ptr<implementation::Reply> output{
        new implementation::Reply(signer, recipient, server, type, success)};

    OT_ASSERT(output);

    Lock lock(output->lock_);
    output->CalculateID(lock);
    output->update_signature(lock);

    OT_ASSERT(false == output->id(lock)->empty());

    return OTXReply{output.release()};
}

OTXReply Reply::Factory(
    const api::Core& api,
    const proto::ServerReply serialized)
{
    return OTXReply{new implementation::Reply(api, serialized)};
}
}  // namespace opentxs::otx

namespace opentxs::otx::implementation
{
Reply::Reply(
    const std::shared_ptr<const opentxs::Nym> signer,
    const Identifier& recipient,
    const Identifier& server,
    const proto::ServerReplyType type,
    const bool success)
    : Signable(signer, OTX_REPLY_CREATE_VERSION, "")
    , recipient_(recipient)
    , server_(server)
    , type_(type)
    , success_(success)
    , number_(0)
    , payload_()
{
}

Reply::Reply(const api::Core& api, const proto::ServerReply serialized)
    : Signable(extract_nym(api, serialized), serialized.version(), "")
    , recipient_(Identifier::Factory(serialized.nym()))
    , server_(Identifier::Factory(serialized.server()))
    , type_(serialized.type())
    , success_(serialized.success())
    , number_(serialized.request())
    , payload_(std::make_shared<proto::OTXPush>(serialized.push()))
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

Reply::Reply(const Reply& rhs)
    : Signable(rhs.nym_, rhs.version_, rhs.conditions_)
    , otx::Reply()
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , success_(rhs.success_)
    , number_(rhs.number_)
    , payload_(rhs.payload_)
{
}

proto::ServerReply Reply::Contract() const
{
    Lock lock(lock_);
    auto output = full_version(lock);

    return output;
}

std::shared_ptr<const opentxs::Nym> Reply::extract_nym(
    const api::Core& api,
    const proto::ServerReply serialized)
{
    const auto serverID = Identifier::Factory(serialized.server());
    const auto server = api.Wallet().Server(serverID);

    if (false == bool(server)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid server id.").Flush();

        return nullptr;
    }

    return server->Nym();
}

proto::ServerReply Reply::full_version(const Lock& lock) const
{
    auto contract = signature_version(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

OTIdentifier Reply::GetID(const Lock& lock) const
{
    auto contract = id_version(lock);
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData(contract));

    return id;
}

proto::ServerReply Reply::id_version(const Lock& lock) const
{
    proto::ServerReply output{};
    output.set_version(version_);
    output.clear_id();  // Must be blank
    output.set_type(type_);
    output.set_nym(recipient_->str());
    output.set_server(server_->str());
    output.set_request(number_);
    output.set_success(success_);

    if (payload_) { *output.mutable_push() = *payload_; }

    output.clear_signature();  // Must be blank

    return output;
}

RequestNumber Reply::Number() const
{
    Lock lock(lock_);

    return number_;
}

std::shared_ptr<proto::OTXPush> Reply::Push() const
{
    Lock lock(lock_);

    return payload_;
}

OTData Reply::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(full_version(lock));
}

bool Reply::SetNumber(const RequestNumber number)
{
    Lock lock(lock_);
    number_ = number;

    return update_signature(lock);
}

bool Reply::SetPush(const proto::OTXPush& push)
{
    Lock lock(lock_);
    payload_ = std::make_shared<proto::OTXPush>(push);

    return update_signature(lock);
}

proto::ServerReply Reply::signature_version(const Lock& lock) const
{
    auto contract = id_version(lock);
    contract.set_id(id_->str());

    return contract;
}

bool Reply::update_signature(const Lock& lock)
{
    if (false == Signable::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = signature_version(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_version(OTX_REPLY_SIGNATURE_VERSION);
    signature.set_role(proto::SIGROLE_SERVERREPLY);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool Reply::validate(const Lock& lock) const
{
    bool validNym{false};

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    if (false == validNym) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(lock), VERBOSE);

    if (false == validSyntax) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid syntax.").Flush();

        return false;
    }

    if (1 != signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number signatures.")
            .Flush();

        return false;
    }

    bool validSig{false};
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (false == validSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature.").Flush();

        return false;
    }

    return true;
}

bool Reply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (false == Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = signature_version(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
}
}  // namespace opentxs::otx::implementation
