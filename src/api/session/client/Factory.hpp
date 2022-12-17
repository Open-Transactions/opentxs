// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "api/session/Factory.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class PeerObject;
}  // namespace proto

class Armored;
class PasswordPrompt;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::client
{
class Factory final : public session::imp::Factory
{
public:
    auto PeerObject(const Nym_p& senderNym, const UnallocatedCString& message)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&&) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const OTPeerRequest request, const VersionNumber version)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& signerNym, const proto::PeerObject& serialized)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> final;

    Factory(const api::session::Client& parent);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    ~Factory() final = default;

private:
    const api::session::Client& client_;
};
}  // namespace opentxs::api::session::client
