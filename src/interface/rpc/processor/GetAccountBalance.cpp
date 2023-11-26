// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/core/Core.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/interface/rpc/AccountData.hpp"
#include "opentxs/interface/rpc/AccountType.hpp"   // IWYU pragma: keep
#include "opentxs/interface/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/interface/rpc/request/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/interface/rpc/response/GetAccountBalance.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::get_account_balance(const request::Base& base) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asGetAccountBalance();
    auto codes = response::Base::Responses{};
    auto balances = response::GetAccountBalance::Data{};
    const auto reply = [&] {
        return std::make_unique<response::GetAccountBalance>(
            in, std::move(codes), std::move(balances));
    };

    try {
        const auto& api = session(base);

        for (const auto& id : in.Accounts()) {
            const auto index = codes.size();

            if (id.empty()) {
                codes.emplace_back(index, ResponseCode::invalid);

                continue;
            }

            const auto accountID = api.Factory().AccountIDFromBase58(id);

            if (is_blockchain_account(base, accountID)) {
                get_account_balance_blockchain(
                    base, index, accountID, balances, codes);
            } else {
                get_account_balance_custodial(
                    api, index, accountID, balances, codes);
            }
        }
    } catch (...) {
        codes.emplace_back(0, ResponseCode::bad_session);
    }

    return reply();
}

auto RPC::get_account_balance_blockchain(
    const request::Base& base,
    const std::size_t index,
    const identifier::Account& accountID,
    UnallocatedVector<AccountData>& balances,
    response::Base::Responses& codes) const noexcept -> void
{
    try {
        const auto& api = client_session(base);
        const auto& blockchain = api.Crypto().Blockchain();
        const auto [chain, owner] = blockchain.LookupAccount(accountID);
        api.Network().Blockchain().Start(chain);
        const auto handle = api.Network().Blockchain().GetChain(chain);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& client = handle.get();
        const auto [confirmed, unconfirmed] = client.GetBalance(owner);
        const auto& definition =
            display::GetDefinition(blockchain_to_unit(chain));
        balances.emplace_back(
            accountID.asBase58(api.Crypto()),
            blockchain::AccountName(chain),
            blockchain::UnitID(api, chain).asBase58(api.Crypto()),
            owner.asBase58(api.Crypto()),
            blockchain::IssuerID(api, chain).asBase58(api.Crypto()),
            definition.Format(confirmed),
            definition.Format(unconfirmed),
            confirmed,
            unconfirmed,
            AccountType::blockchain);
        codes.emplace_back(index, ResponseCode::success);
    } catch (...) {
        codes.emplace_back(index, ResponseCode::account_not_found);
    }
}

auto RPC::get_account_balance_custodial(
    const api::Session& api,
    const std::size_t index,
    const identifier::Account& accountID,
    UnallocatedVector<AccountData>& balances,
    response::Base::Responses& codes) const noexcept -> void
{
    const auto account = api.Wallet().Internal().Account(accountID);

    if (account) {
        const auto& unit = account.get().GetInstrumentDefinitionID();
        const auto balance = account.get().GetBalance();
        const auto formatted = [&] {
            const auto& blockchain = api.Crypto().Blockchain();
            const auto [chain, owner] = blockchain.LookupAccount(accountID);
            const auto& definition =
                display::GetDefinition(blockchain_to_unit(chain));
            return definition.Format(balance);
        }();
        balances.emplace_back(
            accountID.asBase58(api.Crypto()),
            account.get().Alias(),
            unit.asBase58(api.Crypto()),
            api.Storage().Internal().AccountOwner(accountID).asBase58(
                api.Crypto()),
            api.Storage().Internal().AccountIssuer(accountID).asBase58(
                api.Crypto()),
            formatted,
            formatted,
            balance,
            balance,
            (account.get().IsIssuer()) ? AccountType::issuer
                                       : AccountType::normal);
        codes.emplace_back(index, ResponseCode::success);
    } else {
        codes.emplace_back(index, ResponseCode::account_not_found);
    }
}
}  // namespace opentxs::rpc::implementation
