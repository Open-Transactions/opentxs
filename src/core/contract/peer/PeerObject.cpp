// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/PeerObject.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#define OT_METHOD "opentxs::PeerObject::"

namespace opentxs
{
PeerObject::PeerObject(
    const api::client::Contacts& contacts,
    const api::Wallet& wallet,
    const ConstNym& signerNym,
    const proto::PeerObject serialized)
    : wallet_{wallet}
    , type_(serialized.type())
    , version_(serialized.version())
{
    ConstNym objectNym{nullptr};

    if (serialized.has_nym()) {
        objectNym = wallet_.Nym(serialized.nym());
        contacts.Update(serialized.nym());
    }

    if (signerNym) {
        nym_ = signerNym;
    } else if (objectNym) {
        nym_ = objectNym;
    }

    switch (serialized.type()) {
        case (proto::PEEROBJECT_MESSAGE): {
            message_.reset(new std::string(serialized.otmessage()));
        } break;
        case (proto::PEEROBJECT_REQUEST): {
            request_ =
                PeerRequest::Factory(wallet_, nym_, serialized.otrequest());
        } break;
        case (proto::PEEROBJECT_RESPONSE): {
            auto senderNym = wallet_.Nym(
                Identifier::Factory(serialized.otrequest().initiator()));
            request_ = PeerRequest::Factory(
                wallet_, senderNym, serialized.otrequest());

            if (false == bool(nym_)) {
                nym_ = wallet_.Nym(
                    Identifier::Factory(serialized.otrequest().recipient()));
            }

            reply_ = PeerReply::Factory(wallet_, nym_, serialized.otreply());
        } break;
        case (proto::PEEROBJECT_PAYMENT): {
            payment_.reset(new std::string(serialized.otpayment()));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect type.").Flush();
        }
    }
}

PeerObject::PeerObject(
    const api::Wallet& wallet,
    const ConstNym& senderNym,
    const std::string& message)
    : wallet_{wallet}
    , nym_(senderNym)
    , message_(new std::string{message})
    , type_(proto::PEEROBJECT_MESSAGE)
    , version_(PEER_MESSAGE_VERSION)
{
}

PeerObject::PeerObject(
    const api::Wallet& wallet,
    const std::string& payment,
    const ConstNym& senderNym)
    : wallet_{wallet}
    , nym_(senderNym)
    , payment_(new std::string{payment})
    , type_(proto::PEEROBJECT_PAYMENT)
    , version_(PEER_PAYMENT_VERSION)
{
}

PeerObject::PeerObject(
    const api::Wallet& wallet,
    const std::shared_ptr<PeerRequest>& request,
    const std::shared_ptr<PeerReply>& reply,
    const std::uint32_t& version)
    : wallet_{wallet}
    , reply_(reply)
    , request_(request)
    , type_(proto::PEEROBJECT_RESPONSE)
    , version_(version)
{
}

PeerObject::PeerObject(
    const api::Wallet& wallet,
    const std::shared_ptr<PeerRequest>& request,
    const std::uint32_t& version)
    : wallet_{wallet}
    , request_(request)
    , type_(proto::PEEROBJECT_REQUEST)
    , version_(version)
{
}

std::unique_ptr<PeerObject> PeerObject::Create(
    const api::Wallet& wallet,
    const ConstNym& senderNym,
    const std::string& message)
{
    std::unique_ptr<PeerObject> output(
        new PeerObject(wallet, senderNym, message));

    if (!output->Validate()) { output.reset(); }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Create(
    const api::Wallet& wallet,
    const ConstNym& senderNym,
    const std::string& payment,
    const bool isPayment)
{
    if (!isPayment) { return Create(wallet, senderNym, payment); }

    std::unique_ptr<PeerObject> output(
        new PeerObject(wallet, payment, senderNym));

    if (!output->Validate()) { output.reset(); }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Create(
    const api::Wallet& wallet,
    const std::shared_ptr<PeerRequest>& request,
    const std::shared_ptr<PeerReply>& reply,
    const std::uint32_t& version)
{
    std::unique_ptr<PeerObject> output(
        new PeerObject(wallet, request, reply, version));

    if (!output->Validate()) { output.reset(); }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Create(
    const api::Wallet& wallet,
    const std::shared_ptr<PeerRequest>& request,
    const std::uint32_t& version)
{
    std::unique_ptr<PeerObject> output(
        new PeerObject(wallet, request, version));

    if (!output->Validate()) { output.reset(); }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Factory(
    const api::client::Contacts& contacts,
    const api::Wallet& wallet,
    const ConstNym& signerNym,
    const proto::PeerObject& serialized)
{
    const bool valid = proto::Validate(serialized, VERBOSE);
    std::unique_ptr<PeerObject> output;

    if (valid) {
        output.reset(new PeerObject(contacts, wallet, signerNym, serialized));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid peer object.").Flush();
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Factory(
    const api::client::Contacts& contacts,
    const api::Wallet& wallet,
    const ConstNym& recipientNym,
    const Armored& encrypted)
{
    ConstNym notUsed{nullptr};
    std::unique_ptr<PeerObject> output;
    OTEnvelope input;

    if (!input.SetCiphertext(encrypted)) { return output; }

    auto contents = String::Factory();

    if (!input.Open(*recipientNym, contents)) { return output; }

    auto serialized = proto::StringToProto<proto::PeerObject>(contents);
    output = Factory(contacts, wallet, notUsed, serialized);

    return output;
}

proto::PeerObject PeerObject::Serialize() const
{
    proto::PeerObject output;

    output.set_type(type_);

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE): {
            if (PEER_MESSAGE_VERSION > version_) {
                output.set_version(PEER_MESSAGE_VERSION);
            } else {
                output.set_version(version_);
            }

            if (message_) {
                if (nym_) { *output.mutable_nym() = nym_->asPublicNym(); }
                output.set_otmessage(String::Factory(*message_)->Get());
            }
            break;
        }
        case (proto::PEEROBJECT_PAYMENT): {
            if (PEER_PAYMENT_VERSION > version_) {
                output.set_version(PEER_PAYMENT_VERSION);
            } else {
                output.set_version(version_);
            }

            if (payment_) {
                if (nym_) { *output.mutable_nym() = nym_->asPublicNym(); }
                output.set_otpayment(String::Factory(*payment_)->Get());
            }
            break;
        }
        case (proto::PEEROBJECT_REQUEST): {
            output.set_version(version_);

            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
                auto nym = wallet_.Nym(request_->Initiator());

                if (nym) { *output.mutable_nym() = nym->asPublicNym(); }
            }
            break;
        }
        case (proto::PEEROBJECT_RESPONSE): {
            output.set_version(version_);

            if (reply_) { *(output.mutable_otreply()) = reply_->Contract(); }
            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
            }
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown type.").Flush();
        }
    }

    return output;
}

bool PeerObject::Validate() const
{
    bool validChildren = false;

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE): {
            validChildren = bool(message_);
            break;
        }
        case (proto::PEEROBJECT_REQUEST): {
            if (request_) { validChildren = request_->Validate(); }
            break;
        }
        case (proto::PEEROBJECT_RESPONSE): {
            if (!reply_ || !request_) { break; }

            validChildren = reply_->Validate() && request_->Validate();
            break;
        }
        case (proto::PEEROBJECT_PAYMENT): {
            validChildren = bool(payment_);
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown type.").Flush();
        }
    }

    const bool validProto = proto::Validate(Serialize(), VERBOSE);

    return (validChildren && validProto);
}
}  // namespace opentxs
