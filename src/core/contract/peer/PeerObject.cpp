// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::Armored
// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/PeerObject.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <PeerObject.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/core/contract/peer/PeerObject.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/PeerObject.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/peer/PeerObjectType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
constexpr auto PEER_MESSAGE_VERSION = 2;
constexpr auto PEER_PAYMENT_VERSION = 5;
constexpr auto PEER_CASH_VERSION = 7;
}  // namespace opentxs

namespace opentxs::factory
{
auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    const UnallocatedCString& message) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, senderNym, message));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    const UnallocatedCString& payment,
    const bool isPayment) noexcept -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        if (!isPayment) { return factory::PeerObject(api, senderNym, payment); }

        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, payment, senderNym));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const Nym_p& senderNym,
    otx::blind::Purse&& purse) noexcept -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, senderNym, std::move(purse)));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, request, reply, version));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const OTPeerRequest request,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, request, version));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::session::Client& api,
    const Nym_p& signerNym,
    const proto::PeerObject& serialized) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        const bool valid = proto::Validate(serialized, VERBOSE);
        std::unique_ptr<opentxs::PeerObject> output;

        if (valid) {
            output = std::make_unique<peer::implementation::Object>(
                api, signerNym, serialized);
        } else {
            throw std::runtime_error{"Invalid peer object"};
        }

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::session::Client& api,
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        auto notUsed = Nym_p{};
        auto output = std::unique_ptr<opentxs::PeerObject>{};
        auto input = api.Factory().InternalSession().Envelope(encrypted);
        auto contents = String::Factory();

        if (false ==
            input->Open(*recipientNym, contents->WriteInto(), reason)) {
            LogError()("opentxs::factory::")(__func__)(
                ": Unable to decrypt message")
                .Flush();

            return nullptr;
        }

        auto serialized =
            proto::StringToProto<proto::PeerObject>(api.Crypto(), contents);

        return factory::PeerObject(api, notUsed, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory

namespace opentxs::peer::implementation
{
Object::Object(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& message,
    const UnallocatedCString& payment,
    const OTPeerReply reply,
    const OTPeerRequest request,
    otx::blind::Purse&& purse,
    const contract::peer::PeerObjectType type,
    const VersionNumber version) noexcept
    : api_(api)
    , nym_(nym)
    , message_(message.empty() ? nullptr : new UnallocatedCString(message))
    , payment_(payment.empty() ? nullptr : new UnallocatedCString(payment))
    , reply_(reply)
    , request_(request)
    , purse_(std::move(purse))
    , type_(type)
    , version_(version)
{
}

Object::Object(
    const api::session::Client& api,
    const Nym_p& signerNym,
    const proto::PeerObject serialized) noexcept
    : Object(
          api,
          {},
          {},
          {},
          api.Factory().InternalSession().PeerReply(),
          api.Factory().InternalSession().PeerRequest(),
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
        case (contract::peer::PeerObjectType::Message): {
            message_ =
                std::make_unique<UnallocatedCString>(serialized.otmessage());
        } break;
        case (contract::peer::PeerObjectType::Request): {
            request_ = api_.Factory().InternalSession().PeerRequest(
                nym_, serialized.otrequest());
        } break;
        case (contract::peer::PeerObjectType::Response): {
            if (false == bool(nym_)) {
                nym_ = api_.Wallet().Nym(api_.Factory().NymIDFromBase58(
                    serialized.otrequest().recipient()));
            }

            auto senderNym = api_.Wallet().Nym(api_.Factory().NymIDFromBase58(
                serialized.otrequest().initiator()));
            request_ = api_.Factory().InternalSession().PeerRequest(
                senderNym, serialized.otrequest());
            reply_ = api_.Factory().InternalSession().PeerReply(
                nym_, serialized.otreply());
        } break;
        case (contract::peer::PeerObjectType::Payment): {
            payment_ =
                std::make_unique<UnallocatedCString>(serialized.otpayment());
        } break;
        case (contract::peer::PeerObjectType::Cash): {
            purse_ = factory::Purse(api_, serialized.purse());
        } break;
        case contract::peer::PeerObjectType::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Incorrect type.").Flush();
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
          api.Factory().InternalSession().PeerReply(),
          api.Factory().InternalSession().PeerRequest(),
          {},
          contract::peer::PeerObjectType::Message,
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
          api.Factory().InternalSession().PeerReply(),
          api.Factory().InternalSession().PeerRequest(),
          std::move(purse),
          contract::peer::PeerObjectType::Cash,
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
          api.Factory().InternalSession().PeerReply(),
          api.Factory().InternalSession().PeerRequest(),
          {},
          contract::peer::PeerObjectType::Payment,
          PEER_PAYMENT_VERSION)
{
}

Object::Object(
    const api::Session& api,
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) noexcept
    : Object(
          api,
          {},
          {},
          {},
          reply,
          request,
          {},
          contract::peer::PeerObjectType::Response,
          version)
{
}

Object::Object(
    const api::Session& api,
    const OTPeerRequest request,
    const VersionNumber version) noexcept
    : Object(
          api,
          {},
          {},
          {},
          api.Factory().InternalSession().PeerReply(),
          request,
          {},
          contract::peer::PeerObjectType::Request,
          version)
{
}

auto Object::Serialize(proto::PeerObject& output) const noexcept -> bool
{
    output.set_type(translate(type_));

    auto publicNym = [&](Nym_p nym) -> proto::Nym {
        auto data = proto::Nym{};
        if (false == nym->Internal().Serialize(data)) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize nym.").Flush();
        }
        return data;
    };

    switch (type_) {
        case (contract::peer::PeerObjectType::Message): {
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
        case (contract::peer::PeerObjectType::Payment): {
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
        case (contract::peer::PeerObjectType::Request): {
            output.set_version(version_);

            if (0 < request_->Version()) {
                if (false ==
                    request_->Serialize(*(output.mutable_otrequest()))) {
                    return false;
                }
                auto nym = api_.Wallet().Nym(request_->Initiator());

                if (nym) { *output.mutable_nym() = publicNym(nym); }
            }
        } break;
        case (contract::peer::PeerObjectType::Response): {
            output.set_version(version_);

            if (0 < reply_->Version()) {
                if (false == reply_->Serialize(*(output.mutable_otreply()))) {
                    return false;
                }
            }
            if (0 < request_->Version()) {
                if (false ==
                    request_->Serialize(*(output.mutable_otrequest()))) {
                    return false;
                }
            }
        } break;
        case (contract::peer::PeerObjectType::Cash): {
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
        case contract::peer::PeerObjectType::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown type.").Flush();
            return false;
        }
    }

    return true;
}

auto Object::Validate() const noexcept -> bool
{
    bool validChildren = false;

    switch (type_) {
        case (contract::peer::PeerObjectType::Message): {
            validChildren = bool(message_);
        } break;
        case (contract::peer::PeerObjectType::Request): {
            if (0 < request_->Version()) {
                validChildren = request_->Validate();
            }
        } break;
        case (contract::peer::PeerObjectType::Response): {
            if ((0 == reply_->Version()) || (0 == request_->Version())) {
                break;
            }

            validChildren = reply_->Validate() && request_->Validate();
        } break;
        case (contract::peer::PeerObjectType::Payment): {
            validChildren = bool(payment_);
        } break;
        case (contract::peer::PeerObjectType::Cash): {
            validChildren = purse_;
        } break;
        case contract::peer::PeerObjectType::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown type.").Flush();
        }
    }

    auto output = proto::PeerObject{};
    if (false == Serialize(output)) { return false; }

    const bool validProto = proto::Validate(output, VERBOSE);

    return (validChildren && validProto);
}

Object::~Object() = default;
}  // namespace opentxs::peer::implementation
