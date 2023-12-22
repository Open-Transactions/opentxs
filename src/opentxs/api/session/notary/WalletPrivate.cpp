// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/notary/WalletPrivate.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::session::notary
{
WalletPrivate::WalletPrivate(const api::session::Notary& parent)
    : ot_super(parent)
    , server_(parent)
{
}

auto WalletPrivate::ClientContext(const identifier::Nym& remoteNymID) const
    -> std::shared_ptr<const otx::context::Client>
{
    auto handle = context_map_.lock();
    auto& map = *handle;
    const auto& serverNymID = server_.NymID();
    auto base = context(serverNymID, remoteNymID, map);
    auto output = std::dynamic_pointer_cast<const otx::context::Client>(base);

    return output;
}

auto WalletPrivate::Context(
    [[maybe_unused]] const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID) const
    -> std::shared_ptr<const otx::context::Base>
{
    auto handle = context_map_.lock();
    auto& map = *handle;

    return context(server_.NymID(), clientNymID, map);
}

void WalletPrivate::instantiate_client_context(
    const proto::Context& serialized,
    const Nym_p& localNym,
    const Nym_p& remoteNym,
    std::shared_ptr<otx::context::internal::Base>& output) const
{
    output.reset(factory::ClientContext(
        api_, serialized, localNym, remoteNym, server_.ID()));
}

auto WalletPrivate::load_legacy_account(
    const identifier::Account& accountID,
    const eLock& lock,
    WalletPrivate::AccountLock& row) const -> bool
{
    // WTF clang? This is perfectly valid c++17. Fix your shit.
    // auto& [rowMutex, pAccount] = row;
    const auto& rowMutex = std::get<0>(row);
    auto& pAccount = std::get<1>(row);

    assert_true(CheckLock(lock, rowMutex));

    pAccount.reset(Account::LoadExistingAccount(api_, accountID, server_.ID()));

    if (false == bool(pAccount)) { return false; }

    const auto signerNym = Nym(server_.NymID());

    if (false == bool(signerNym)) {
        LogError()()("Unable to load signer nym.").Flush();

        return false;
    }

    if (false == pAccount->VerifySignature(*signerNym)) {
        LogError()()("Invalid signature.").Flush();

        return false;
    }

    LogError()()("Legacy account ")(accountID, api_.Crypto())(" exists.")
        .Flush();

    auto serialized = String::Factory();
    auto saved = pAccount->SaveContractRaw(serialized);

    assert_true(saved);

    const auto& ownerID = pAccount->GetNymID();

    assert_false(ownerID.empty());

    const auto& unitID = pAccount->GetInstrumentDefinitionID();

    assert_false(unitID.empty());

    const auto contract = UnitDefinition(unitID);
    const auto& serverID = pAccount->GetPurportedNotaryID();

    assert_true(server_.ID() == serverID);

    saved = api_.Storage().Internal().Store(
        accountID,
        serialized->Get(),
        "",
        ownerID,
        server_.NymID(),
        contract->Signer()->ID(),
        serverID,
        unitID,
        extract_unit(unitID));

    assert_true(saved);

    return true;
}

auto WalletPrivate::mutable_ClientContext(
    const identifier::Nym& remoteNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Client>
{
    const auto& serverID = server_.ID();
    const auto& serverNymID = server_.NymID();
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto base = context(serverNymID, remoteNymID, map);
    const std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    if (base) {
        assert_true(otx::ConsensusType::Client == base->Type());
    } else {
        // Obtain nyms.
        const auto local = Nym(serverNymID);

        assert_false(
            nullptr == local, "Local nym does not exist in the wallet.");

        const auto remote = Nym(remoteNymID);

        assert_false(
            nullptr == remote, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {
            serverNymID.asBase58(api_.Crypto()),
            remoteNymID.asBase58(api_.Crypto())};
        auto& entry = map[contextID];
        entry.reset(factory::ClientContext(api_, local, remote, serverID));
        base = entry;
    }

    assert_false(nullptr == base);

    auto* child = dynamic_cast<otx::context::Client*>(base.get());

    assert_false(nullptr == child);

    return {child, callback};
}

auto WalletPrivate::mutable_Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Base>
{
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto base = context(server_.NymID(), clientNymID, map);
    const std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    assert_false(nullptr == base);

    return {base.get(), callback};
}

auto WalletPrivate::signer_nym(const identifier::Nym&) const -> Nym_p
{
    return Nym(server_.NymID());
}
}  // namespace opentxs::api::session::notary
