// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#pragma once

#include <utility>

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
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

namespace blockchain
{
namespace crypto
{
class Account;
class Subaccount;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using BlockchainAccountStatusType = List<
    BlockchainAccountStatusExternalInterface,
    BlockchainAccountStatusInternalInterface,
    BlockchainAccountStatusRowID,
    BlockchainAccountStatusRowInterface,
    BlockchainAccountStatusRowInternal,
    BlockchainAccountStatusRowBlank,
    BlockchainAccountStatusSortKey,
    BlockchainAccountStatusPrimaryID>;

class BlockchainAccountStatus final : public BlockchainAccountStatusType,
                                      Worker<BlockchainAccountStatus>
{
public:
    auto Chain() const noexcept -> blockchain::Type final { return chain_; }
    auto Owner() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }

    BlockchainAccountStatus(
        const api::session::Client& api,
        const BlockchainAccountStatusPrimaryID& id,
        const blockchain::Type chain,
        const SimpleCallback& cb) noexcept;

    ~BlockchainAccountStatus() final;

private:
    friend Worker<BlockchainAccountStatus>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        newaccount = value(WorkType::BlockchainAccountCreated),
        header = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        progress = value(WorkType::BlockchainWalletScanProgress),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    using SubaccountMap = UnallocatedMap<
        BlockchainAccountStatusRowID,
        std::pair<UnallocatedCString, CustomData>>;
    using ChildMap =
        UnallocatedMap<blockchain::crypto::SubaccountType, SubaccountMap>;

    const blockchain::Type chain_;

    auto construct_row(
        const BlockchainAccountStatusRowID& id,
        const BlockchainAccountStatusSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto last(const BlockchainAccountStatusRowID& id) const noexcept
        -> bool final
    {
        return BlockchainAccountStatusType::last(id);
    }
    auto populate(
        const blockchain::crypto::Account& account,
        const Identifier& subaccountID,
        const blockchain::crypto::SubaccountType type,
        const blockchain::crypto::Subchain subchain,
        ChildMap& out) const noexcept -> void;
    auto populate(
        const blockchain::crypto::Subaccount& node,
        const Identifier& sourceID,
        const UnallocatedCString& sourceDescription,
        const UnallocatedCString& subaccountName,
        const blockchain::crypto::Subchain subchain,
        SubaccountMap& out) const noexcept -> void;
    auto subchain_display_name(
        const blockchain::crypto::Subaccount& node,
        BlockchainSubaccountRowID subchain) const noexcept
        -> std::pair<BlockchainSubaccountSortKey, CustomData>;

    auto add_children(ChildMap&& children) noexcept -> void;
    auto load() noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_account(const Message& in) noexcept -> void;
    auto process_progress(const Message& in) noexcept -> void;
    auto process_reorg(const Message& in) noexcept -> void;
    auto startup() noexcept -> void;

    BlockchainAccountStatus() = delete;
    BlockchainAccountStatus(const BlockchainAccountStatus&) = delete;
    BlockchainAccountStatus(BlockchainAccountStatus&&) = delete;
    auto operator=(const BlockchainAccountStatus&)
        -> BlockchainAccountStatus& = delete;
    auto operator=(BlockchainAccountStatus&&)
        -> BlockchainAccountStatus& = delete;
};
}  // namespace opentxs::ui::implementation
