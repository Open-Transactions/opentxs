// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <tuple>

#include "opentxs/blockchain/crypto/Account.hpp"
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

namespace crypto
{
struct Notifications;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Account : virtual public crypto::Account {
    virtual auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto ClaimAccountID(
        const identifier::Account& id,
        crypto::Subaccount* node) const noexcept -> void = 0;
    virtual auto FindNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto Get(Notifications& out) const noexcept -> void = 0;
    virtual auto LookupUTXO(const Coin& coin) const noexcept
        -> std::optional<std::pair<Key, Amount>> = 0;

    virtual auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const block::TransactionHash& notification,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;
    virtual auto Startup() noexcept -> void = 0;

    ~Account() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
