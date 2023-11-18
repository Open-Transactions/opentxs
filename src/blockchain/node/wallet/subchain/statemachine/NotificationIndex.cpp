// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/NotificationIndex.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <iterator>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"  // IWYU pragma: keep
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
auto Index::NotificationFactory(
    const std::shared_ptr<const SubchainStateData>& parent,
    const PaymentCode& code) noexcept -> Index
{
    const auto& asio = parent->api_.Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();

    return Index{std::allocate_shared<NotificationIndex>(
        alloc::PMR<NotificationIndex>{asio.Alloc(batchID)},
        parent,
        code,
        batchID)};
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
NotificationIndex::NotificationIndex(
    const std::shared_ptr<const SubchainStateData>& parent,
    const PaymentCode& code,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(parent, batch, alloc)
    , code_(code)
    , pc_display_(code_.asBase58(), alloc)
{
}

auto NotificationIndex::need_index(const std::optional<Bip32Index>& current)
    const noexcept -> std::optional<Bip32Index>
{
    const auto version = code_.Version();

    if (current.value_or(0) < version) {
        log_()("Payment code ")(
            pc_display_)(" notification elements not yet indexed for version ")(
            version)
            .Flush();

        return static_cast<Bip32Index>(version);
    } else {
        log_()("Payment code ")(pc_display_)(" already indexed").Flush();

        return std::nullopt;
    }
}

auto NotificationIndex::process(
    const std::optional<Bip32Index>& current,
    Bip32Index target,
    allocator_type monotonic) noexcept -> void
{
    auto elements = database::ElementMap{monotonic};
    auto postcondition = ScopeGuard{[&] { done(std::move(elements)); }};

    for (auto i{code_.Version()}; i > 0; --i) {
        auto& vector = elements[i];

        switch (i) {
            case 1:
            case 2: {
                code_.Locator(writer(vector.emplace_back()), i);
            } break;
            case 3:
            default: {
                vector.reserve(2);
                auto b = std::array<std::byte, 33>{};
                auto& prefix = b[0];
                auto* start = std::next(b.data(), 1);
                auto* stop = std::next(b.data(), b.size());
                code_.Locator(preallocated(32, start), i);
                prefix = std::byte{0x02};
                vector.emplace_back(b.data(), stop);
                prefix = std::byte{0x03};
                vector.emplace_back(b.data(), stop);
            }
        }
    }

    log_()("Payment code ")(pc_display_)(" indexed").Flush();
}
}  // namespace opentxs::blockchain::node::wallet
