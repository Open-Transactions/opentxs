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
#include "opentxs/otx/Request.hpp"

#include "Request.hpp"

template class opentxs::Pimpl<opentxs::otx::Request>;

#define OTX_REQUEST_CREATE_VERSION 1
#define OTX_REQUEST_SIGNATURE_VERSION 3

#define OT_METHOD "opentxs::otx::implementation::Request::"

namespace opentxs::otx
{
OTXRequest Request::Factory(
    const std::shared_ptr<const opentxs::Nym> signer,
    const Identifier& server,
    const proto::ServerRequestType type)
{
    OT_ASSERT(signer);

    std::unique_ptr<implementation::Request> output{
        new implementation::Request(signer, signer->ID(), server, type)};

    OT_ASSERT(output);

    Lock lock(output->lock_);
    output->CalculateID(lock);
    output->update_signature(lock);

    OT_ASSERT(false == output->id(lock)->empty());

    return OTXRequest{output.release()};
}

OTXRequest Request::Factory(
    const api::Core& api,
    const proto::ServerRequest serialized)
{
    return OTXRequest{new implementation::Request(api, serialized)};
}
}  // namespace opentxs::otx

namespace opentxs::otx::implementation
{
Request::Request(
    const std::shared_ptr<const opentxs::Nym> signer,
    const Identifier& initiator,
    const Identifier& server,
    const proto::ServerRequestType type)
    : Signable(signer, OTX_REQUEST_CREATE_VERSION, "")
    , initiator_(initiator)
    , server_(server)
    , type_(type)
    , number_(0)
    , include_nym_(Flag::Factory(false))
{
}

Request::Request(const api::Core& api, const proto::ServerRequest serialized)
    : Signable(extract_nym(api, serialized), serialized.version(), "")
    , initiator_((nym_) ? nym_->ID() : Identifier::Factory().get())
    , server_(Identifier::Factory(serialized.server()))
    , type_(serialized.type())
    , number_(serialized.request())
    , include_nym_(Flag::Factory(false))
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

Request::Request(const Request& rhs)
    : Signable(rhs.nym_, rhs.version_, rhs.conditions_)
    , otx::Request()
    , initiator_(rhs.initiator_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , number_(rhs.number_)
    , include_nym_(Flag::Factory(rhs.include_nym_.get()))
{
}

proto::ServerRequest Request::Contract() const
{
    Lock lock(lock_);
    auto output = full_version(lock);

    return output;
}

std::shared_ptr<const opentxs::Nym> Request::extract_nym(
    const api::Core& api,
    const proto::ServerRequest serialized)
{
    if (serialized.has_credentials()) {

        return api.Wallet().Nym(serialized.credentials());
    } else if (false == serialized.nym().empty()) {

        return api.Wallet().Nym(Identifier::Factory(serialized.nym()));
    }

    return nullptr;
}

proto::ServerRequest Request::full_version(const Lock& lock) const
{
    auto contract = signature_version(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    if (include_nym_.get() && bool(nym_)) {
        *contract.mutable_credentials() = nym_->asPublicNym();
    }

    return contract;
}

OTIdentifier Request::GetID(const Lock& lock) const
{
    auto contract = id_version(lock);
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData(contract));

    return id;
}

proto::ServerRequest Request::id_version(const Lock& lock) const
{
    proto::ServerRequest output{};
    output.set_version(version_);
    output.clear_id();  // Must be blank
    output.set_type(type_);
    output.set_nym(initiator_->str());
    output.set_server(server_->str());
    output.set_request(number_);
    output.clear_signature();  // Must be blank

    return output;
}

RequestNumber Request::Number() const
{
    Lock lock(lock_);

    return number_;
}

OTData Request::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(full_version(lock));
}

bool Request::SetIncludeNym(const bool include)
{
    Lock lock(lock_);

    if (include) {
        include_nym_->On();
    } else {
        include_nym_->Off();
    }

    return update_signature(lock);
}

bool Request::SetNumber(const RequestNumber number)
{
    Lock lock(lock_);
    number_ = number;

    return update_signature(lock);
}

proto::ServerRequest Request::signature_version(const Lock& lock) const
{
    auto contract = id_version(lock);
    contract.set_id(id_->str());

    return contract;
}

bool Request::update_signature(const Lock& lock)
{
    if (false == Signable::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = signature_version(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_version(OTX_REQUEST_SIGNATURE_VERSION);
    signature.set_role(proto::SIGROLE_SERVERREQUEST);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool Request::validate(const Lock& lock) const
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

bool Request::verify_signature(
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
