// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "interface/qt/SendMonitor.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui::implementation
{
auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const UnallocatedCString& input,
    const UnallocatedCString& memo,
    Scale scale,
    SendMonitor::Callback cb) const noexcept -> int
{
    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& network = handle.get();
        const auto recipient = api_.Factory().PaymentCode(address);
        const auto& definition =
            display::GetDefinition(BlockchainToUnit(chain_));
        const auto amount = definition.Import(input, scale);

        if (0 < recipient.Version()) {

            return SendMonitor().watch(
                network.SendToPaymentCode(primary_id_, recipient, amount, memo),
                std::move(cb));
        } else {

            return SendMonitor().watch(
                network.SendToAddress(primary_id_, address, amount, memo),
                std::move(cb));
        }
    } catch (...) {

        return -1;
    }
}
}  // namespace opentxs::ui::implementation
