// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <string_view>

#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace contract
{
namespace peer
{
class RequestPrivate;
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Claim;
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class PeerRequest;
}  // namespace protobuf

class Amount;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BailmentNoticeRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    std::string_view txid,
    const opentxs::Amount& amount,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto BailmentNoticeRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::ConnectionInfoType kind,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const opentxs::Amount& amount,
    std::string_view terms,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto StoreSecretRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& responder,
    const contract::peer::SecretType kind,
    std::span<const std::string_view> data,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto StoreSecretRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto VerificationRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identity::wot::Claim& claim,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
auto VerificationRequest(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerRequest& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::RequestPrivate*;
}  // namespace opentxs::factory
