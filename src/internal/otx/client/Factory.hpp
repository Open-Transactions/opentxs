// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
class Wallet;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace otx
{
namespace client
{
class Issuer;
class Pair;
class ServerAction;
}  // namespace client
}  // namespace otx

namespace proto
{
class Issuer;
}  // namespace proto

class Flag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Issuer(
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> otx::client::Issuer*;
auto Issuer(
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> otx::client::Issuer*;
auto PairAPI(const Flag& running, const api::session::Client& client)
    -> otx::client::Pair*;
auto ServerAction(
    const api::session::Client& api,
    const ContextLockCallback& lockCallback)
    -> std::unique_ptr<otx::client::ServerAction>;
}  // namespace opentxs::factory
