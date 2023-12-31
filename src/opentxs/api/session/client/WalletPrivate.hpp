// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/api/session/WalletPrivate.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace client
{
class WalletPrivate;  // IWYU pragma: keep
}  // namespace client

class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
namespace internal
{
class Base;
}  // namespace internal

class Base;
class Server;
}  // namespace context
}  // namespace otx

namespace protobuf
{
class Context;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::client::WalletPrivate final
    : public session::WalletPrivate
{
public:
    auto Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> final;
    auto mutable_Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> final;
    auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Server> final;
    auto ServerContext(
        const identifier::Nym& localNymID,
        const identifier::Generic& remoteID) const
        -> std::shared_ptr<const otx::context::Server> final;

    WalletPrivate(const api::session::Client& parent);
    WalletPrivate() = delete;
    WalletPrivate(const WalletPrivate&) = delete;
    WalletPrivate(WalletPrivate&&) = delete;
    auto operator=(const WalletPrivate&) -> WalletPrivate& = delete;
    auto operator=(WalletPrivate&&) -> WalletPrivate& = delete;

    ~WalletPrivate() final = default;

private:
    using ot_super = session::WalletPrivate;

    const api::session::Client& client_;
    OTZMQPublishSocket request_sent_;
    OTZMQPublishSocket reply_received_;

    void instantiate_server_context(
        const protobuf::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const final;
    void nym_to_contact(
        const identity::Nym& nym,
        const UnallocatedCString& name) const noexcept final;
    auto signer_nym(const identifier::Nym& id) const -> Nym_p final;
};
