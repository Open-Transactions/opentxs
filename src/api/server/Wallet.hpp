// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::server::implementation
{
class Wallet final : public api::implementation::Wallet
{
public:
    std::shared_ptr<const opentxs::ClientContext> ClientContext(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const override;
    std::shared_ptr<const opentxs::Context> Context(
        const Identifier& notaryID,
        const identifier::Nym& clientNymID) const override;
    Editor<opentxs::ClientContext> mutable_ClientContext(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID) const override;
    Editor<opentxs::Context> mutable_Context(
        const Identifier& notaryID,
        const identifier::Nym& clientNymID) const override;

    ~Wallet() = default;

private:
    friend opentxs::Factory;

    using ot_super = api::implementation::Wallet;

    const api::server::Manager& server_;

    void instantiate_client_context(
        const proto::Context& serialized,
        const std::shared_ptr<const opentxs::Nym>& localNym,
        const std::shared_ptr<const opentxs::Nym>& remoteNym,
        std::shared_ptr<opentxs::internal::Context>& output) const override;
    bool load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const override;
    std::shared_ptr<const opentxs::Nym> signer_nym(
        const identifier::Nym& id) const override;

    Wallet(const api::server::Manager& server);
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace opentxs::api::server::implementation
