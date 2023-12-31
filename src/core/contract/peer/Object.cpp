// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::Armored
// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/Object.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/PeerObject.pb.h>
#include <opentxs/protobuf/PeerRequest.pb.h>
#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/core/contract/peer/Reply.hpp"
#include "internal/core/contract/peer/Request.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/contract/peer/Types.internal.hpp"
#include "opentxs/core/contract/peer/ObjectType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/protobuf/syntax/PeerObject.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
constexpr auto PEER_MESSAGE_VERSION = 2;
constexpr auto PEER_PAYMENT_VERSION = 5;
constexpr auto PEER_CASH_VERSION = 7;
}  // namespace opentxs

namespace opentxs::peer::implementation
{
Object::Object(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& message,
    const UnallocatedCString& payment,
    contract::peer::Reply reply,
    contract::peer::Request request,
    otx::blind::Purse&& purse,
    const contract::peer::ObjectType type,
    const VersionNumber version) noexcept
    : api_(api)
    , nym_(nym)
    , message_(message.empty() ? nullptr : new UnallocatedCString(message))
    , payment_(payment.empty() ? nullptr : new UnallocatedCString(payment))
    , reply_(std::move(reply))
    , request_(std::move(request))
    , purse_(std::move(purse))
    , type_(type)
    , version_(version)
{
}

Object::Object(
    const api::session::Client& api,
    const Nym_p& signerNym,
    const protobuf::PeerObject serialized) noexcept(false)
    : Object(
          api,
          {},
          {},
          {},
          {},
          {},
          {},
          translate(serialized.type()),
          serialized.version())
{
    Nym_p objectNym{nullptr};

    if (serialized.has_nym()) {
        objectNym = api_.Wallet().Internal().Nym(serialized.nym());
    }

    if (signerNym) {
        nym_ = signerNym;
    } else if (objectNym) {
        nym_ = objectNym;
    }

    switch (translate(serialized.type())) {
        case (contract::peer::ObjectType::Message): {
            message_ =
                std::make_unique<UnallocatedCString>(serialized.otmessage());
        } break;
        case (contract::peer::ObjectType::Request): {
            request_ = api_.Factory().Internal().Session().PeerRequest(
                serialized.otrequest());
        } break;
        case (contract::peer::ObjectType::Response): {
            if (false == bool(nym_)) {
                nym_ = api_.Wallet().Nym(api_.Factory().Internal().NymID(
                    serialized.otrequest().recipient()));
            }

            request_ = api_.Factory().Internal().Session().PeerRequest(
                serialized.otrequest());
            reply_ = api_.Factory().Internal().Session().PeerReply(
                serialized.otreply());
        } break;
        case (contract::peer::ObjectType::Payment): {
            payment_ =
                std::make_unique<UnallocatedCString>(serialized.otpayment());
        } break;
        case (contract::peer::ObjectType::Cash): {
            purse_ = factory::Purse(api_, serialized.purse());
        } break;
        case contract::peer::ObjectType::Error:
        default: {
            LogError()()("Incorrect type.").Flush();
        }
    }
}

Object::Object(
    const api::Session& api,
    const Nym_p& senderNym,
    const UnallocatedCString& message) noexcept
    : Object(
          api,
          senderNym,
          message,
          {},
          {},
          {},
          {},
          contract::peer::ObjectType::Message,
          PEER_MESSAGE_VERSION)
{
}

Object::Object(
    const api::Session& api,
    const Nym_p& senderNym,
    otx::blind::Purse&& purse) noexcept
    : Object(
          api,
          senderNym,
          {},
          {},
          {},
          {},
          std::move(purse),
          contract::peer::ObjectType::Cash,
          PEER_CASH_VERSION)
{
}

Object::Object(
    const api::Session& api,
    const UnallocatedCString& payment,
    const Nym_p& senderNym) noexcept
    : Object(
          api,
          senderNym,
          {},
          payment,
          {},
          {},
          {},
          contract::peer::ObjectType::Payment,
          PEER_PAYMENT_VERSION)
{
}

Object::Object(
    const api::Session& api,
    contract::peer::Request request,
    contract::peer::Reply reply,
    const VersionNumber version) noexcept
    : Object(
          api,
          {},
          {},
          {},
          std::move(reply),
          std::move(request),
          {},
          contract::peer::ObjectType::Response,
          version)
{
}

Object::Object(
    const api::Session& api,
    contract::peer::Request request,
    const VersionNumber version) noexcept
    : Object(
          api,
          {},
          {},
          {},
          {},
          std::move(request),
          {},
          contract::peer::ObjectType::Request,
          version)
{
}

auto Object::Serialize(protobuf::PeerObject& output) const noexcept -> bool
{
    output.set_type(translate(type_));

    auto publicNym = [&](Nym_p nym) -> protobuf::Nym {
        auto data = protobuf::Nym{};
        if (false == nym->Internal().Serialize(data)) {
            LogError()()("Failed to serialize nym.").Flush();
        }
        return data;
    };

    switch (type_) {
        case (contract::peer::ObjectType::Message): {
            if (PEER_MESSAGE_VERSION > version_) {
                output.set_version(PEER_MESSAGE_VERSION);
            } else {
                output.set_version(version_);
            }

            if (message_) {
                if (nym_) { *output.mutable_nym() = publicNym(nym_); }
                output.set_otmessage(String::Factory(*message_)->Get());
            }
        } break;
        case (contract::peer::ObjectType::Payment): {
            if (PEER_PAYMENT_VERSION > version_) {
                output.set_version(PEER_PAYMENT_VERSION);
            } else {
                output.set_version(version_);
            }

            if (payment_) {
                if (nym_) { *output.mutable_nym() = publicNym(nym_); }
                output.set_otpayment(String::Factory(*payment_)->Get());
            }
        } break;
        case (contract::peer::ObjectType::Request): {
            output.set_version(version_);

            if (request_.IsValid()) {
                if (false == request_.Internal().Serialize(
                                 *output.mutable_otrequest())) {
                    return false;
                }
                auto nym = api_.Wallet().Nym(request_.Initiator());

                if (nym) { *output.mutable_nym() = publicNym(nym); }
            }
        } break;
        case (contract::peer::ObjectType::Response): {
            output.set_version(version_);

            if (reply_.IsValid()) {
                if (false ==
                    reply_.Internal().Serialize(*output.mutable_otreply())) {
                    return false;
                }
            }
            if (request_.IsValid()) {
                if (false == request_.Internal().Serialize(
                                 *output.mutable_otrequest())) {
                    return false;
                }
            }
        } break;
        case (contract::peer::ObjectType::Cash): {
            if (PEER_CASH_VERSION > version_) {
                output.set_version(PEER_CASH_VERSION);
            } else {
                output.set_version(version_);
            }

            if (purse_) {
                if (nym_) { *output.mutable_nym() = publicNym(nym_); }

                purse_.Internal().Serialize(*output.mutable_purse());
            }
        } break;
        case contract::peer::ObjectType::Error:
        default: {
            LogError()()("Unknown type.").Flush();
            return false;
        }
    }

    return true;
}

auto Object::Validate() const noexcept -> bool
{
    auto validChildren = false;

    switch (type_) {
        case (contract::peer::ObjectType::Message): {
            validChildren = bool(message_);
        } break;
        case (contract::peer::ObjectType::Request): {
            if (request_.IsValid()) { validChildren = request_.Validate(); }
        } break;
        case (contract::peer::ObjectType::Response): {
            if ((false == reply_.IsValid()) || (false == request_.IsValid())) {
                break;
            }

            validChildren = reply_.Validate() && request_.Validate();
        } break;
        case (contract::peer::ObjectType::Payment): {
            validChildren = bool(payment_);
        } break;
        case (contract::peer::ObjectType::Cash): {
            validChildren = purse_;
        } break;
        case contract::peer::ObjectType::Error:
        default: {
            LogError()()("Unknown type.").Flush();
        }
    }

    auto output = protobuf::PeerObject{};
    if (false == Serialize(output)) { return false; }

    const bool validProto = protobuf::syntax::check(LogError(), output);

    return (validChildren && validProto);
}

Object::~Object() = default;
}  // namespace opentxs::peer::implementation
