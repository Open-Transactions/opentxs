// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/Factory.hpp"

#include <PeerObject.pb.h>
#include <stdexcept>
#include <utility>

#include "core/contract/peer/Object.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/peer/Object.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/PeerObject.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

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
        LogError()()(e.what()).Flush();

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
        LogError()()(e.what()).Flush();

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
        LogError()()(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const contract::peer::Request& request,
    const contract::peer::Reply& reply,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, request, reply, version));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return nullptr;
    }
}

auto PeerObject(
    const api::Session& api,
    const contract::peer::Request& request,
    const VersionNumber version) noexcept
    -> std::unique_ptr<opentxs::PeerObject>
{
    try {
        std::unique_ptr<opentxs::PeerObject> output(
            new peer::implementation::Object(api, request, version));

        if (!output->Validate()) { output.reset(); }

        return output;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

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
        LogError()()(e.what()).Flush();

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
            LogError()()("Unable to decrypt message").Flush();

            return nullptr;
        }

        auto serialized =
            proto::StringToProto<proto::PeerObject>(api.Crypto(), contents);

        return factory::PeerObject(api, notUsed, serialized);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory
