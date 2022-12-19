// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Block;
}  // namespace block

namespace node
{
namespace internal
{
class Manager;
}  // namespace internal

class BlockOracle;
class FilterOracle;
class HeaderOracle;
class Wallet;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT Manager
{
public:
    using PendingOutgoing = std::future<SendOutcome>;

    virtual auto AddBlock(const block::Block& block) const noexcept -> bool = 0;
    virtual auto AddPeer(
        const network::blockchain::Address& address) const noexcept -> bool = 0;
    virtual auto BlockOracle() const noexcept -> const node::BlockOracle& = 0;
    virtual auto FilterOracle() const noexcept -> const node::FilterOracle& = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetConfirmations(const UnallocatedCString& txid) const noexcept
        -> block::Height = 0;
    virtual auto GetType() const noexcept -> Type = 0;
    virtual auto HeaderOracle() const noexcept -> const node::HeaderOracle& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Manager& = 0;
    virtual auto Listen(
        const network::blockchain::Address& address) const noexcept -> bool = 0;
    virtual auto Profile() const noexcept -> BlockchainProfile = 0;
    virtual auto SendToAddress(
        const identifier::Nym& sender,
        const UnallocatedCString& address,
        const Amount amount,
        const UnallocatedCString& memo = {}) const noexcept
        -> PendingOutgoing = 0;
    virtual auto SendToPaymentCode(
        const identifier::Nym& sender,
        const UnallocatedCString& recipient,
        const Amount amount,
        const UnallocatedCString& memo = {}) const noexcept
        -> PendingOutgoing = 0;
    virtual auto SendToPaymentCode(
        const identifier::Nym& sender,
        const PaymentCode& recipient,
        const Amount amount,
        const UnallocatedCString& memo = {}) const noexcept
        -> PendingOutgoing = 0;
    virtual auto Wallet() const noexcept -> const node::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Manager& = 0;

    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    auto operator=(const Manager&) -> Manager& = delete;
    auto operator=(Manager&&) -> Manager& = delete;

    virtual ~Manager() = default;

protected:
    Manager() noexcept = default;
};
}  // namespace opentxs::blockchain::node
