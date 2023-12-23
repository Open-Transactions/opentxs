// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Spend.hpp"  // IWYU pragma: associated

#include "internal/blockchain/node/SpendPolicy.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Account.hpp"

namespace opentxs::blockchain::node::internal
{
auto Spend::Add(const proto::BlockchainTransaction&) noexcept -> void {}

auto Spend::AddNotification(const block::TransactionHash&) const noexcept
    -> void
{
}

auto Spend::AddressRecipients() const noexcept
    -> std::span<const AddressRecipient>
{
    return {};
}

auto Spend::Check() noexcept -> std::optional<SendResult>
{
    return SendResult::UnspecifiedError;
}

auto Spend::Finalize(const Log&, alloc::Strategy) noexcept(false) -> void {}

auto Spend::IsExpired() const noexcept -> bool { return true; }

auto Spend::OutgoingKeys() const noexcept -> Set<crypto::Key> { return {}; }

auto Spend::PasswordPrompt() const noexcept -> std::string_view { return {}; }

auto Spend::PaymentCodeRecipients() const noexcept
    -> std::span<const PaymentCodeRecipient>
{
    return {};
}

auto Spend::Policy() const noexcept -> SpendPolicy { return {}; }

auto Spend::Serialize(proto::BlockchainTransactionProposal&) const noexcept
    -> bool
{
    return false;
}
}  // namespace opentxs::blockchain::node::internal
