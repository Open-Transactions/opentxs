// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/WalletPrivate.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identity/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace notary
{
class WalletPrivate;  // IWYU pragma: keep
}  // namespace notary

class Notary;
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
}  // namespace otx

namespace proto
{
class Context;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::notary::WalletPrivate final
    : public session::WalletPrivate
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

    WalletPrivate(const api::session::Notary& parent);
    WalletPrivate() = delete;
    WalletPrivate(const WalletPrivate&) = delete;
    WalletPrivate(WalletPrivate&&) = delete;
    auto operator=(const WalletPrivate&) -> WalletPrivate& = delete;
    auto operator=(WalletPrivate&&) -> WalletPrivate& = delete;

    ~WalletPrivate() final = default;

private:
    using ot_super = session::WalletPrivate;

    const api::session::Notary& server_;

    void instantiate_client_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<otx::context::internal::Base>& output) const final;
    auto load_legacy_account(
        const identifier::Account& accountID,
        const eLock& lock,
        AccountLock& row) const -> bool final;
    auto signer_nym(const identifier::Nym& id) const -> Nym_p final;
};
