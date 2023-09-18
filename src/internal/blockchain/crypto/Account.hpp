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
namespace api
{
class Session;
}  // namespace api

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

namespace identifier
{
class Account;
class Nym;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Account : virtual public crypto::Account {
    static auto GetID(
        const api::Session& api,
        const identifier::Nym& owner,
        blockchain::Type chain) noexcept -> identifier::Account;

    [[nodiscard]] virtual auto ClaimAccountID(
        const identifier::Account& id,
        bool existing,
        crypto::Subaccount* node) const noexcept -> bool = 0;
    virtual auto FindNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto Get(Notifications& out) const noexcept -> void = 0;

    virtual auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;
    virtual auto AddOrUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;
    virtual auto Startup() noexcept -> void = 0;
    using crypto::Account::Subaccount;
    virtual auto Subaccount(const identifier::Account& id) noexcept(false)
        -> crypto::Subaccount& = 0;

    ~Account() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
