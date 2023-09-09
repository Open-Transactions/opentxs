// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <tuple>

#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace node
{
namespace internal
{
struct SpendPolicy;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
class BlockchainTransactionProposal;
}  // namespace proto

class Amount;
class Log;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Spend
{
public:
    using ContactID = identifier::Generic;
    using Address = ByteArray;
    using AddressRecipient =
        std::tuple<ContactID, Amount, crypto::AddressStyle, Address>;
    using Pubkey = ByteArray;
    using PaymentCodeRecipient =
        std::tuple<ContactID, Amount, PaymentCode, crypto::Key, Pubkey>;

    virtual auto AddressRecipients() const noexcept
        -> std::span<const AddressRecipient>;
    virtual auto AddNotification(
        const block::TransactionHash& txid) const noexcept -> void;
    virtual auto IsExpired() const noexcept -> bool;
    virtual auto OutgoingKeys() const noexcept -> Set<crypto::Key>;
    virtual auto PasswordPrompt() const noexcept -> std::string_view;
    virtual auto PaymentCodeRecipients() const noexcept
        -> std::span<const PaymentCodeRecipient>;
    virtual auto Policy() const noexcept -> SpendPolicy;
    [[nodiscard]] virtual auto Serialize(
        proto::BlockchainTransactionProposal& out) const noexcept -> bool;

    virtual auto Add(const proto::BlockchainTransaction& tx) noexcept -> void;
    [[nodiscard]] virtual auto Check() noexcept -> std::optional<SendResult>;
    virtual auto Finalize(const Log& log, alloc::Strategy alloc) noexcept(false)
        -> void;

    Spend() noexcept = default;
    Spend(const Spend&) = delete;
    Spend(Spend&&) = delete;
    auto operator=(const Spend&) -> Spend& = delete;
    auto operator=(Spend&&) -> Spend& = delete;

    virtual ~Spend() = default;
};
}  // namespace opentxs::blockchain::node::internal
