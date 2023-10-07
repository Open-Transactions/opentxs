// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated

#include <chrono>
#include <stdexcept>
#include <utility>

#include "interface/qt/SendMonitor.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Funding.hpp"           // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::implementation
{
using namespace std::literals;

auto BlockchainAccountActivity::Notify(
    std::span<const PaymentCode> contacts,
    SendMonitor::Callback cb) const noexcept -> int
{
    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& wallet = handle.get().Wallet();
        auto spend = wallet.CreateSpend(primary_id_);

        if (false == spend.SetSweepFromAccount(true)) {
            throw std::runtime_error{"failed to set funding policy"};
        }

        if (false == spend.Notify(contacts)) {
            throw std::runtime_error{"failed to set notifications"};
        }

        return SendMonitor().watch(wallet.Execute(spend), std::move(cb));
    } catch (...) {

        return -1;
    }
}
auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const UnallocatedCString& input,
    const std::string_view memo,
    Scale scale,
    SendMonitor::Callback cb,
    std::span<const PaymentCode> notify) const noexcept -> int
{
    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& wallet = handle.get().Wallet();
        const auto recipient = api_.Factory().PaymentCodeFromBase58(address);
        const auto& definition =
            display::GetDefinition(blockchain_to_unit(chain_));
        const auto amount = definition.Import(input, scale);

        if (false == amount.has_value()) {
            throw std::runtime_error{"invalid amount"};
        }

        auto spend = wallet.CreateSpend(primary_id_);

        if (false == spend.SetMemo(memo)) {

            throw std::runtime_error{"failed set memo"};
        }

        if (false == spend.Notify(notify)) {
            throw std::runtime_error{"failed to set notifications"};
        }

        if (0 < recipient.Version()) {
            if (false == spend.SendToPaymentCode(recipient, *amount)) {
                throw std::runtime_error{
                    "failed to set recipient payment code"};
            }
        } else {
            if (false == spend.SendToAddress(address, *amount)) {
                throw std::runtime_error{"failed to set recipient address"};
            }
        }

        return SendMonitor().watch(wallet.Execute(spend), std::move(cb));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return -1;
    }
}
}  // namespace opentxs::ui::implementation
