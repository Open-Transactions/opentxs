// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"

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
}  // namespace blockchain

namespace identifier
{
class Account;
}  // namespace identifier

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct PaymentCode : virtual public crypto::PaymentCode,
                     virtual public Deterministic {
    static auto GetID(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote) noexcept -> identifier::Account;

    virtual auto AddIncomingNotification(
        const block::TransactionHash& tx) const noexcept -> bool = 0;
    virtual auto AddNotification(
        const block::TransactionHash& tx) const noexcept -> bool = 0;
    virtual auto ReorgNotification(
        const block::TransactionHash& tx) const noexcept -> bool = 0;
};
}  // namespace opentxs::blockchain::crypto::internal
