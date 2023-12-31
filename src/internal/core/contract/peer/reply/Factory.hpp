// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

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

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace blockchain

namespace contract
{
namespace peer
{
class ReplyPrivate;
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Generic;
class Notary;
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Verification;
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class PeerReply;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BailmentNoticeReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto BailmentNoticeReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto BailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    std::string_view terms,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto BailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto ConnectionReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    bool ack,
    std::string_view url,
    std::string_view login,
    std::string_view password,
    std::string_view key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto ConnectionReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const contract::peer::RequestType type,
    const bool& ack,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto OutbailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    std::string_view terms,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto OutbailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto StoreSecretReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto StoreSecretReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto VerificationReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const std::optional<identity::wot::Verification>& response,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
auto VerificationReply(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::PeerReply& serialized,
    alloc::Strategy alloc) noexcept -> contract::peer::ReplyPrivate*;
}  // namespace opentxs::factory
