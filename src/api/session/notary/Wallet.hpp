// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "api/session/Wallet.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Notary;
class Wallet;
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace otx
{
namespace context
{
namespace internal
{
class Base;
}  // namespace internal

class Base;
class Client;
}  // namespace context

class Base;
class Client;
}  // namespace otx

namespace proto
{
class Context;
}  // namespace proto

class Factory;
class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::server
{
class Wallet final : public session::imp::Wallet
{
public:
    auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> final;
    auto Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> final;
    auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const
        -> Editor<otx::context::Client> final;
    auto mutable_Context(
        const identifier::Notary& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> final;

    Wallet(const api::session::Notary& parent);

    ~Wallet() final = default;

private:
    using ot_super = session::imp::Wallet;

    const api::session::Notary& server_;

    void instantiate_client_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const final;
    auto load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const -> bool final;
    auto signer_nym(const identifier::Nym& id) const -> Nym_p final;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::api::session::server
