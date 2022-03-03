// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/interface/ui/BlockchainSelection.hpp"
#include "opentxs/interface/ui/Blockchains.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
using BLOCKCHAINSELECTIONKEY = std::pair<opentxs::UnallocatedCString, bool>;

template <>
struct less<BLOCKCHAINSELECTIONKEY> {
    auto operator()(
        const BLOCKCHAINSELECTIONKEY& lhs,
        const BLOCKCHAINSELECTIONKEY& rhs) const -> bool
    {
        const auto& [lName, lTestnet] = lhs;
        const auto& [rName, rTestnet] = rhs;

        if ((!lTestnet) && (rTestnet)) { return true; }

        if (lTestnet && (!rTestnet)) { return false; }

        if (lName < rName) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs::ui::implementation
{
using BlockchainSelectionList = List<
    BlockchainSelectionExternalInterface,
    BlockchainSelectionInternalInterface,
    BlockchainSelectionRowID,
    BlockchainSelectionRowInterface,
    BlockchainSelectionRowInternal,
    BlockchainSelectionRowBlank,
    BlockchainSelectionSortKey,
    BlockchainSelectionPrimaryID>;

class BlockchainSelection final : public BlockchainSelectionList,
                                  Worker<BlockchainSelection>
{
public:
    auto Disable(const blockchain::Type type) const noexcept -> bool final;
    auto Enable(const blockchain::Type type) const noexcept -> bool final;
    auto EnabledCount() const noexcept -> std::size_t final;
    auto Set(EnabledCallback&& cb) const noexcept -> void final;

    BlockchainSelection(
        const api::session::Client& api,
        const ui::Blockchains type,
        const SimpleCallback& cb) noexcept;

    ~BlockchainSelection() final;

private:
    friend Worker<BlockchainSelection>;

    struct Callback {
        auto run(blockchain::Type chain, bool enabled, std::size_t total)
            const noexcept -> void
        {
            auto lock = Lock{lock_};

            if (cb_) { cb_(chain, enabled, total); }
        }
        auto set(const EnabledCallback& cb) noexcept -> void
        {
            auto lock = Lock{lock_};
            cb_ = cb;
        }

    private:
        mutable std::mutex lock_{};
        EnabledCallback cb_{};
    };

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        statechange = value(WorkType::BlockchainStateChange),
        enable = OT_ZMQ_INTERNAL_SIGNAL + 0,
        disable = OT_ZMQ_INTERNAL_SIGNAL + 1,
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const UnallocatedSet<blockchain::Type> filter_;
    mutable UnallocatedMap<blockchain::Type, bool> chain_state_;
    mutable std::atomic<std::size_t> enabled_count_;
    mutable Callback enabled_callback_;

    static auto filter(const ui::Blockchains type) noexcept
        -> UnallocatedSet<blockchain::Type>;

    auto process_state(const blockchain::Type chain, const bool enabled)
        const noexcept -> void;

    auto construct_row(
        const BlockchainSelectionRowID& id,
        const BlockchainSelectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto disable(const Message& in) noexcept -> void;
    auto enable(const Message& in) noexcept -> void;
    auto pipeline(const Message& in) noexcept -> void;
    auto process_state(const Message& in) noexcept -> void;
    auto startup() noexcept -> void;

    BlockchainSelection() = delete;
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    auto operator=(const BlockchainSelection&) -> BlockchainSelection& = delete;
    auto operator=(BlockchainSelection&&) -> BlockchainSelection& = delete;
};
}  // namespace opentxs::ui::implementation
