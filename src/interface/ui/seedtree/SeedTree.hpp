// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <shared_mutex>
#include <tuple>
#include <utility>

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/WorkType.internal.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using SeedTreeList = List<
    SeedTreeExternalInterface,
    SeedTreeInternalInterface,
    SeedTreeRowID,
    SeedTreeRowInterface,
    SeedTreeRowInternal,
    SeedTreeRowBlank,
    SeedTreeSortKey,
    SeedTreePrimaryID>;

class SeedTree final : public SeedTreeList, Worker<SeedTree>
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ClearCallbacks() const noexcept -> void final;
    auto Debug() const noexcept -> UnallocatedCString final;
    auto DefaultNym() const noexcept -> identifier::Nym final;
    auto DefaultSeed() const noexcept -> crypto::SeedID final;

    auto SetCallbacks(Callbacks&&) noexcept -> void final;

    SeedTree(
        const api::session::Client& api,
        const SimpleCallback& cb) noexcept;
    SeedTree() = delete;
    SeedTree(const SeedTree&) = delete;
    SeedTree(SeedTree&&) = delete;
    auto operator=(const SeedTree&) -> SeedTree& = delete;
    auto operator=(SeedTree&&) -> SeedTree& = delete;

    ~SeedTree() final;

private:
    friend Worker<SeedTree>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        new_nym = value(WorkType::NymCreated),
        changed_nym = value(WorkType::NymUpdated),
        changed_seed = value(WorkType::SeedUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    using NymData = std::pair<SeedTreeItemSortKey, UnallocatedCString>;
    using NymMap = UnallocatedMap<identifier::Nym, NymData>;
    using SeedData =
        std::tuple<bool, UnallocatedCString, crypto::SeedStyle, NymMap>;
    using ChildMap = UnallocatedMap<crypto::SeedID, SeedData>;
    using GuardedCallbacks =
        libguarded::ordered_guarded<Callbacks, std::shared_mutex>;
    using GuardedNym =
        libguarded::ordered_guarded<identifier::Nym, std::shared_mutex>;
    using GuardedSeed =
        libguarded::ordered_guarded<crypto::SeedID, std::shared_mutex>;

    mutable GuardedCallbacks callbacks_;
    GuardedNym default_nym_;
    GuardedSeed default_seed_;

    auto construct_row(
        const SeedTreeRowID& id,
        const SeedTreeSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto load_seed(
        const crypto::SeedID& id,
        UnallocatedCString& name,
        crypto::SeedStyle& type,
        bool& isPrimary) const noexcept(false) -> void;
    auto load_nym(identifier::Nym&& id, ChildMap& out) const noexcept -> void;
    auto load_nyms(ChildMap& out) const noexcept -> void;
    auto load_seed(const crypto::SeedID& id, ChildMap& out) const
        noexcept(false) -> SeedData&;
    auto load_seeds(ChildMap& out) const noexcept -> void;
    auto nym_name(const identity::Nym& nym) const noexcept
        -> UnallocatedCString;

    auto add_children(ChildMap&& children) noexcept -> void;
    auto check_default_nym() noexcept -> void;
    auto check_default_seed() noexcept -> void;
    auto load() noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_nym(Message&& in) noexcept -> void;
    auto process_nym(const identifier::Nym& id) noexcept -> void;
    auto process_seed(Message&& in) noexcept -> void;
    auto process_seed(const crypto::SeedID& id) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
