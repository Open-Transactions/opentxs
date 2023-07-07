// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

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
namespace request
{
namespace internal
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class StoreSecret;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

class Amount;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const opentxs::Amount& amount,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::BailmentNotice>;
auto BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::BailmentNotice>;
auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Bailment>;
auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Bailment>;
auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::ConnectionInfoType type,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Connection>;
auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Connection>;
auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Faucet>;
auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Faucet>;
auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const opentxs::Amount& amount,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Outbailment>;
auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Outbailment>;
auto StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const contract::peer::SecretType type,
    const UnallocatedCString& primary,
    const UnallocatedCString& secondary,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::StoreSecret>;
auto StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::StoreSecret>;
}  // namespace opentxs::factory
