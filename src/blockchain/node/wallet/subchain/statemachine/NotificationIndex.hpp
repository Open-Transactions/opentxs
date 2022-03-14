// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class NotificationIndex final : public Index::Imp
{
public:
    NotificationIndex(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const PaymentCode& code,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    NotificationIndex() = delete;
    NotificationIndex(const Imp&) = delete;
    NotificationIndex(NotificationIndex&&) = delete;
    NotificationIndex& operator=(const NotificationIndex&) = delete;
    NotificationIndex& operator=(NotificationIndex&&) = delete;

    ~NotificationIndex() final = default;

private:
    const PaymentCode code_;
    const CString pc_display_;

    auto need_index(const std::optional<Bip32Index>& current) const noexcept
        -> std::optional<Bip32Index> final;

    auto process(
        const std::optional<Bip32Index>& current,
        Bip32Index target) noexcept -> void final;
};
}  // namespace opentxs::blockchain::node::wallet
