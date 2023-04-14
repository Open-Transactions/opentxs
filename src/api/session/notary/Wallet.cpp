// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Wallet

#include "api/session/notary/Wallet.hpp"  // IWYU pragma: associated

#include <exception>
#include <functional>
#include <utility>

#include "internal/api/session/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto WalletAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>
{
    using ReturnType = api::session::notary::Wallet;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::notary
{
Wallet::Wallet(const api::session::Notary& parent)
    : ot_super(parent)
    , server_(parent)
{
}

auto Wallet::ClientContext(const identifier::Nym& remoteNymID) const
    -> std::shared_ptr<const otx::context::Client>
{
    const auto& serverNymID = server_.NymID();
    auto base = context(serverNymID, remoteNymID);
    auto output = std::dynamic_pointer_cast<const otx::context::Client>(base);

    return output;
}

auto Wallet::Context(
    [[maybe_unused]] const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID) const
    -> std::shared_ptr<const otx::context::Base>
{
    return context(server_.NymID(), clientNymID);
}

void Wallet::instantiate_client_context(
    const proto::Context& serialized,
    const Nym_p& localNym,
    const Nym_p& remoteNym,
    std::shared_ptr<otx::context::internal::Base>& output) const
{
    output.reset(factory::ClientContext(
        api_, serialized, localNym, remoteNym, server_.ID()));
}

auto Wallet::load_legacy_account(
    const identifier::Generic& accountID,
    const eLock& lock,
    Wallet::AccountLock& row) const -> bool
{
    // WTF clang? This is perfectly valid c++17. Fix your shit.
    // auto& [rowMutex, pAccount] = row;
    const auto& rowMutex = std::get<0>(row);
    auto& pAccount = std::get<1>(row);

    OT_ASSERT(CheckLock(lock, rowMutex));

    pAccount.reset(Account::LoadExistingAccount(api_, accountID, server_.ID()));

    if (false == bool(pAccount)) { return false; }

    const auto signerNym = Nym(server_.NymID());

    if (false == bool(signerNym)) {
        LogError()(OT_PRETTY_CLASS())("Unable to load signer nym.").Flush();

        return false;
    }

    if (false == pAccount->VerifySignature(*signerNym)) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();

        return false;
    }

    LogError()(OT_PRETTY_CLASS())("Legacy account ")(accountID)(" exists.")
        .Flush();

    auto serialized = String::Factory();
    auto saved = pAccount->SaveContractRaw(serialized);

    OT_ASSERT(saved);

    const auto& ownerID = pAccount->GetNymID();

    OT_ASSERT(false == ownerID.empty());

    const auto& unitID = pAccount->GetInstrumentDefinitionID();

    OT_ASSERT(false == unitID.empty());

    const auto contract = UnitDefinition(unitID);
    const auto& serverID = pAccount->GetPurportedNotaryID();

    OT_ASSERT(server_.ID() == serverID);

    saved = api_.Storage().Store(
        accountID.asBase58(api_.Crypto()),
        serialized->Get(),
        "",
        ownerID,
        server_.NymID(),
        contract->Nym()->ID(),
        serverID,
        unitID,
        extract_unit(unitID));

    OT_ASSERT(saved);

    return true;
}

auto Wallet::mutable_ClientContext(
    const identifier::Nym& remoteNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Client>
{
    const auto& serverID = server_.ID();
    const auto& serverNymID = server_.NymID();
    Lock lock(context_map_lock_);
    auto base = context(serverNymID, remoteNymID);
    std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    if (base) {
        OT_ASSERT(otx::ConsensusType::Client == base->Type());
    } else {
        // Obtain nyms.
        const auto local = Nym(serverNymID);

        OT_ASSERT_MSG(local, "Local nym does not exist in the wallet.");

        const auto remote = Nym(remoteNymID);

        OT_ASSERT_MSG(remote, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {
            serverNymID.asBase58(api_.Crypto()),
            remoteNymID.asBase58(api_.Crypto())};
        auto& entry = context_map_[contextID];
        entry.reset(factory::ClientContext(api_, local, remote, serverID));
        base = entry;
    }

    OT_ASSERT(base);

    auto* child = dynamic_cast<otx::context::Client*>(base.get());

    OT_ASSERT(nullptr != child);

    return {child, callback};
}

auto Wallet::mutable_Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Base>
{
    auto base = context(server_.NymID(), clientNymID);
    std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    OT_ASSERT(base);

    return {base.get(), callback};
}

auto Wallet::signer_nym(const identifier::Nym&) const -> Nym_p
{
    return Nym(server_.NymID());
}
}  // namespace opentxs::api::session::notary
