// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

#include "api/Wallet.hpp"

#include "Wallet.hpp"

#define OT_METHOD "opentxs::api::server::implementation::Wallet::"

namespace opentxs
{
api::Wallet* Factory::Wallet(const api::server::Manager& server)
{
    return new api::server::implementation::Wallet(server);
}
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
Wallet::Wallet(const api::server::Manager& server)
    : ot_super(server)
    , server_(server)
{
}

std::shared_ptr<const opentxs::ClientContext> Wallet::ClientContext(
    const identifier::Nym&,  // Not used for now.
    const identifier::Nym& remoteNymID) const
{
    const auto& serverNymID = server_.NymID();
    auto base = context(serverNymID, remoteNymID);
    auto output = std::dynamic_pointer_cast<const opentxs::ClientContext>(base);

    return output;
}

std::shared_ptr<const opentxs::Context> Wallet::Context(
    [[maybe_unused]] const Identifier& notaryID,
    const identifier::Nym& clientNymID) const
{
    return context(server_.NymID(), clientNymID);
}

void Wallet::instantiate_client_context(
    const proto::Context& serialized,
    const std::shared_ptr<const opentxs::Nym>& localNym,
    const std::shared_ptr<const opentxs::Nym>& remoteNym,
    std::shared_ptr<opentxs::internal::Context>& output) const
{
    output.reset(opentxs::Factory::ClientContext(
        api_, serialized, localNym, remoteNym, server_.ID()));
}

bool Wallet::load_legacy_account(
    const Identifier& accountID,
    const eLock& lock,
    Wallet::AccountLock& row) const
{
    // WTF clang? This is perfectly valid c++17. Fix your shit.
    // auto& [rowMutex, pAccount] = row;
    const auto& rowMutex = std::get<0>(row);
    auto& pAccount = std::get<1>(row);

    OT_ASSERT(verify_lock(lock, rowMutex))

    pAccount.reset(Account::LoadExistingAccount(api_, accountID, server_.ID()));

    if (false == bool(pAccount)) { return false; }

    const auto signerNym = Nym(server_.NymID());

    if (false == bool(signerNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load signer nym.")
            .Flush();

        return false;
    }

    if (false == pAccount->VerifySignature(*signerNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature.").Flush();

        return false;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Legacy account ")(accountID.str())(
        " exists.")
        .Flush();

    auto serialized = String::Factory();
    auto saved = pAccount->SaveContractRaw(serialized);

    OT_ASSERT(saved)

    const auto& ownerID = pAccount->GetNymID();

    OT_ASSERT(false == ownerID.empty())

    const auto& unitID = pAccount->GetInstrumentDefinitionID();

    OT_ASSERT(false == unitID.empty())

    const auto contract = UnitDefinition(unitID);

    OT_ASSERT(contract)

    const auto& serverID = pAccount->GetPurportedNotaryID();

    OT_ASSERT(server_.ID() == serverID)

    saved = api_.Storage().Store(
        accountID.str(),
        serialized->Get(),
        "",
        ownerID,
        server_.NymID(),
        contract->Nym()->ID(),
        serverID,
        unitID,
        extract_unit(unitID));

    OT_ASSERT(saved)

    return true;
}

Editor<opentxs::ClientContext> Wallet::mutable_ClientContext(
    const identifier::Nym&,  // Not used for now.
    const identifier::Nym& remoteNymID) const
{
    const auto& serverID = server_.ID();
    const auto& serverNymID = server_.NymID();
    Lock lock(context_map_lock_);
    auto base = context(serverNymID, remoteNymID);
    std::function<void(opentxs::Context*)> callback =
        [&](opentxs::Context* in) -> void {
        this->save(dynamic_cast<opentxs::internal::Context*>(in));
    };

    if (base) {
        OT_ASSERT(proto::CONSENSUSTYPE_CLIENT == base->Type());
    } else {
        // Obtain nyms.
        const auto local = Nym(serverNymID);

        OT_ASSERT_MSG(local, "Local nym does not exist in the wallet.");

        const auto remote = Nym(remoteNymID);

        OT_ASSERT_MSG(remote, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {serverNymID.str(), remoteNymID.str()};
        auto& entry = context_map_[contextID];
        entry.reset(
            opentxs::Factory::ClientContext(api_, local, remote, serverID));
        base = entry;
    }

    OT_ASSERT(base);

    auto child = dynamic_cast<opentxs::ClientContext*>(base.get());

    OT_ASSERT(nullptr != child);

    return Editor<opentxs::ClientContext>(child, callback);
}

Editor<opentxs::Context> Wallet::mutable_Context(
    const Identifier& notaryID,
    const identifier::Nym& clientNymID) const
{
    auto base = context(server_.NymID(), clientNymID);
    std::function<void(opentxs::Context*)> callback =
        [&](opentxs::Context* in) -> void {
        this->save(dynamic_cast<opentxs::internal::Context*>(in));
    };

    OT_ASSERT(base);

    return Editor<opentxs::Context>(base.get(), callback);
}

std::shared_ptr<const opentxs::Nym> Wallet::signer_nym(
    const identifier::Nym& id) const
{
    return Nym(server_.NymID());
}
}  // namespace opentxs::api::server::implementation
