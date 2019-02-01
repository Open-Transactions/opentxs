// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/blind/Purse.hpp"
#endif
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include "Factory.hpp"

#include "PeerObject.hpp"

#define OT_METHOD "opentxs::peer::implementation::Object::"

namespace opentxs
{
opentxs::PeerObject* Factory::PeerObject(
    const api::Core& api,
    const ConstNym& senderNym,
    const std::string& message)
{
    std::unique_ptr<opentxs::PeerObject> output(
        new peer::implementation::Object(api, senderNym, message));

    if (!output->Validate()) { output.reset(); }

    return output.release();
}

opentxs::PeerObject* Factory::PeerObject(
    const api::Core& api,
    const ConstNym& senderNym,
    const std::string& payment,
    const bool isPayment)
{
    if (!isPayment) { return Factory::PeerObject(api, senderNym, payment); }

    std::unique_ptr<opentxs::PeerObject> output(
        new peer::implementation::Object(api, payment, senderNym));

    if (!output->Validate()) { output.reset(); }

    return output.release();
}

#if OT_CASH
opentxs::PeerObject* Factory::PeerObject(
    const api::Core& api,
    const ConstNym& senderNym,
    const std::shared_ptr<blind::Purse> purse)
{
    std::unique_ptr<opentxs::PeerObject> output(
        new peer::implementation::Object(api, senderNym, purse));

    if (!output->Validate()) { output.reset(); }

    return output.release();
}
#endif

opentxs::PeerObject* Factory::PeerObject(
    const api::Core& api,
    const std::shared_ptr<const PeerRequest> request,
    const std::shared_ptr<const PeerReply> reply,
    const std::uint32_t& version)
{
    std::unique_ptr<opentxs::PeerObject> output(
        new peer::implementation::Object(api, request, reply, version));

    if (!output->Validate()) { output.reset(); }

    return output.release();
}

opentxs::PeerObject* Factory::PeerObject(
    const api::Core& api,
    const std::shared_ptr<const PeerRequest> request,
    const std::uint32_t& version)
{
    std::unique_ptr<opentxs::PeerObject> output(
        new peer::implementation::Object(api, request, version));

    if (!output->Validate()) { output.reset(); }

    return output.release();
}

opentxs::PeerObject* Factory::PeerObject(
    const api::client::Contacts& contacts,
    const api::Core& api,
    const ConstNym& signerNym,
    const proto::PeerObject& serialized)
{
    const bool valid = proto::Validate(serialized, VERBOSE);
    std::unique_ptr<opentxs::PeerObject> output;

    if (valid) {
        output.reset(new peer::implementation::Object(
            contacts, api, signerNym, serialized));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid peer object.").Flush();
    }

    return output.release();
}

opentxs::PeerObject* Factory::PeerObject(
    const api::client::Contacts& contacts,
    const api::Core& api,
    const ConstNym& recipientNym,
    const Armored& encrypted)
{
    ConstNym notUsed{nullptr};
    std::unique_ptr<opentxs::PeerObject> output;
    OTEnvelope input;

    if (!input.SetCiphertext(encrypted)) { return output.release(); }

    auto contents = String::Factory();

    if (!input.Open(*recipientNym, contents)) { return output.release(); }

    auto serialized = proto::StringToProto<proto::PeerObject>(contents);

    return Factory::PeerObject(contacts, api, notUsed, serialized);
}
}  // namespace opentxs

namespace opentxs::peer::implementation
{
Object::Object(
    const api::Core& api,
    const ConstNym& nym,
    const std::string& message,
    const std::string& payment,
    const std::shared_ptr<const PeerReply> reply,
    const std::shared_ptr<const PeerRequest> request,
#if OT_CASH
    const std::shared_ptr<blind::Purse> purse,
#endif
    const proto::PeerObjectType type,
    const std::uint32_t& version)
    : api_(api)
    , nym_(nym)
    , message_(message.empty() ? nullptr : new std::string(message))
    , payment_(payment.empty() ? nullptr : new std::string(payment))
    , reply_(reply)
    , request_(request)
#if OT_CASH
    , purse_(purse)
#endif
    , type_(type)
    , version_(version)
{
}

Object::Object(
    const api::client::Contacts& contacts,
    const api::Core& api,
    const ConstNym& signerNym,
    const proto::PeerObject serialized)
    : Object(
          api,
          {},
          {},
          {},
          {},
          {},
#if OT_CASH
          {},
#endif
          serialized.type(),
          serialized.version())
{
    ConstNym objectNym{nullptr};

    if (serialized.has_nym()) {
        objectNym = api_.Wallet().Nym(serialized.nym());
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
            request_ = PeerRequest::Factory(
                api_.Wallet(), nym_, serialized.otrequest());
        } break;
        case (proto::PEEROBJECT_RESPONSE): {
            auto senderNym = api_.Wallet().Nym(
                identifier::Nym::Factory(serialized.otrequest().initiator()));
            request_ = PeerRequest::Factory(
                api_.Wallet(), senderNym, serialized.otrequest());

            if (false == bool(nym_)) {
                nym_ = api_.Wallet().Nym(identifier::Nym::Factory(
                    serialized.otrequest().recipient()));
            }

            reply_ =
                PeerReply::Factory(api_.Wallet(), nym_, serialized.otreply());
        } break;
        case (proto::PEEROBJECT_PAYMENT): {
            payment_.reset(new std::string(serialized.otpayment()));
        } break;
        case (proto::PEEROBJECT_CASH): {
#if OT_CASH
            purse_.reset(opentxs::Factory::Purse(api_, serialized.purse()));
#endif
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect type.").Flush();
        }
    }
}

Object::Object(
    const api::Core& api,
    const ConstNym& senderNym,
    const std::string& message)
    : Object(
          api,
          senderNym,
          message,
          {},
          {},
          {},
#if OT_CASH
          {},
#endif
          proto::PEEROBJECT_MESSAGE,
          PEER_MESSAGE_VERSION)
{
}

#if OT_CASH
Object::Object(
    const api::Core& api,
    const ConstNym& senderNym,
    const std::shared_ptr<blind::Purse> purse)
    : Object(
          api,
          senderNym,
          {},
          {},
          {},
          {},
          purse,
          proto::PEEROBJECT_CASH,
          PEER_CASH_VERSION)
{
}
#endif

Object::Object(
    const api::Core& api,
    const std::string& payment,
    const ConstNym& senderNym)
    : Object(
          api,
          senderNym,
          {},
          payment,
          {},
          {},
#if OT_CASH
          {},
#endif
          proto::PEEROBJECT_PAYMENT,
          PEER_PAYMENT_VERSION)
{
}

Object::Object(
    const api::Core& api,
    const std::shared_ptr<const PeerRequest> request,
    const std::shared_ptr<const PeerReply> reply,
    const std::uint32_t& version)
    : Object(
          api,
          {},
          {},
          {},
          reply,
          request,
#if OT_CASH
          {},
#endif
          proto::PEEROBJECT_RESPONSE,
          version)
{
}

Object::Object(
    const api::Core& api,
    const std::shared_ptr<const PeerRequest> request,
    const std::uint32_t& version)
    : Object(
          api,
          {},
          {},
          {},
          {},
          request,
#if OT_CASH
          {},
#endif
          proto::PEEROBJECT_REQUEST,
          version)
{
}

proto::PeerObject Object::Serialize() const
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
        } break;
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
        } break;
        case (proto::PEEROBJECT_REQUEST): {
            output.set_version(version_);

            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
                auto nym = api_.Wallet().Nym(
                    identifier::Nym::Factory(request_->Initiator().str()));

                if (nym) { *output.mutable_nym() = nym->asPublicNym(); }
            }
        } break;
        case (proto::PEEROBJECT_RESPONSE): {
            output.set_version(version_);

            if (reply_) { *(output.mutable_otreply()) = reply_->Contract(); }
            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
            }
        } break;
#if OT_CASH
        case (proto::PEEROBJECT_CASH): {
            if (PEER_CASH_VERSION > version_) {
                output.set_version(PEER_CASH_VERSION);
            } else {
                output.set_version(version_);
            }

            if (purse_) {
                if (nym_) { *output.mutable_nym() = nym_->asPublicNym(); }
                *output.mutable_purse() = purse_->Serialize();
            }
        } break;
#endif
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown type.").Flush();
        }
    }

    return output;
}

bool Object::Validate() const
{
    bool validChildren = false;

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE): {
            validChildren = bool(message_);
        } break;
        case (proto::PEEROBJECT_REQUEST): {
            if (request_) { validChildren = request_->Validate(); }
        } break;
        case (proto::PEEROBJECT_RESPONSE): {
            if (!reply_ || !request_) { break; }

            validChildren = reply_->Validate() && request_->Validate();
        } break;
        case (proto::PEEROBJECT_PAYMENT): {
            validChildren = bool(payment_);
        } break;
#if OT_CASH
        case (proto::PEEROBJECT_CASH): {
            validChildren = bool(purse_);
        } break;
#endif
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown type.").Flush();
        }
    }

    const bool validProto = proto::Validate(Serialize(), VERBOSE);

    return (validChildren && validProto);
}
}  // namespace opentxs::peer::implementation
