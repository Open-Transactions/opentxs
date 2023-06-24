// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "api/session/Wallet.hpp"  // IWYU pragma: associated

#include <Context.pb.h>
#include <Credential.pb.h>
#include <Issuer.pb.h>  // IWYU pragma: keep
#include <Nym.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <Purse.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <algorithm>
#include <compare>
#include <functional>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>

#include "2_Factory.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"  // IWYU pragma: keep
#include "internal/core/contract/SecurityContract.hpp"  // IWYU pragma: keep
#include "internal/core/contract/Types.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/PeerObject.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/NymFile.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"
#include "internal/serialization/protobuf/verify/Purse.hpp"
#include "internal/serialization/protobuf/verify/ServerContract.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Shared.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/peer/PeerObjectType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Parameters.hpp"      // IWYU pragma: keep
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PublishContract.hpp"
#include "opentxs/network/otdht/PublishContractReply.hpp"
#include "opentxs/network/otdht/QueryContract.hpp"
#include "opentxs/network/otdht/QueryContractReply.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Exclusive.tpp"
#include "util/Work.hpp"

template class opentxs::Exclusive<opentxs::Account>;
template class opentxs::Shared<opentxs::Account>;

namespace opentxs::api::session::imp
{
Wallet::Wallet(const api::Session& api)
    : api_(api)
    , context_map_()
    , context_map_lock_()
    , account_map_()
    , nym_map_()
    , server_map_()
    , unit_map_()
    , issuer_map_()
    , create_nym_lock_()
    , account_map_lock_()
    , nym_map_lock_()
    , server_map_lock_()
    , unit_map_lock_()
    , issuer_map_lock_()
    , peer_map_lock_()
    , peer_lock_()
    , nymfile_map_lock_()
    , nymfile_lock_()
    , purse_lock_()
    , purse_map_()
    , account_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , issuer_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , nym_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , nym_created_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , server_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , unit_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , peer_reply_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , peer_reply_new_publisher_(
          api_.Network().ZeroMQ().Internal().PublishSocket())
    , peer_request_publisher_(
          api_.Network().ZeroMQ().Internal().PublishSocket())
    , peer_request_new_publisher_(
          api_.Network().ZeroMQ().Internal().PublishSocket())
    , find_nym_(api_.Network().ZeroMQ().Internal().PushSocket(
          opentxs::network::zeromq::socket::Direction::Connect))
    , handle_([&] {
        using Type = opentxs::network::zeromq::socket::Type;

        return api_.Network().ZeroMQ().Internal().MakeBatch(
            {
                Type::Router,  // NOTE p2p_socket_
                Type::Pull,    // NOTE loopback_
            },
            "api::session::Wallet");
    }())
    , batch_([&]() -> auto& {
        using Callback = opentxs::network::zeromq::ListenCallback;
        auto& out = handle_.batch_;
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_p2p(std::move(in)); }));

        return out;
    }())
    , p2p_callback_(batch_.listen_callbacks_.at(0))
    , p2p_socket_([&]() -> auto& {
        auto& out = batch_.sockets_.at(0);
        const auto endpoint =
            CString{api_.Endpoints().Internal().OTDHTWallet()};
        const auto rc = out.Bind(endpoint.c_str());

        OT_ASSERT(rc);

        LogTrace()(OT_PRETTY_CLASS())("wallet socket bound to ")(endpoint)
            .Flush();

        return out;
    }())
    , loopback_(batch_.sockets_.at(1))
    , to_loopback_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        const auto endpoint = opentxs::network::zeromq::MakeArbitraryInproc();
        const auto& context = api_.Network().ZeroMQ();
        auto socket = factory::ZMQSocket(context, Type::Push);
        auto rc = loopback_.Bind(endpoint.c_str());

        OT_ASSERT(rc);

        rc = socket.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return socket;
    }())
    , thread_(api_.Network().ZeroMQ().Internal().Start(
          batch_.id_,
          {
              {p2p_socket_.ID(),
               &p2p_socket_,
               [id = p2p_socket_.ID(), &cb = p2p_callback_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {loopback_.ID(),
               &loopback_,
               [id = loopback_.ID(), &socket = p2p_socket_, &batch = batch_](
                   auto&& m) {
                   if (batch.toggle_) {
                       socket.Send(std::move(m), __FILE__, __LINE__);
                   }
               }},
          }))
{
    LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(batch_.id_).Flush();
    account_publisher_->Start(api_.Endpoints().AccountUpdate().data());
    issuer_publisher_->Start(api_.Endpoints().IssuerUpdate().data());
    nym_publisher_->Start(api_.Endpoints().NymDownload().data());
    nym_created_publisher_->Start(api_.Endpoints().NymCreated().data());
    server_publisher_->Start(api_.Endpoints().ServerUpdate().data());
    unit_publisher_->Start(api_.Endpoints().UnitUpdate().data());
    peer_reply_publisher_->Start(
        api_.Endpoints().Internal().PeerReplyUpdate().data());
    peer_reply_new_publisher_->Start(api_.Endpoints().PeerReply().data());
    peer_request_publisher_->Start(
        api_.Endpoints().Internal().PeerRequestUpdate().data());
    peer_request_new_publisher_->Start(api_.Endpoints().PeerRequest().data());
    find_nym_->Start(api_.Endpoints().FindNym().data());

    OT_ASSERT(nullptr != thread_);
}

auto Wallet::account(
    const Lock& lock,
    const identifier::Account& account,
    const bool create) const -> Wallet::AccountLock&
{
    OT_ASSERT(CheckLock(lock, account_map_lock_));

    auto& row = account_map_[account];
    auto& [rowMutex, pAccount] = row;

    if (pAccount) {
        LogVerbose()(OT_PRETTY_CLASS())("Account ")(account, api_.Crypto())(
            " already exists in map.")
            .Flush();

        return row;
    }

    eLock rowLock(rowMutex);
    // What if more than one thread tries to create the same row at the same
    // time? One thread will construct the Account object and the other(s) will
    // block until the lock is obtained. Therefore this check is necessary to
    // avoid creating the same account twice.
    if (pAccount) { return row; }

    UnallocatedCString serialized{""};
    UnallocatedCString alias{""};
    const auto loaded = api_.Storage().Load(
        account.asBase58(api_.Crypto()), serialized, alias, true);

    if (loaded) {
        LogVerbose()(OT_PRETTY_CLASS())("Account ")(account, api_.Crypto())(
            " loaded from storage.")
            .Flush();
        pAccount.reset(account_factory(account, alias, serialized));

        OT_ASSERT(pAccount);
    } else {
        if (false == create) {
            LogDetail()(OT_PRETTY_CLASS())("Trying to load account ")(
                account, api_.Crypto())(" via legacy method.")
                .Flush();
            const auto legacy = load_legacy_account(account, rowLock, row);

            if (legacy) { return row; }

            throw std::out_of_range("Unable to load account from storage");
        }
    }

    return row;
}

auto Wallet::Account(const identifier::Account& accountID) const
    -> SharedAccount
{
    Lock mapLock(account_map_lock_);

    try {
        auto& [rowMutex, pAccount] = account(mapLock, accountID, false);

        if (pAccount) { return {pAccount.get(), rowMutex}; }
    } catch (...) {

        return {};
    }

    return {};
}

auto Wallet::account_alias(
    const UnallocatedCString& accountID,
    const UnallocatedCString& hint) const -> UnallocatedCString
{
    if (false == hint.empty()) { return hint; }

    return api_.Storage().AccountAlias(
        api_.Factory().AccountIDFromBase58(accountID));
}

auto Wallet::account_factory(
    const identifier::Account& accountID,
    const UnallocatedCString& alias,
    const UnallocatedCString& serialized) const -> opentxs::Account*
{
    auto strContract = String::Factory(), strFirstLine = String::Factory();
    const bool bProcessed = DearmorAndTrim(
        api_.Crypto(),
        String::Factory(serialized.c_str()),
        strContract,
        strFirstLine);

    if (false == bProcessed) {
        LogError()(OT_PRETTY_CLASS())("Failed to dearmor serialized account.")
            .Flush();

        return nullptr;
    }

    const auto owner = api_.Storage().AccountOwner(accountID);
    const auto notary = api_.Storage().AccountServer(accountID);

    std::unique_ptr<opentxs::Account> pAccount{
        new opentxs::Account{api_, owner, accountID, notary}};

    if (false == bool(pAccount)) {
        LogError()(OT_PRETTY_CLASS())("Failed to create account.").Flush();

        return nullptr;
    }

    auto& account = *pAccount;

    if (account.GetNymID() != owner) {
        LogError()(OT_PRETTY_CLASS())("Nym id (")(
            account.GetNymID(), api_.Crypto())(
            ") does not match expect value (")(owner, api_.Crypto())(")")
            .Flush();
        account.SetNymID(owner);
    }

    if (account.GetRealAccountID() != accountID) {
        LogError()(OT_PRETTY_CLASS())("Account id (")(
            account.GetRealAccountID(), api_.Crypto())(
            ") does not match expect value (")(accountID, api_.Crypto())(")")
            .Flush();
        account.SetRealAccountID(accountID);
    }

    if (account.GetPurportedAccountID() != accountID) {
        LogError()(OT_PRETTY_CLASS())("Purported account id (")(
            account.GetPurportedAccountID(), api_.Crypto())(
            ") does not match expect value (")(accountID, api_.Crypto())(")")
            .Flush();
        account.SetPurportedAccountID(accountID);
    }

    if (account.GetRealNotaryID() != notary) {
        LogError()(OT_PRETTY_CLASS())("Notary id (")(
            account.GetRealNotaryID(), api_.Crypto())(
            ") does not match expect value (")(notary, api_.Crypto())(")")
            .Flush();
        account.SetRealNotaryID(notary);
    }

    if (account.GetPurportedNotaryID() != notary) {
        LogError()(OT_PRETTY_CLASS())("Purported notary id (")(
            account.GetPurportedNotaryID(), api_.Crypto())(
            ") does not match expect value (")(notary, api_.Crypto())(")")
            .Flush();
        account.SetPurportedNotaryID(notary);
    }

    account.SetLoadInsecure();
    auto deserialized = account.LoadContractFromString(strContract);

    if (false == deserialized) {
        LogError()(OT_PRETTY_CLASS())("Failed to deserialize account.").Flush();

        return nullptr;
    }

    const auto signerID = api_.Storage().AccountSigner(accountID);

    if (signerID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Unknown signer nym.").Flush();

        return nullptr;
    }

    const auto signerNym = Nym(signerID);

    if (false == bool(signerNym)) {
        LogError()(OT_PRETTY_CLASS())("Unable to load signer nym.").Flush();

        return nullptr;
    }

    if (false == account.VerifySignature(*signerNym)) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();

        return nullptr;
    }

    account.SetAlias(alias.c_str());

    return pAccount.release();
}

auto Wallet::AccountPartialMatch(const UnallocatedCString& hint) const
    -> identifier::Generic
{
    const auto list = api_.Storage().AccountList();

    for (const auto& [id, alias] : list) {
        if (0 == id.compare(0, hint.size(), hint)) {

            return api_.Factory().IdentifierFromBase58(id);
        }

        if (0 == alias.compare(0, hint.size(), hint)) {

            return api_.Factory().IdentifierFromBase58(alias);
        }
    }

    return {};
}

auto Wallet::BasketContract(
    const identifier::UnitDefinition& id,
    const std::chrono::milliseconds& timeout) const noexcept(false)
    -> OTBasketContract
{
    UnitDefinition(id, timeout);

    Lock mapLock(unit_map_lock_);
    auto it = unit_map_.find(id);

    if (unit_map_.end() == it) {
        throw std::runtime_error("Basket contract ID not found");
    }

    auto output = std::dynamic_pointer_cast<contract::unit::Basket>(it->second);

    if (output) {

        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Unit definition is not a basket contract");
    }
}

auto Wallet::CreateAccount(
    const identifier::Nym& ownerNymID,
    const identifier::Notary& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identity::Nym& signer,
    Account::AccountType type,
    TransactionNumber stash,
    const PasswordPrompt& reason) const -> ExclusiveAccount
{
    Lock mapLock(account_map_lock_);

    try {
        const auto contract = UnitDefinition(instrumentDefinitionID);
        std::unique_ptr<opentxs::Account> newAccount(
            opentxs::Account::GenerateNewAccount(
                api_,
                signer.ID(),
                notaryID,
                signer,
                ownerNymID,
                instrumentDefinitionID,
                reason,
                type,
                stash));

        OT_ASSERT(newAccount);

        const auto& accountID = newAccount->GetRealAccountID();
        auto& [rowMutex, pAccount] = account(mapLock, accountID, true);

        if (pAccount) {
            LogError()(OT_PRETTY_CLASS())("Account already exists.").Flush();

            return {};
        } else {
            pAccount.reset(newAccount.release());

            OT_ASSERT(pAccount);

            const auto id = accountID.asBase58(api_.Crypto());
            pAccount->SetNymID(ownerNymID);
            pAccount->SetPurportedAccountID(accountID);
            pAccount->SetRealNotaryID(notaryID);
            pAccount->SetPurportedNotaryID(notaryID);
            auto serialized = String::Factory();
            pAccount->SaveContractRaw(serialized);
            const auto saved = api_.Storage().Store(
                id,
                serialized->Get(),
                "",
                ownerNymID,
                signer.ID(),
                contract->Nym()->ID(),
                notaryID,
                instrumentDefinitionID,
                extract_unit(instrumentDefinitionID));

            OT_ASSERT(saved);

            std::function<void(
                std::unique_ptr<opentxs::Account>&, eLock&, bool)>
                callback = [this, id, &reason](
                               std::unique_ptr<opentxs::Account>& in,
                               eLock& lock,
                               bool success) -> void {
                this->save(reason, id, in, lock, success);
            };

            return {&pAccount, rowMutex, callback};
        }
    } catch (...) {

        return {};
    }
}

auto Wallet::DefaultNym() const noexcept
    -> std::pair<identifier::Nym, std::size_t>
{
    auto lock = Lock{create_nym_lock_};

    return std::make_pair(api_.Storage().DefaultNym(), LocalNymCount());
}

auto Wallet::DeleteAccount(const identifier::Account& accountID) const -> bool
{
    Lock mapLock(account_map_lock_);

    try {
        auto& [rowMutex, pAccount] = account(mapLock, accountID, false);
        eLock lock(rowMutex);

        if (pAccount) {
            const auto deleted =
                api_.Storage().DeleteAccount(accountID.asBase58(api_.Crypto()));

            if (deleted) {
                pAccount.reset();

                return true;
            }
        }
    } catch (...) {

        return false;
    }

    return false;
}

auto Wallet::IssuerAccount(const identifier::UnitDefinition& unitID) const
    -> SharedAccount
{
    const auto accounts = api_.Storage().AccountsByContract(unitID);
    Lock mapLock(account_map_lock_);

    try {
        for (const auto& accountID : accounts) {
            auto& [rowMutex, pAccount] = account(mapLock, accountID, false);

            if (pAccount) {
                if (pAccount->IsIssuer()) { return {pAccount.get(), rowMutex}; }
            }
        }
    } catch (...) {

        return {};
    }

    return {};
}

auto Wallet::mutable_Account(
    const identifier::Account& accountID,
    const PasswordPrompt& reason,
    const AccountCallback callback) const -> ExclusiveAccount
{
    Lock mapLock(account_map_lock_);

    try {
        auto& [rowMutex, pAccount] = account(mapLock, accountID, false);
        const auto id = accountID.asBase58(api_.Crypto());

        if (pAccount) {
            std::function<void(
                std::unique_ptr<opentxs::Account>&, eLock&, bool)>
                save = [this, id, &reason](
                           std::unique_ptr<opentxs::Account>& in,
                           eLock& lock,
                           bool success) -> void {
                this->save(reason, id, in, lock, success);
            };

            return {&pAccount, rowMutex, save, callback};
        }
    } catch (...) {

        return {};
    }

    return {};
}

auto Wallet::UpdateAccount(
    const identifier::Account& accountID,
    const otx::context::Server& context,
    const String& serialized,
    const PasswordPrompt& reason) const -> bool
{
    return UpdateAccount(accountID, context, serialized, "", reason);
}

auto Wallet::UpdateAccount(
    const identifier::Account& accountID,
    const otx::context::Server& context,
    const String& serialized,
    const UnallocatedCString& label,
    const PasswordPrompt& reason) const -> bool
{
    Lock mapLock(account_map_lock_);
    auto& [rowMutex, pAccount] = account(mapLock, accountID, true);
    eLock rowLock(rowMutex);
    mapLock.unlock();
    const auto& localNym = *context.Nym();
    std::unique_ptr<opentxs::Account> newAccount{nullptr};
    newAccount.reset(
        new opentxs::Account(api_, localNym.ID(), accountID, context.Notary()));

    if (false == bool(newAccount)) {
        LogError()(OT_PRETTY_CLASS())("Unable to construct account.").Flush();

        return false;
    }

    if (false == newAccount->LoadContractFromString(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Unable to deserialize account.").Flush();

        return false;
    }

    if (false == newAccount->VerifyAccount(context.RemoteNym())) {
        LogError()(OT_PRETTY_CLASS())("Unable to verify account.").Flush();

        return false;
    }

    if (localNym.ID() != newAccount->GetNymID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong nym on account.").Flush();

        return false;
    }

    if (context.Notary() != newAccount->GetRealNotaryID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong server on account.").Flush();

        return false;
    }

    newAccount->ReleaseSignatures();

    if (false == newAccount->SignContract(localNym, reason)) {
        LogError()(OT_PRETTY_CLASS())("Unable to sign account.").Flush();

        return false;
    }

    if (false == newAccount->SaveContract()) {
        LogError()(OT_PRETTY_CLASS())("Unable to serialize account.").Flush();

        return false;
    }

    pAccount.reset(newAccount.release());

    OT_ASSERT(pAccount);

    const auto& unitID = pAccount->GetInstrumentDefinitionID();

    try {
        const auto contract = UnitDefinition(unitID);
        auto raw = String::Factory();
        auto saved = pAccount->SaveContractRaw(raw);

        if (false == saved) {
            LogError()(OT_PRETTY_CLASS())("Unable to serialize account.")
                .Flush();

            return false;
        }

        const auto alias =
            account_alias(accountID.asBase58(api_.Crypto()), label);
        saved = api_.Storage().Store(
            accountID.asBase58(api_.Crypto()),
            raw->Get(),
            alias,
            localNym.ID(),
            localNym.ID(),
            contract->Nym()->ID(),
            context.Notary(),
            unitID,
            extract_unit(contract));

        if (false == saved) {
            LogError()(OT_PRETTY_CLASS())("Unable to save account.").Flush();

            return false;
        }

        pAccount->SetAlias(alias);
        const auto balance = pAccount->GetBalance();
        account_publisher_->Send([&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::AccountUpdated, true);
            accountID.Internal().Serialize(work);
            balance.Serialize(work.AppendBytes());

            return work;
        }());

        return true;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to load unit definition contract ")(unitID, api_.Crypto())
            .Flush();

        return false;
    }
}

auto Wallet::CurrencyTypeBasedOnUnitType(
    const identifier::UnitDefinition& contractID) const -> UnitType
{
    return extract_unit(contractID);
}

auto Wallet::extract_unit(const identifier::UnitDefinition& contractID) const
    -> UnitType
{
    try {
        const auto contract = UnitDefinition(contractID);

        return extract_unit(contract);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            " Unable to load unit definition contract ")(
            contractID, api_.Crypto())(".")
            .Flush();

        return UnitType::Unknown;
    }
}

auto Wallet::extract_unit(const contract::Unit& contract) const -> UnitType
{
    try {
        return contract.UnitOfAccount();
    } catch (...) {

        return UnitType::Unknown;
    }
}

auto Wallet::context(
    const identifier::Nym& localNymID,
    const identifier::Nym& remoteNymID) const
    -> std::shared_ptr<otx::context::Base>
{
    const auto local = localNymID.asBase58(api_.Crypto());
    const auto remote = remoteNymID.asBase58(api_.Crypto());
    const ContextID context = {local, remote};
    auto it = context_map_.find(context);
    const bool inMap = (it != context_map_.end());

    if (inMap) { return it->second; }

    // Load from storage, if it exists.
    auto serialized = proto::Context{};
    const bool loaded =
        api_.Storage().Load(localNymID, remoteNymID, serialized, true);

    if (!loaded) { return nullptr; }

    if (local != serialized.localnym()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect localnym in protobuf.")
            .Flush();

        return nullptr;
    }

    if (remote != serialized.remotenym()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect localnym in protobuf.")
            .Flush();

        return nullptr;
    }

    auto& entry = context_map_[context];

    // Obtain nyms.
    const auto localNym = Nym(localNymID);
    const auto remoteNym = Nym(remoteNymID);

    if (!localNym) {
        LogError()(OT_PRETTY_CLASS())("Unable to load local nym.").Flush();

        return nullptr;
    }

    if (!remoteNym) {
        LogError()(OT_PRETTY_CLASS())("Unable to load remote nym.").Flush();

        return nullptr;
    }

    switch (translate(serialized.type())) {
        case otx::ConsensusType::Server: {
            instantiate_server_context(serialized, localNym, remoteNym, entry);
        } break;
        case otx::ConsensusType::Client: {
            instantiate_client_context(serialized, localNym, remoteNym, entry);
        } break;
        case otx::ConsensusType::Error:
        case otx::ConsensusType::Peer:
        default: {
            return nullptr;
        }
    }

    OT_ASSERT(entry);

    const bool valid = entry->Validate();

    if (!valid) {
        context_map_.erase(context);

        LogError()(OT_PRETTY_CLASS())("Invalid signature on context.").Flush();

        OT_FAIL;
    }

    return entry;
}

auto Wallet::ClientContext(const identifier::Nym& remoteNymID) const
    -> std::shared_ptr<const otx::context::Client>
{
    // Overridden in appropriate child class.
    OT_FAIL;
}

auto Wallet::ServerContext(
    const identifier::Nym& localNymID,
    const identifier::Generic& remoteID) const
    -> std::shared_ptr<const otx::context::Server>
{
    // Overridden in appropriate child class.
    OT_FAIL;
}

auto Wallet::mutable_ClientContext(
    const identifier::Nym& remoteNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Client>
{
    // Overridden in appropriate child class.
    OT_FAIL;
}

auto Wallet::mutable_ServerContext(
    const identifier::Nym& localNymID,
    const identifier::Generic& remoteID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Server>
{
    // Overridden in appropriate child class.
    OT_FAIL;
}

auto Wallet::ImportAccount(std::unique_ptr<opentxs::Account>& imported) const
    -> bool
{
    if (false == bool(imported)) {
        LogError()(OT_PRETTY_CLASS())("Invalid account.").Flush();

        return false;
    }

    const auto& accountID = imported->GetRealAccountID();
    Lock mapLock(account_map_lock_);

    try {
        auto& [rowMutex, pAccount] = account(mapLock, accountID, true);
        eLock rowLock(rowMutex);
        mapLock.unlock();

        if (pAccount) {
            LogError()(OT_PRETTY_CLASS())("Account already exists.").Flush();

            return false;
        }

        pAccount.reset(imported.release());

        OT_ASSERT(pAccount);

        const auto& contractID = pAccount->GetInstrumentDefinitionID();

        try {
            const auto contract = UnitDefinition(contractID);
            auto serialized = String::Factory();
            auto alias = String::Factory();
            pAccount->SaveContractRaw(serialized);
            pAccount->GetName(alias);
            const auto saved = api_.Storage().Store(
                accountID.asBase58(api_.Crypto()),
                serialized->Get(),
                alias->Get(),
                pAccount->GetNymID(),
                pAccount->GetNymID(),
                contract->Nym()->ID(),
                pAccount->GetRealNotaryID(),
                contractID,
                extract_unit(contract));

            if (false == saved) {
                LogError()(OT_PRETTY_CLASS())("Failed to save account.")
                    .Flush();
                imported.reset(pAccount.release());

                return false;
            }

            return true;
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("Unable to load unit definition.")
                .Flush();
            imported.reset(pAccount.release());

            return false;
        }
    } catch (...) {
    }

    LogError()(OT_PRETTY_CLASS())("Unable to import account.").Flush();

    return false;
}

auto Wallet::IssuerList(const identifier::Nym& nymID) const
    -> UnallocatedSet<identifier::Nym>
{
    UnallocatedSet<identifier::Nym> output{};
    auto list = api_.Storage().IssuerList(nymID);

    for (const auto& it : list) {
        output.emplace(api_.Factory().NymIDFromBase58(it.first));
    }

    return output;
}

auto Wallet::Issuer(
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) const
    -> std::shared_ptr<const otx::client::Issuer>
{
    auto& [lock, pIssuer] = issuer(nymID, issuerID, false);

    return pIssuer;
}

auto Wallet::mutable_Issuer(
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) const -> Editor<otx::client::Issuer>
{
    auto& [mutex, pIssuer] = issuer(nymID, issuerID, true);

    OT_ASSERT(pIssuer);

    std::function<void(otx::client::Issuer*, const Lock&)> callback =
        [=, this](otx::client::Issuer* in, const Lock& lock) -> void {
        save(lock, in);
    };

    return {mutex, pIssuer.get(), callback};
}

auto Wallet::issuer(
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID,
    const bool create) const -> Wallet::IssuerLock&
{
    Lock lock(issuer_map_lock_);
    const auto key = IssuerID{nymID, issuerID};
    auto& output = issuer_map_[key];
    auto& [issuerMutex, pIssuer] = output;

    if (pIssuer) { return output; }

    const auto isBlockchain =
        (blockchain::Type::UnknownBlockchain !=
         blockchain::Chain(api_, issuerID));

    if (isBlockchain) {
        LogError()(OT_PRETTY_CLASS())(
            " erroneously attempting to load a blockchain as an otx issuer")
            .Flush();
    }

    auto serialized = proto::Issuer{};
    const bool loaded = api_.Storage().Load(
        nymID, issuerID.asBase58(api_.Crypto()), serialized, true);

    if (loaded) {
        if (isBlockchain) {
            LogError()(OT_PRETTY_CLASS())("deleting invalid issuer").Flush();
            // TODO
        } else {
            pIssuer.reset(factory::Issuer(
                api_.Crypto(), api_.Factory(), *this, nymID, serialized));

            OT_ASSERT(pIssuer);

            return output;
        }
    }

    if (create && (!isBlockchain)) {
        pIssuer.reset(factory::Issuer(
            api_.Crypto(), api_.Factory(), *this, nymID, issuerID));

        OT_ASSERT(pIssuer);

        save(lock, pIssuer.get());

        return output;
    }

    issuer_map_.erase(key);
    static auto blank = IssuerLock{};

    return blank;
}

auto Wallet::IsLocalNym(const std::string_view id) const -> bool
{
    return IsLocalNym(api_.Factory().NymIDFromBase58(id));
}

auto Wallet::IsLocalNym(const identifier::Nym& id) const -> bool
{
    return api_.Storage().LocalNyms().count(id);
}

auto Wallet::LocalNymCount() const -> std::size_t
{
    return api_.Storage().LocalNyms().size();
}

auto Wallet::LocalNyms() const -> Set<identifier::Nym>
{
    return api_.Storage().LocalNyms();
}

auto Wallet::Nym(
    const identifier::Nym& id,
    const std::chrono::milliseconds& timeout) const -> Nym_p
{
    if (blockchain::Type::UnknownBlockchain != blockchain::Chain(api_, id)) {
        LogError()(OT_PRETTY_CLASS())(
            " erroneously attempting to load a blockchain as a nym")
            .Flush();

        return nullptr;
    }

    auto mapLock = Lock{nym_map_lock_};
    bool inMap = (nym_map_.find(id) != nym_map_.end());
    bool valid = false;

    if (!inMap) {
        auto serialized = proto::Nym{};
        auto alias = UnallocatedCString{};
        bool loaded = api_.Storage().Load(id, serialized, alias, true);

        if (loaded) {
            auto& pNym = nym_map_[id].second;
            pNym.reset(opentxs::Factory::Nym(api_, serialized, alias));

            if (pNym && pNym->CompareID(id)) {
                valid = pNym->VerifyPseudonym();
                pNym->SetAliasStartup(alias);
            } else {
                nym_map_.erase(id);
            }
        } else {
            search_nym(id);

            if (timeout > 0ms) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = 100ms;

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (nym_map_.find(id) != nym_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return Nym(id);  // timeout of zero prevents infinite
                                 // recursion
            }
        }
    } else {
        auto& pNym = nym_map_[id].second;
        if (pNym) { valid = pNym->VerifyPseudonym(); }
    }

    if (valid) { return nym_map_[id].second; }

    return nullptr;
}

auto Wallet::Nym(const proto::Nym& serialized) const -> Nym_p
{
    const auto nymID = api_.Factory().NymIDFromBase58(serialized.nymid());

    if (nymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym ID.").Flush();

        return {};
    }

    auto existing = Nym(nymID);

    if (existing && (existing->Revision() >= serialized.revision())) {
        LogDetail()(OT_PRETTY_CLASS())(
            " Incoming nym is not newer than existing nym.")
            .Flush();

        return existing;
    } else {
        auto pCandidate = std::unique_ptr<identity::internal::Nym>{
            opentxs::Factory::Nym(api_, serialized, "")};

        if (false == bool(pCandidate)) { return {}; }

        auto& candidate = *pCandidate;

        if (false == candidate.CompareID(nymID)) { return existing; }

        if (candidate.VerifyPseudonym()) {
            LogDetail()(OT_PRETTY_CLASS())("Saving updated nym ")(
                nymID, api_.Crypto())
                .Flush();
            candidate.WriteCredentials();
            SaveCredentialIDs(candidate);
            auto mapNym = [&] {
                auto mapLock = Lock{nym_map_lock_};
                auto& out = nym_map_[nymID].second;
                // TODO update existing nym rather than destroying it
                out.reset(pCandidate.release());

                return out;
            }();

            notify_new(nymID);

            return mapNym;
        } else {
            LogError()(OT_PRETTY_CLASS())("Incoming nym is not valid.").Flush();
        }
    }

    return existing;
}

auto Wallet::Nym(const ReadView& bytes) const -> Nym_p
{
    return Nym(proto::Factory<proto::Nym>(bytes));
}

auto Wallet::Nym(
    const identity::Type type,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    return Nym({api_.Factory()}, type, reason, name);
}

auto Wallet::Nym(
    const opentxs::crypto::Parameters& parameters,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    return Nym(parameters, identity::Type::individual, reason, name);
}

auto Wallet::Nym(const PasswordPrompt& reason, const UnallocatedCString& name)
    const -> Nym_p
{
    return Nym({api_.Factory()}, identity::Type::individual, reason, name);
}

auto Wallet::Nym(
    const opentxs::crypto::Parameters& parameters,
    const identity::Type type,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    auto lock = Lock{create_nym_lock_};
    std::shared_ptr<identity::internal::Nym> pNym(
        opentxs::Factory::Nym(api_, parameters, type, name, reason));

    if (false == bool(pNym)) {
        LogError()(OT_PRETTY_CLASS())("Failed to create nym").Flush();

        return {};
    }

    const auto first = (0u == LocalNymCount());
    auto& nym = *pNym;
    const auto& id = nym.ID();

    if (nym.VerifyPseudonym()) {
        nym.SetAlias(name);

        {
            auto mapLock = Lock{nym_map_lock_};
            auto it = nym_map_.find(id);

            if (nym_map_.end() != it) { return it->second.second; }
        }

        if (SaveCredentialIDs(nym)) {
            nym_to_contact(nym, name);

            {
                auto nymfile = mutable_nymfile(pNym, pNym, id, reason);
            }

            if (first) {
                LogTrace()(OT_PRETTY_CLASS())(
                    "Marking first created nym as default");
                api_.Storage().SetDefaultNym(id);
            } else {
                LogTrace()(OT_PRETTY_CLASS())("Default nym already set")
                    .Flush();
            }

            {
                auto mapLock = Lock{nym_map_lock_};
                auto& pMapNym = nym_map_[id].second;
                pMapNym = pNym;
                nym_created_publisher_->Send([&] {
                    auto work = opentxs::network::zeromq::tagged_message(
                        WorkType::NymCreated, true);
                    work.AddFrame(pNym->ID());

                    return work;
                }());
            }

            return pNym;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to save credentials").Flush();

            return {};
        }
    } else {

        return {};
    }
}

auto Wallet::mutable_Nym(
    const identifier::Nym& id,
    const PasswordPrompt& reason) const -> NymData
{
    const UnallocatedCString nym = id.asBase58(api_.Crypto());
    auto exists = Nym(id);

    if (false == bool(exists)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nym)(" not found.").Flush();
    }

    auto mapLock = Lock{nym_map_lock_};
    auto it = nym_map_.find(id);

    if (nym_map_.end() == it) { OT_FAIL; }

    std::function<void(NymData*, Lock&)> callback = [&](NymData* nymData,
                                                        Lock& lock) -> void {
        this->save(nymData, lock);
    };

    return {
        api_.Crypto(),
        api_.Factory(),
        it->second.first,
        it->second.second,
        callback};
}

auto Wallet::Nymfile(const identifier::Nym& id, const PasswordPrompt& reason)
    const -> std::unique_ptr<const opentxs::NymFile>
{
    Lock lock(nymfile_lock(id));
    const auto targetNym = Nym(id);
    const auto signerNym = signer_nym(id);

    if (false == bool(targetNym)) { return {}; }
    if (false == bool(signerNym)) { return {}; }

    auto nymfile = std::unique_ptr<opentxs::internal::NymFile>(
        opentxs::Factory::NymFile(api_, targetNym, signerNym));

    OT_ASSERT(nymfile);

    if (false == nymfile->LoadSignedNymFile(reason)) {
        LogError()(OT_PRETTY_CLASS())(" Failure calling load_signed_nymfile: ")(
            id, api_.Crypto())(".")
            .Flush();

        return {};
    }

    return nymfile;
}

auto Wallet::mutable_Nymfile(
    const identifier::Nym& id,
    const PasswordPrompt& reason) const -> Editor<opentxs::NymFile>
{
    const auto targetNym = Nym(id);
    const auto signerNym = signer_nym(id);

    return mutable_nymfile(targetNym, signerNym, id, reason);
}

auto Wallet::mutable_nymfile(
    const Nym_p& targetNym,
    const Nym_p& signerNym,
    const identifier::Nym& id,
    const PasswordPrompt& reason) const -> Editor<opentxs::NymFile>
{
    auto nymfile = std::unique_ptr<opentxs::internal::NymFile>(
        opentxs::Factory::NymFile(api_, targetNym, signerNym));

    OT_ASSERT(nymfile);

    if (false == nymfile->LoadSignedNymFile(reason)) {
        nymfile->SaveSignedNymFile(reason);
    }

    using EditorType = Editor<opentxs::NymFile>;
    EditorType::LockedSave callback = [&](opentxs::NymFile* in,
                                          Lock& lock) -> void {
        this->save(reason, in, lock);
    };
    EditorType::OptionalCallback deleter = [](const opentxs::NymFile& in) {
        auto* p = &const_cast<opentxs::NymFile&>(in);
        delete p;
    };

    return {nymfile_lock(id), nymfile.release(), callback, deleter};
}

auto Wallet::notify_changed(const identifier::Nym& id) const noexcept -> void
{
    nym_publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::NymUpdated, true);
        work.AddFrame(id);

        return work;
    }());
}

auto Wallet::notify_new(const identifier::Nym& id) const noexcept -> void
{
    api_.Internal().NewNym(id);
    notify_changed(id);
}

auto Wallet::nymfile_lock(const identifier::Nym& nymID) const -> std::mutex&
{
    Lock map_lock(nymfile_map_lock_);
    auto& output = nymfile_lock_[nymID];
    map_lock.unlock();

    return output;
}

auto Wallet::NymByIDPartialMatch(const UnallocatedCString& hint) const -> Nym_p
{
    const auto str =
        api_.Factory().NymIDFromBase58(hint).asBase58(api_.Crypto());

    for (const auto& [id, alias] : api_.Storage().NymList()) {
        const auto match = (id.compare(0, hint.length(), hint) == 0) ||
                           (id.compare(0, str.length(), str) == 0) ||
                           (alias.compare(0, hint.length(), hint) == 0);

        if (match) { return Nym(api_.Factory().NymIDFromBase58(id)); }
    }

    return {};
}

auto Wallet::NymList() const -> ObjectList { return api_.Storage().NymList(); }

auto Wallet::NymNameByIndex(const std::size_t index, String& name) const -> bool
{
    const auto nymNames = api_.Storage().LocalNyms();

    if (index < nymNames.size()) {
        std::size_t idx{0};
        for (const auto& nymName : nymNames) {
            if (idx == index) {
                name.Set(String::Factory(nymName, api_.Crypto()));

                return true;
            }

            ++idx;
        }
    }

    return false;
}

auto Wallet::peer_lock(const UnallocatedCString& nymID) const -> std::mutex&
{
    Lock map_lock(peer_map_lock_);
    auto& output = peer_lock_[nymID];
    map_lock.unlock();

    return output;
}

auto Wallet::PeerReply(
    const identifier::Nym& nym,
    const identifier::Generic& reply,
    const otx::client::StorageBox& box,
    proto::PeerReply& output) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());

    Lock lock(peer_lock(nymID));
    if (false == api_.Storage().Load(
                     nym, reply.asBase58(api_.Crypto()), box, output, true)) {
        return false;
    }

    return true;
}

auto Wallet::PeerReply(
    const identifier::Nym& nym,
    const identifier::Generic& reply,
    const otx::client::StorageBox& box,
    Writer&& destination) const -> bool
{
    auto peerreply = proto::PeerReply{};
    if (false == PeerReply(nym, reply, box, peerreply)) { return false; }

    return write(peerreply, std::move(destination));
}

auto Wallet::PeerReplyComplete(
    const identifier::Nym& nym,
    const identifier::Generic& replyID) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    auto reply = proto::PeerReply{};
    Lock lock(peer_lock(nymID));
    const bool haveReply = api_.Storage().Load(
        nym,
        replyID.asBase58(api_.Crypto()),
        otx::client::StorageBox::SENTPEERREPLY,
        reply,
        false);

    if (!haveReply) {
        LogError()(OT_PRETTY_CLASS())("Sent reply not found.").Flush();

        return false;
    }

    // This reply may have been loaded by request id.
    const auto& realReplyID = reply.id();

    const bool savedReply = api_.Storage().Store(
        reply, nym, otx::client::StorageBox::FINISHEDPEERREPLY);

    if (!savedReply) {
        LogError()(OT_PRETTY_CLASS())("Failed to save finished reply.").Flush();

        return false;
    }

    const bool removedReply = api_.Storage().RemoveNymBoxItem(
        nym, otx::client::StorageBox::SENTPEERREPLY, realReplyID);

    if (!removedReply) {
        LogError()(OT_PRETTY_CLASS())(
            " Failed to delete finished reply from sent box.")
            .Flush();
    }

    return removedReply;
}

auto Wallet::PeerReplyCreate(
    const identifier::Nym& nym,
    const proto::PeerRequest& request,
    const proto::PeerReply& reply) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    if (reply.cookie() != request.id()) {
        LogError()(OT_PRETTY_CLASS())(
            " Reply cookie does not match request id.")
            .Flush();

        return false;
    }

    if (reply.type() != request.type()) {
        LogError()(OT_PRETTY_CLASS())(
            " Reply type does not match request type.")
            .Flush();

        return false;
    }

    const bool createdReply = api_.Storage().Store(
        reply, nym, otx::client::StorageBox::SENTPEERREPLY);

    if (!createdReply) {
        LogError()(OT_PRETTY_CLASS())("Failed to save sent reply.").Flush();

        return false;
    }

    const bool processedRequest = api_.Storage().Store(
        request, nym, otx::client::StorageBox::PROCESSEDPEERREQUEST);

    if (!processedRequest) {
        LogError()(OT_PRETTY_CLASS())("Failed to save processed request.")
            .Flush();

        return false;
    }

    const bool movedRequest = api_.Storage().RemoveNymBoxItem(
        nym, otx::client::StorageBox::INCOMINGPEERREQUEST, request.id());

    if (!processedRequest) {
        LogError()(OT_PRETTY_CLASS())(
            " Failed to delete processed request from incoming box.")
            .Flush();
    }

    return movedRequest;
}

auto Wallet::PeerReplyCreateRollback(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const identifier::Generic& reply) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));
    const auto requestID = request.asBase58(api_.Crypto());
    const auto replyID = reply.asBase58(api_.Crypto());
    auto requestItem = proto::PeerRequest{};
    bool output = true;
    time_t notUsed = 0;
    const bool loadedRequest = api_.Storage().Load(
        nym,
        requestID,
        otx::client::StorageBox::PROCESSEDPEERREQUEST,
        requestItem,
        notUsed);

    if (loadedRequest) {
        const bool requestRolledBack = api_.Storage().Store(
            requestItem, nym, otx::client::StorageBox::INCOMINGPEERREQUEST);

        if (requestRolledBack) {
            const bool purgedRequest = api_.Storage().RemoveNymBoxItem(
                nym, otx::client::StorageBox::PROCESSEDPEERREQUEST, requestID);
            if (!purgedRequest) {
                LogError()(OT_PRETTY_CLASS())(
                    " Failed to delete request from processed box.")
                    .Flush();
                output = false;
            }
        } else {
            LogError()(OT_PRETTY_CLASS())(
                " Failed to save request to incoming box.")
                .Flush();
            output = false;
        }
    } else {
        LogError()(OT_PRETTY_CLASS())(
            " Did not find the request in the processed box.")
            .Flush();
        output = false;
    }

    const bool removedReply = api_.Storage().RemoveNymBoxItem(
        nym, otx::client::StorageBox::SENTPEERREPLY, replyID);

    if (!removedReply) {
        LogError()(OT_PRETTY_CLASS())(" Failed to delete reply from sent box.")
            .Flush();
        output = false;
    }

    return output;
}

auto Wallet::PeerReplySent(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::SENTPEERREPLY);
}

auto Wallet::PeerReplyIncoming(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::INCOMINGPEERREPLY);
}

auto Wallet::PeerReplyFinished(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::FINISHEDPEERREPLY);
}

auto Wallet::PeerReplyProcessed(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::PROCESSEDPEERREPLY);
}

auto Wallet::PeerReplyReceive(
    const identifier::Nym& nym,
    const PeerObject& object) const -> bool
{
    if (contract::peer::PeerObjectType::Response != object.Type()) {
        LogError()(OT_PRETTY_CLASS())("This is not a peer reply.").Flush();

        return false;
    }

    const auto request = object.Request();
    const auto reply = object.Reply();

    if (0 == request->Version()) {
        LogError()(OT_PRETTY_CLASS())("Null request.").Flush();

        return false;
    }

    if (0 == reply->Version()) {
        LogError()(OT_PRETTY_CLASS())("Null reply.").Flush();

        return false;
    }

    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));
    auto requestID = request->ID();
    auto serializedRequest = proto::PeerRequest{};
    std::time_t notUsed;
    const bool haveRequest = api_.Storage().Load(
        nym,
        requestID.asBase58(api_.Crypto()),
        otx::client::StorageBox::SENTPEERREQUEST,
        serializedRequest,
        notUsed,
        false);

    if (false == haveRequest) {
        LogError()(OT_PRETTY_CLASS())(
            " The request for this reply does not exist in the sent box.")
            .Flush();

        return false;
    }

    auto serialized = proto::PeerReply{};

    if (false == reply->Serialize(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize reply.").Flush();

        return false;
    }
    const bool receivedReply = api_.Storage().Store(
        serialized, nym, otx::client::StorageBox::INCOMINGPEERREPLY);

    if (receivedReply) {
        peer_reply_publisher_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame();
            out.AddFrame(nym);
            out.Internal().AddFrame(serialized);

            return out;
        }());
        peer_reply_new_publisher_->Send([&] {
            auto out = MakeWork(WorkType::PeerRequest);
            reply->ID().Internal().Serialize(out);
            reply->Recipient().Internal().Serialize(out);
            reply->Initiator().Internal().Serialize(out);
            out.AddFrame(reply->Type());
            out.Internal().AddFrame(serialized);

            return out;
        }());
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save incoming reply.").Flush();

        return false;
    }

    const bool finishedRequest = api_.Storage().Store(
        serializedRequest, nym, otx::client::StorageBox::FINISHEDPEERREQUEST);

    if (!finishedRequest) {
        LogError()(OT_PRETTY_CLASS())(
            " Failed to save request to finished box.")
            .Flush();

        return false;
    }

    const bool removedRequest = api_.Storage().RemoveNymBoxItem(
        nym,
        otx::client::StorageBox::SENTPEERREQUEST,
        requestID.asBase58(api_.Crypto()));

    if (!finishedRequest) {
        LogError()(OT_PRETTY_CLASS())(
            " Failed to delete finished request from sent box.")
            .Flush();
    }

    return removedRequest;
}

auto Wallet::PeerRequest(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box,
    std::time_t& time,
    proto::PeerRequest& output) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());

    Lock lock(peer_lock(nymID));
    if (false ==
        api_.Storage().Load(
            nym, request.asBase58(api_.Crypto()), box, output, time, true)) {
        return false;
    }

    return true;
}

auto Wallet::PeerRequest(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box,
    std::time_t& time,
    Writer&& destination) const -> bool
{
    auto peerrequest = proto::PeerRequest{};
    if (false == PeerRequest(nym, request, box, time, peerrequest)) {
        return false;
    }

    return write(peerrequest, std::move(destination));
}

auto Wallet::PeerRequestComplete(
    const identifier::Nym& nym,
    const identifier::Generic& replyID) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));
    auto reply = proto::PeerReply{};
    const bool haveReply = api_.Storage().Load(
        nym,
        replyID.asBase58(api_.Crypto()),
        otx::client::StorageBox::INCOMINGPEERREPLY,
        reply,
        false);

    if (!haveReply) {
        LogError()(OT_PRETTY_CLASS())(
            " The reply does not exist in the incoming box.")
            .Flush();

        return false;
    }

    // This reply may have been loaded by request id.
    const auto& realReplyID = reply.id();

    const bool storedReply = api_.Storage().Store(
        reply, nym, otx::client::StorageBox::PROCESSEDPEERREPLY);

    if (!storedReply) {
        LogError()(OT_PRETTY_CLASS())(" Failed to save reply to processed box.")
            .Flush();

        return false;
    }

    const bool removedReply = api_.Storage().RemoveNymBoxItem(
        nym, otx::client::StorageBox::INCOMINGPEERREPLY, realReplyID);

    if (!removedReply) {
        LogError()(OT_PRETTY_CLASS())(
            " Failed to delete completed reply from incoming box.")
            .Flush();
    }

    return removedReply;
}

auto Wallet::PeerRequestCreate(
    const identifier::Nym& nym,
    const proto::PeerRequest& request) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().Store(
        request, nym, otx::client::StorageBox::SENTPEERREQUEST);
}

auto Wallet::PeerRequestCreateRollback(
    const identifier::Nym& nym,
    const identifier::Generic& request) const -> bool
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().RemoveNymBoxItem(
        nym,
        otx::client::StorageBox::SENTPEERREQUEST,
        request.asBase58(api_.Crypto()));
}

auto Wallet::PeerRequestDelete(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box) const -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST:
        case otx::client::StorageBox::INCOMINGPEERREQUEST:
        case otx::client::StorageBox::FINISHEDPEERREQUEST:
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
            return api_.Storage().RemoveNymBoxItem(
                nym, box, request.asBase58(api_.Crypto()));
        }
        default: {
            return false;
        }
    }
}

auto Wallet::PeerRequestSent(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::SENTPEERREQUEST);
}

auto Wallet::PeerRequestIncoming(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::INCOMINGPEERREQUEST);
}

auto Wallet::PeerRequestFinished(const identifier::Nym& nym) const -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::FINISHEDPEERREQUEST);
}

auto Wallet::PeerRequestProcessed(const identifier::Nym& nym) const
    -> ObjectList
{
    const auto nymID = nym.asBase58(api_.Crypto());
    Lock lock(peer_lock(nymID));

    return api_.Storage().NymBoxList(
        nym, otx::client::StorageBox::PROCESSEDPEERREQUEST);
}

auto Wallet::PeerRequestReceive(
    const identifier::Nym& nym,
    const PeerObject& object) const -> bool
{
    if (contract::peer::PeerObjectType::Request != object.Type()) {
        LogError()(OT_PRETTY_CLASS())("This is not a peer request.").Flush();

        return false;
    }

    const auto& request = object.Request();

    if (0 == request->Version()) {
        LogError()(OT_PRETTY_CLASS())("Null request.").Flush();

        return false;
    }

    auto serialized = proto::PeerRequest{};

    if (false == request->Serialize(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize request.").Flush();

        return false;
    }

    const auto nymID = nym.asBase58(api_.Crypto());
    const auto saved = [&] {
        Lock lock(peer_lock(nymID));

        return api_.Storage().Store(
            serialized, nym, otx::client::StorageBox::INCOMINGPEERREQUEST);
    }();

    if (saved) {
        peer_request_publisher_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame();
            out.AddFrame(nymID);
            out.Internal().AddFrame(serialized);

            return out;
        }());
        peer_request_new_publisher_->Send([&] {
            auto out = MakeWork(WorkType::PeerRequest);
            request->ID().Internal().Serialize(out);
            request->Recipient().Internal().Serialize(out);
            request->Initiator().Internal().Serialize(out);
            out.AddFrame(request->Type());
            out.Internal().AddFrame(serialized);

            return out;
        }());
    }

    return saved;
}

auto Wallet::PeerRequestUpdate(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box) const -> bool
{
    switch (box) {
        case otx::client::StorageBox::SENTPEERREQUEST:
        case otx::client::StorageBox::INCOMINGPEERREQUEST:
        case otx::client::StorageBox::FINISHEDPEERREQUEST:
        case otx::client::StorageBox::PROCESSEDPEERREQUEST: {
            return api_.Storage().SetPeerRequestTime(
                nym, request.asBase58(api_.Crypto()), box);
        }
        default: {
            return false;
        }
    }
}

auto Wallet::process_p2p(opentxs::network::zeromq::Message&& msg) const noexcept
    -> void
{
    const auto body = msg.Payload();

    if (0 == body.size()) { OT_FAIL; }

    using Job = opentxs::network::otdht::Job;
    const auto type = body[0].as<Job>();

    switch (type) {
        case Job::Response: {
            process_p2p_response(std::move(msg));
        } break;
        case Job::PublishContract: {
            process_p2p_publish_contract(std::move(msg));
        } break;
        case Job::QueryContract: {
            process_p2p_query_contract(std::move(msg));
        } break;
        case Job::Register: {
            batch_.toggle_ = true;
        } break;
        case Job::Shutdown:
        case Job::BlockHeader:
        case Job::Reorg:
        case Job::SyncServerUpdated:
        case Job::SyncAck:
        case Job::SyncReply:
        case Job::SyncPush:
        case Job::Request:
        case Job::Processed:
        case Job::StateMachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "Unsupported message type on internal socket: ")(
                static_cast<OTZMQWorkType>(type))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Wallet::process_p2p_publish_contract(
    opentxs::network::zeromq::Message&& msg) const noexcept -> void
{
    try {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        if (!base) {
            throw std::runtime_error{"failed to instantiate message"};
        }

        using Type = opentxs::network::otdht::MessageType;
        const auto type = base->Type();

        if (Type::publish_contract != type) {
            const auto error = CString{}
                                   .append("Unsupported message type ")
                                   .append(print(type));

            throw std::runtime_error{error.c_str()};
        }

        const auto& contract = base->asPublishContract();
        const auto& id = contract.ID();
        auto payload = [&] {
            const auto ctype = contract.ContractType();
            using enum contract::Type;

            switch (ctype) {
                case nym: {
                    const auto nym = Nym(contract.Payload());

                    return (nym && nym->ID() == id);
                }
                case notary: {
                    const auto notary = Server(contract.Payload());

                    return (notary->ID() == id);
                }
                case unit: {
                    const auto unit = UnitDefinition(contract.Payload());

                    return (unit->ID() == id);
                }
                case invalid:
                default: {
                    const auto error =
                        CString{"unsupported or unknown contract type: "}
                            .append(print(ctype));

                    throw std::runtime_error{error.c_str()};
                }
            }
        }();
        p2p_socket_.Send(
            [&] {
                auto out =
                    opentxs::network::zeromq::reply_to_message(std::move(msg));
                const auto reply =
                    factory::BlockchainSyncPublishContractReply(id, payload);
                reply.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Wallet::process_p2p_query_contract(
    opentxs::network::zeromq::Message&& msg) const noexcept -> void
{
    try {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        if (!base) {
            throw std::runtime_error{"failed to instantiate message"};
        }

        using Type = opentxs::network::otdht::MessageType;
        const auto type = base->Type();

        if (Type::contract_query != type) {
            const auto error = CString{}
                                   .append("Unsupported message type ")
                                   .append(print(type));

            throw std::runtime_error{error.c_str()};
        }

        auto payload = [&] {
            const auto& id = base->asQueryContract().ID();
            const auto ctype = translate(id.Type());

            try {
                using enum contract::Type;
                switch (ctype) {
                    case nym: {
                        const auto nymID =
                            api_.Factory().Internal().NymIDConvertSafe(id);
                        const auto nym = Nym(nymID);

                        if (!nym) {
                            throw std::runtime_error{
                                UnallocatedCString{"nym "} +
                                nymID.asBase58(api_.Crypto()) + " not found"};
                        }

                        return factory::BlockchainSyncQueryContractReply(*nym);
                    }
                    case notary: {
                        const auto notaryID =
                            api_.Factory().Internal().NotaryIDConvertSafe(id);

                        return factory::BlockchainSyncQueryContractReply(
                            Server(notaryID));
                    }
                    case unit: {
                        const auto unitID =
                            api_.Factory().Internal().UnitIDConvertSafe(id);

                        return factory::BlockchainSyncQueryContractReply(
                            UnitDefinition(unitID));
                    }
                    case invalid:
                    default: {
                        const auto error =
                            CString{"unsupported or unknown contract type: "}
                                .append(print(type));

                        throw std::runtime_error{error.c_str()};
                    }
                }
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return factory::BlockchainSyncQueryContractReply(id);
            }
        }();
        p2p_socket_.Send(
            [&] {
                auto out =
                    opentxs::network::zeromq::reply_to_message(std::move(msg));
                payload.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Wallet::process_p2p_response(
    opentxs::network::zeromq::Message&& msg) const noexcept -> void
{
    try {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        if (!base) {
            throw std::runtime_error{"failed to instantiate message"};
        }

        using Type = opentxs::network::otdht::MessageType;
        const auto type = base->Type();

        switch (type) {
            case Type::publish_ack: {
                const auto& contract = base->asPublishContractReply();
                const auto& id = contract.ID();
                const auto& log = LogVerbose();
                log("Contract ")(id, api_.Crypto())(" ");

                if (contract.Success()) {
                    log("successfully");
                } else {
                    log("not");
                }

                log(" published").Flush();
            } break;
            case Type::contract: {
                const auto& contract = base->asQueryContractReply();
                const auto& id = contract.ID();
                const auto& log = LogVerbose();
                const auto success = [&] {
                    const auto ctype = contract.ContractType();
                    using enum contract::Type;

                    switch (ctype) {
                        case nym: {
                            log("Nym");

                            if (!valid(contract.Payload())) { return false; }

                            const auto nym = Nym(contract.Payload());

                            return (nym && nym->ID() == id);
                        }
                        case notary: {
                            log("Notary contract");

                            if (!valid(contract.Payload())) { return false; }

                            const auto notary = Server(contract.Payload());

                            return (notary->ID() == id);
                        }
                        case unit: {
                            log("Unit definition");

                            if (!valid(contract.Payload())) { return false; }

                            const auto unit =
                                UnitDefinition(contract.Payload());

                            return (unit->ID() == id);
                        }
                        case invalid:
                        default: {
                            const auto error =
                                CString{
                                    "unsupported or unknown contract type: "}
                                    .append(print(ctype));

                            throw std::runtime_error{error.c_str()};
                        }
                    }
                }();
                log(" ")(id, api_.Crypto())(" ");

                if (success) {
                    log("successfully retrieved");
                } else {
                    log("not found on remote node");
                }

                log.Flush();
            } break;
            default:
                const auto error = CString{}
                                       .append("Unsupported message type ")
                                       .append(print(type));

                throw std::runtime_error{error.c_str()};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Wallet::PublishNotary(const identifier::Notary& id) const noexcept -> bool
{
    try {
        to_loopback_.modify_detach([notary = Server(id)](auto& socket) {
            const auto command = factory::BlockchainSyncPublishContract(notary);
            socket.Send(
                [&] {
                    auto out = opentxs::network::zeromq::Message{};
                    command.Serialize(out);

                    return out;
                }(),
                __FILE__,
                __LINE__);
        });

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Wallet::PublishNym(const identifier::Nym& id) const noexcept -> bool
{
    auto nym = Nym(id);

    if (!nym) {
        LogError()(OT_PRETTY_CLASS())("nym ")(id, api_.Crypto())(
            " does not exist")
            .Flush();

        return false;
    }

    to_loopback_.modify_detach([n = std::move(nym)](auto& socket) {
        const auto command = factory::BlockchainSyncPublishContract(*n);
        socket.Send(
            [&] {
                auto out = opentxs::network::zeromq::Message{};
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    });

    return true;
}

auto Wallet::PublishUnit(const identifier::UnitDefinition& id) const noexcept
    -> bool
{
    try {
        to_loopback_.modify_detach([unit = UnitDefinition(id)](auto& socket) {
            const auto command = factory::BlockchainSyncPublishContract(unit);
            socket.Send(
                [&] {
                    auto out = opentxs::network::zeromq::Message{};
                    command.Serialize(out);

                    return out;
                }(),
                __FILE__,
                __LINE__);
        });

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Wallet::purse(
    const identifier::Nym& nym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const bool checking) const -> PurseMap::mapped_type&
{
    const auto id = PurseID{nym, server, unit};
    auto lock = Lock{purse_lock_};
    auto& out = purse_map_[id];
    auto& [mutex, purse] = out;

    if (purse) { return out; }

    auto serialized = proto::Purse{};
    const auto loaded =
        api_.Storage().Load(nym, server, unit, serialized, checking);

    if (false == loaded) {
        if (false == checking) {
            LogError()(OT_PRETTY_CLASS())("Purse does not exist").Flush();
        }

        return out;
    }

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Invalid purse").Flush();

        return out;
    }

    purse = factory::Purse(api_, serialized);

    if (false == bool(purse)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate purse").Flush();
    }

    return out;
}

auto Wallet::Purse(
    const identifier::Nym& nym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const bool checking) const -> const otx::blind::Purse&
{
    return purse(nym, server, unit, checking).second;
}

auto Wallet::mutable_Purse(
    const identifier::Nym& nymID,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const PasswordPrompt& reason,
    const otx::blind::CashType type) const
    -> Editor<otx::blind::Purse, std::shared_mutex>
{
    auto& [mutex, purse] = this->purse(nymID, server, unit, true);

    if (!purse) {
        const auto nym = Nym(nymID);

        OT_ASSERT(nym);

        purse = factory::Purse(api_, *nym, server, unit, type, reason);
    }

    OT_ASSERT(purse);

    return {
        mutex,
        &purse,
        [this, nym = identifier::Nym{nymID}](
            auto* in, const auto& lock) -> void { this->save(lock, nym, in); }};
}

auto Wallet::RemoveServer(const identifier::Notary& id) const -> bool
{
    Lock mapLock(server_map_lock_);
    auto deleted = server_map_.erase(id);

    if (0 != deleted) {
        return api_.Storage().RemoveServer(id.asBase58(api_.Crypto()));
    }

    return false;
}

auto Wallet::RemoveUnitDefinition(const identifier::UnitDefinition& id) const
    -> bool
{
    Lock mapLock(unit_map_lock_);
    auto deleted = unit_map_.erase(id);

    if (0 != deleted) {
        return api_.Storage().RemoveUnitDefinition(id.asBase58(api_.Crypto()));
    }

    return false;
}

auto Wallet::publish_server(const identifier::Notary& id) const noexcept -> void
{
    server_publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::NotaryUpdated, true);
        work.AddFrame(id);

        return work;
    }());
}

auto Wallet::publish_unit(const identifier::UnitDefinition& id) const noexcept
    -> void
{
    unit_publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::UnitDefinitionUpdated, true);
        work.AddFrame(id);

        return work;
    }());
}

auto Wallet::reverse_unit_map(const UnitNameMap& map) -> Wallet::UnitNameReverse
{
    UnitNameReverse output{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}

void Wallet::save(
    const PasswordPrompt& reason,
    const UnallocatedCString id,
    std::unique_ptr<opentxs::Account>& in,
    eLock&,
    bool success) const
{
    OT_ASSERT(in);

    auto& account = *in;
    const auto accountID = api_.Factory().AccountIDFromBase58(id);

    if (false == success) {
        // Reload the last valid state for this Account.
        UnallocatedCString serialized{""};
        UnallocatedCString alias{""};
        const auto loaded = api_.Storage().Load(id, serialized, alias, false);

        OT_ASSERT(loaded);

        in.reset(account_factory(accountID, alias, serialized));

        OT_ASSERT(in);

        return;
    }

    const auto signerID = api_.Storage().AccountSigner(accountID);

    OT_ASSERT(false == signerID.empty());

    const auto signerNym = Nym(signerID);

    OT_ASSERT(signerNym);

    account.ReleaseSignatures();
    auto saved = account.SignContract(*signerNym, reason);

    OT_ASSERT(saved);

    saved = account.SaveContract();

    OT_ASSERT(saved);

    auto serialized = String::Factory();
    saved = in->SaveContractRaw(serialized);

    OT_ASSERT(saved);

    const auto contractID = api_.Storage().AccountContract(accountID);

    OT_ASSERT(false == contractID.empty());

    saved = api_.Storage().Store(
        accountID.asBase58(api_.Crypto()),
        serialized->Get(),
        in->Alias(),
        api_.Storage().AccountOwner(accountID),
        api_.Storage().AccountSigner(accountID),
        api_.Storage().AccountIssuer(accountID),
        api_.Storage().AccountServer(accountID),
        contractID,
        extract_unit(contractID));

    OT_ASSERT(saved);
}

void Wallet::save(
    const PasswordPrompt& reason,
    otx::context::internal::Base* context) const
{
    if (nullptr == context) { return; }

    Lock lock(context->GetLock());
    const bool sig = context->UpdateSignature(lock, reason);

    OT_ASSERT(sig);
    OT_ASSERT(context->ValidateContext(lock));

    api_.Storage().Store(context->GetContract(lock));
}

void Wallet::save(const Lock& lock, otx::client::Issuer* in) const
{
    OT_ASSERT(nullptr != in);
    OT_ASSERT(lock.owns_lock());

    const auto& nymID = in->LocalNymID();
    const auto& issuerID = in->IssuerID();
    auto serialized = proto::Issuer{};
    auto loaded = in->Serialize(serialized);

    OT_ASSERT(loaded);

    api_.Storage().Store(nymID, serialized);
    issuer_publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::IssuerUpdated, true);
        work.AddFrame(nymID);
        work.AddFrame(issuerID);

        return work;
    }());
}

void Wallet::save(
    const eLock& lock,
    const identifier::Nym nym,
    otx::blind::Purse* in) const
{
    OT_ASSERT(nullptr != in);
    OT_ASSERT(lock.owns_lock());

    auto& purse = *in;

    if (!purse) { OT_FAIL; }

    const auto serialized = [&] {
        auto proto = proto::Purse{};
        purse.Internal().Serialize(proto);

        return proto;
    }();

    OT_ASSERT(proto::Validate(serialized, VERBOSE));

    const auto stored = api_.Storage().Store(nym, serialized);

    OT_ASSERT(stored);
}

void Wallet::save(NymData* nymData, const Lock& lock) const
{
    OT_ASSERT(nullptr != nymData);
    OT_ASSERT(lock.owns_lock());

    SaveCredentialIDs(nymData->nym());
    notify_changed(nymData->nym().ID());
}

void Wallet::save(
    const PasswordPrompt& reason,
    opentxs::NymFile* nymfile,
    const Lock& lock) const
{
    OT_ASSERT(nullptr != nymfile);
    OT_ASSERT(lock.owns_lock());

    auto* internal = dynamic_cast<opentxs::internal::NymFile*>(nymfile);

    OT_ASSERT(nullptr != internal);

    const auto saved = internal->SaveSignedNymFile(reason);

    OT_ASSERT(saved);
}

auto Wallet::SaveCredentialIDs(const identity::Nym& nym) const -> bool
{
    auto index = proto::Nym{};
    if (false == dynamic_cast<const identity::internal::Nym&>(nym)
                     .SerializeCredentialIndex(
                         index, identity::internal::Nym::Mode::Abbreviated)) {
        return false;
    }
    const bool valid = proto::Validate(index, VERBOSE);

    if (!valid) { return false; }

    if (!api_.Storage().Store(index, nym.Alias())) {
        LogError()(OT_PRETTY_CLASS())(
            "Failure trying to store credential list for Nym: ")(
            nym.ID(), api_.Crypto())
            .Flush();

        return false;
    }

    LogDetail()(OT_PRETTY_CLASS())("Credentials saved.").Flush();

    return true;
}

auto Wallet::search_notary(const identifier::Notary& id) const noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(
        "Searching remote networks for unknown notary ")(id, api_.Crypto())
        .Flush();
    to_loopback_.modify_detach([id](auto& socket) {
        const auto command = factory::BlockchainSyncQueryContract(id);
        socket.Send(
            [&] {
                auto out = opentxs::network::zeromq::Message{};
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    });
}

auto Wallet::search_nym(const identifier::Nym& id) const noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(
        "Searching remote networks for unknown nym ")(id, api_.Crypto())
        .Flush();
    to_loopback_.modify_detach([id](auto& socket) {
        const auto command = factory::BlockchainSyncQueryContract(id);
        socket.Send(
            [&] {
                auto out = opentxs::network::zeromq::Message{};
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    });
}

auto Wallet::search_unit(const identifier::UnitDefinition& id) const noexcept
    -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(
        "Searching remote networks for unknown unit definition ")(
        id, api_.Crypto())
        .Flush();
    to_loopback_.modify_detach([id](auto& socket) {
        const auto command = factory::BlockchainSyncQueryContract(id);
        socket.Send(
            [&] {
                auto out = opentxs::network::zeromq::Message{};
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    });
}

auto Wallet::SetDefaultNym(const identifier::Nym& id) const noexcept -> bool
{
    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid id").Flush();

        return false;
    }

    if (0u == LocalNyms().count(id)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(id, api_.Crypto())(
            " is not local")
            .Flush();

        return false;
    }

    auto out = api_.Storage().SetDefaultNym(id);

    if (out) { notify_changed(id); }

    return out;
}

auto Wallet::SetNymAlias(
    const identifier::Nym& id,
    const UnallocatedCString& alias) const -> bool
{
    auto mapLock = Lock{nym_map_lock_};
    auto& nym = nym_map_[id].second;
    if (nullptr != nym) { nym->SetAlias(alias); }

    return api_.Storage().SetNymAlias(id, alias);
}

auto Wallet::Server(
    const identifier::Notary& id,
    const std::chrono::milliseconds& timeout) const -> OTServerContract
{
    if (blockchain::Type::UnknownBlockchain != blockchain::Chain(api_, id)) {
        throw std::runtime_error{"Attempting to load a blockchain as a notary"};
    }

    if (id.empty()) {
        throw std::runtime_error{"Attempting to load a null notary contract"};
    }

    Lock mapLock(server_map_lock_);
    bool inMap = (server_map_.find(id) != server_map_.end());
    bool valid = false;

    if (!inMap) {
        auto serialized = proto::ServerContract{};
        auto alias = UnallocatedCString{};
        bool loaded = api_.Storage().Load(id, serialized, alias, true);

        if (loaded) {
            auto nym = Nym(api_.Factory().NymIDFromBase58(serialized.nymid()));

            if (!nym && serialized.has_publicnym()) {
                nym = Nym(serialized.publicnym());
            }

            if (nym) {
                auto& pServer = server_map_[id];
                pServer =
                    opentxs::Factory::ServerContract(api_, nym, serialized);

                if (pServer) {
                    valid = true;  // Factory() performs validation
                    pServer->InitAlias(alias);
                } else {
                    server_map_.erase(id);
                }
            }
        } else {
            search_notary(id);

            if (timeout > 0ms) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = 100ms;

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (server_map_.find(id) != server_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return Server(id);  // timeout of zero prevents infinite
                                    // recursion
            }
        }
    } else {
        auto& pServer = server_map_[id];
        if (pServer) { valid = pServer->Validate(); }
    }

    if (valid) { return OTServerContract{server_map_[id]}; }

    throw std::runtime_error("Server contract not found");
}

auto Wallet::server(std::unique_ptr<contract::Server> contract) const
    noexcept(false) -> OTServerContract
{
    if (false == bool(contract)) {
        throw std::runtime_error("Null server contract");
    }

    if (false == contract->Validate()) {
        throw std::runtime_error("Invalid server contract");
    }

    const auto id = [&] {
        const auto generic = contract->ID();
        auto output = identifier::Notary{};
        output.Assign(generic);

        return output;
    }();

    OT_ASSERT(false == id.empty());
    OT_ASSERT(contract->Alias() == contract->EffectiveName());

    auto serialized = proto::ServerContract{};

    if (false == contract->Serialize(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize contract.").Flush();
    }

    if (api_.Storage().Store(serialized, contract->Alias())) {
        {
            Lock mapLock(server_map_lock_);
            server_map_[id].reset(contract.release());
        }

        publish_server(id);
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save server contract.")
            .Flush();
    }

    return Server(id);
}

auto Wallet::Server(const proto::ServerContract& contract) const
    -> OTServerContract
{
    if (false == proto::Validate(contract, VERBOSE)) {
        throw std::runtime_error("Invalid serialized server contract");
    }

    const auto serverID = api_.Factory().NotaryIDFromBase58(contract.id());

    if (serverID.empty()) {
        throw std::runtime_error(
            "Attempting to load notary contract with empty notary ID");
    }

    const auto nymID = api_.Factory().NymIDFromBase58(contract.nymid());

    if (nymID.empty()) {
        throw std::runtime_error(
            "Attempting to load notary contract with empty nym ID");
    }

    find_nym_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::OTXSearchNym, true);
        work.AddFrame(nymID);

        return work;
    }());
    auto nym = Nym(nymID);

    if (!nym && contract.has_publicnym()) { nym = Nym(contract.publicnym()); }

    if (!nym) { throw std::runtime_error("Unable to load notary nym"); }

    auto candidate = std::unique_ptr<contract::Server>{
        opentxs::Factory::ServerContract(api_, nym, contract)};

    if (!candidate) {
        throw std::runtime_error("Failed to instantiate contract");
    }

    if (false == candidate->Validate()) {
        throw std::runtime_error("Invalid contract");
    }

    if (serverID != candidate->ID()) {
        throw std::runtime_error("Wrong contract ID");
    }

    auto serialized = proto::ServerContract{};

    if (false == candidate->Serialize(serialized)) {
        throw std::runtime_error("Failed to serialize server contract");
    }

    const auto stored =
        api_.Storage().Store(serialized, candidate->EffectiveName());

    if (false == stored) {
        throw std::runtime_error("Failed to save server contract");
    }

    {
        Lock mapLock(server_map_lock_);
        server_map_[serverID].reset(candidate.release());
    }

    publish_server(serverID);

    return Server(serverID);
}

auto Wallet::Server(const ReadView& contract) const -> OTServerContract
{
    return Server(opentxs::proto::Factory<proto::ServerContract>(contract));
}

auto Wallet::Server(
    const UnallocatedCString& nymid,
    const UnallocatedCString& name,
    const UnallocatedCString& terms,
    const UnallocatedList<contract::Server::Endpoint>& endpoints,
    const opentxs::PasswordPrompt& reason,
    const VersionNumber version) const -> OTServerContract
{
    auto nym = Nym(api_.Factory().NymIDFromBase58(nymid));

    if (nym) {
        auto list = UnallocatedList<Endpoint>{};
        std::transform(
            std::begin(endpoints),
            std::end(endpoints),
            std::back_inserter(list),
            [](const auto& in) -> Endpoint {
                return {
                    static_cast<int>(std::get<0>(in)),
                    static_cast<int>(std::get<1>(in)),
                    std::get<2>(in),
                    std::get<3>(in),
                    std::get<4>(in)};
            });
        auto pContract =
            std::unique_ptr<contract::Server>{opentxs::Factory::ServerContract(
                api_, nym, list, terms, name, version, reason)};

        if (pContract) {

            return this->server(std::move(pContract));
        } else {
            LogError()(OT_PRETTY_CLASS())(" Error: Failed to create contract.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Nym does not exist.").Flush();
    }

    return Server(identifier::Notary{});
}

auto Wallet::ServerList() const -> ObjectList
{
    return api_.Storage().ServerList();
}

auto Wallet::server_to_nym(identifier::Generic& input) const -> identifier::Nym
{
    auto output = identifier::Nym{};
    output.Assign(input);
    const auto inputIsNymID = [&] {
        auto nym = Nym(output);

        return nym.operator bool();
    }();

    if (inputIsNymID) {
        const auto list = ServerList();
        std::size_t matches = 0;

        for (const auto& [serverID, alias] : list) {
            try {
                const auto id = api_.Factory().NotaryIDFromBase58(serverID);
                auto server = Server(id);

                if (server->Nym()->ID() == input) {
                    matches++;
                    // set input to the notary ID
                    input.Assign(server->ID());
                }
            } catch (...) {
            }
        }

        OT_ASSERT(2 > matches);
    } else {
        output.clear();

        try {
            const auto notaryID = [&] {
                auto out = identifier::Notary{};
                out.Assign(input);

                return out;
            }();
            const auto contract = Server(notaryID);
            output = contract->Nym()->ID();
        } catch (...) {
            LogDetail()(OT_PRETTY_CLASS())("Non-existent server: ")(
                input, api_.Crypto())
                .Flush();
        }
    }

    return output;
}

auto Wallet::SetServerAlias(
    const identifier::Notary& id,
    const UnallocatedCString& alias) const -> bool
{
    const bool saved = api_.Storage().SetServerAlias(id, alias);

    if (saved) {
        {
            Lock mapLock(server_map_lock_);
            server_map_.erase(id);
        }

        publish_server(id);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save server contract ")(
            id, api_.Crypto())
            .Flush();
    }

    return false;
}

auto Wallet::SetUnitDefinitionAlias(
    const identifier::UnitDefinition& id,
    const UnallocatedCString& alias) const -> bool
{
    const bool saved = api_.Storage().SetUnitDefinitionAlias(id, alias);

    if (saved) {
        {
            Lock mapLock(unit_map_lock_);
            unit_map_.erase(id);
        }

        publish_unit(id);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save unit definition ")(
            id, api_.Crypto())
            .Flush();
    }

    return false;
}

auto Wallet::UnitDefinitionList() const -> ObjectList
{
    return api_.Storage().UnitDefinitionList();
}

auto Wallet::UnitDefinition(
    const identifier::UnitDefinition& id,
    const std::chrono::milliseconds& timeout) const -> OTUnitDefinition
{
    if (blockchain::Type::UnknownBlockchain != blockchain::Chain(api_, id)) {
        throw std::runtime_error{
            "Attempting to load a blockchain as a unit definition"};
    }

    if (id.empty()) {
        throw std::runtime_error{"Attempting to load a null unit definition"};
    }

    Lock mapLock(unit_map_lock_);
    bool inMap = (unit_map_.find(id) != unit_map_.end());
    bool valid = false;

    if (!inMap) {
        auto serialized = proto::UnitDefinition{};
        UnallocatedCString alias;
        bool loaded = api_.Storage().Load(id, serialized, alias, true);

        if (loaded) {
            auto nym = Nym(api_.Factory().NymIDFromBase58(serialized.issuer()));

            if (!nym && serialized.has_issuer_nym()) {
                nym = Nym(serialized.issuer_nym());
            }

            if (nym) {
                auto& pUnit = unit_map_[id];
                pUnit = opentxs::Factory::UnitDefinition(api_, nym, serialized);

                if (pUnit) {
                    valid = true;  // Factory() performs validation
                    pUnit->InitAlias(alias);
                } else {
                    unit_map_.erase(id);
                }
            }
        } else {
            search_unit(id);

            if (timeout > 0ms) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = 100ms;

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (unit_map_.find(id) != unit_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return UnitDefinition(id);  // timeout of zero prevents infinite
                                            // recursion
            }
        }
    } else {
        auto& pUnit = unit_map_[id];
        if (pUnit) { valid = pUnit->Validate(); }
    }

    if (valid) { return OTUnitDefinition{unit_map_[id]}; }

    throw std::runtime_error("Unit definition does not exist");
}

auto Wallet::unit_definition(std::shared_ptr<contract::Unit>&& contract) const
    -> OTUnitDefinition
{
    if (false == bool(contract)) {
        throw std::runtime_error("Null unit definition contract");
    }

    if (false == contract->Validate()) {
        throw std::runtime_error("Invalid unit definition contract");
    }

    const auto id = [&] {
        auto out = identifier::UnitDefinition{};
        out.Assign(contract->ID());

        return out;
    }();

    auto serialized = proto::UnitDefinition{};

    if (false == contract->Serialize(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize unit definition")
            .Flush();
    }

    if (api_.Storage().Store(serialized, contract->Alias())) {
        {
            Lock mapLock(unit_map_lock_);
            auto it = unit_map_.find(id);

            if (unit_map_.end() == it) {
                unit_map_.emplace(id, std::move(contract));
            } else {
                it->second = std::move(contract);
            }
        }

        publish_unit(id);
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save unit definition").Flush();
    }

    return UnitDefinition(id);
}

auto Wallet::UnitDefinition(const proto::UnitDefinition& contract) const
    -> OTUnitDefinition
{
    if (false == proto::Validate(contract, VERBOSE)) {
        throw std::runtime_error("Invalid serialized unit definition");
    }

    const auto unitID = api_.Factory().UnitIDFromBase58(contract.id());

    if (unitID.empty()) {
        throw std::runtime_error("Invalid unit definition id");
    }

    const auto nymID = api_.Factory().NymIDFromBase58(contract.issuer());

    if (nymID.empty()) { throw std::runtime_error("Invalid nym ID"); }

    find_nym_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::OTXSearchNym, true);
        work.AddFrame(nymID);

        return work;
    }());
    auto nym = Nym(nymID);

    if (!nym && contract.has_issuer_nym()) { nym = Nym(contract.issuer_nym()); }

    if (!nym) { throw std::runtime_error("Invalid nym"); }

    auto candidate = opentxs::Factory::UnitDefinition(api_, nym, contract);

    if (!candidate) {
        throw std::runtime_error("Failed to instantiate contract");
    }

    if (false == candidate->Validate()) {
        throw std::runtime_error("Invalid contract");
    }

    if (unitID != candidate->ID()) {
        throw std::runtime_error("Wrong contract ID");
    }

    auto serialized = proto::UnitDefinition{};

    if (false == candidate->Serialize(serialized)) {
        throw std::runtime_error("Failed to serialize unit definition");
    }

    const auto stored = api_.Storage().Store(serialized, candidate->Alias());

    if (false == stored) {
        throw std::runtime_error("Failed to save unit definition");
    }

    {
        Lock mapLock(unit_map_lock_);
        unit_map_[unitID] = candidate;
    }

    publish_unit(unitID);

    return UnitDefinition(unitID);
}

auto Wallet::UnitDefinition(const ReadView contract) const -> OTUnitDefinition
{
    return UnitDefinition(
        opentxs::proto::Factory<proto::UnitDefinition>(contract));
}

auto Wallet::CurrencyContract(
    const UnallocatedCString& nymid,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const Amount& redemptionIncrement,
    const PasswordPrompt& reason) const -> OTUnitDefinition
{
    return CurrencyContract(
        nymid,
        shortname,
        terms,
        unitOfAccount,
        redemptionIncrement,
        display::GetDefinition(unitOfAccount),
        contract::Unit::DefaultVersion,
        reason);
}

auto Wallet::CurrencyContract(
    const UnallocatedCString& nymid,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const Amount& redemptionIncrement,
    const display::Definition& displayDefinition,
    const PasswordPrompt& reason) const -> OTUnitDefinition
{
    return CurrencyContract(
        nymid,
        shortname,
        terms,
        unitOfAccount,
        redemptionIncrement,
        displayDefinition,
        contract::Unit::DefaultVersion,
        reason);
}

auto Wallet::CurrencyContract(
    const UnallocatedCString& nymid,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const Amount& redemptionIncrement,
    const VersionNumber version,
    const PasswordPrompt& reason) const -> OTUnitDefinition
{
    return CurrencyContract(
        nymid,
        shortname,
        terms,
        unitOfAccount,
        redemptionIncrement,
        display::GetDefinition(unitOfAccount),
        version,
        reason);
}

auto Wallet::CurrencyContract(
    const UnallocatedCString& nymid,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const Amount& redemptionIncrement,
    const display::Definition& displayDefinition,
    const VersionNumber version,
    const PasswordPrompt& reason) const -> OTUnitDefinition
{
    auto unit = UnallocatedCString{};
    auto nym = Nym(api_.Factory().NymIDFromBase58(nymid));

    if (nym) {
        auto contract = opentxs::Factory::CurrencyContract(
            api_,
            nym,
            shortname,
            terms,
            unitOfAccount,
            version,
            reason,
            displayDefinition,
            redemptionIncrement);

        if (contract) {

            return unit_definition(std::move(contract));
        } else {
            LogError()(OT_PRETTY_CLASS())(" Error: Failed to create contract.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Nym does not exist.").Flush();
    }

    return UnitDefinition(api_.Factory().UnitIDFromBase58(unit));
}

auto Wallet::SecurityContract(
    const UnallocatedCString& nymid,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement,
    const VersionNumber version) const -> OTUnitDefinition
{
    UnallocatedCString unit;
    auto nym = Nym(api_.Factory().NymIDFromBase58(nymid));

    if (nym) {
        auto contract = opentxs::Factory::SecurityContract(
            api_,
            nym,
            shortname,
            terms,
            unitOfAccount,
            version,
            reason,
            displayDefinition,
            redemptionIncrement);

        if (contract) {

            return unit_definition(std::move(contract));
        } else {
            LogError()(OT_PRETTY_CLASS())(" Error: Failed to create contract.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Nym does not exist.").Flush();
    }

    return UnitDefinition(api_.Factory().UnitIDFromBase58(unit));
}

auto Wallet::LoadCredential(
    const UnallocatedCString& id,
    std::shared_ptr<proto::Credential>& credential) const -> bool
{
    if (false == bool(credential)) {
        credential = std::make_shared<proto::Credential>();
    }

    OT_ASSERT(credential);

    return api_.Storage().Load(id, *credential);
}

auto Wallet::SaveCredential(const proto::Credential& credential) const -> bool
{
    return api_.Storage().Store(credential);
}

Wallet::~Wallet() { handle_.Release(); }
}  // namespace opentxs::api::session::imp
