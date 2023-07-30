// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::ui::implementation
{
using SeedListList = List<
    SeedListExternalInterface,
    SeedListInternalInterface,
    SeedListRowID,
    SeedListRowInterface,
    SeedListRowInternal,
    SeedListRowBlank,
    SeedListSortKey,
    SeedListPrimaryID>;

class SeedList final : public SeedListList, Worker<SeedList>
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }

    SeedList(
        const api::session::Client& api,
        const SimpleCallback& cb) noexcept;
    SeedList() = delete;
    SeedList(const SeedList&) = delete;
    SeedList(SeedList&&) = delete;
    auto operator=(const SeedList&) -> SeedList& = delete;
    auto operator=(SeedList&&) -> SeedList& = delete;

    ~SeedList() final;

private:
    friend Worker<SeedList>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        changed_seed = value(WorkType::SeedUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto construct_row(
        const SeedListRowID& id,
        const SeedListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto load() noexcept -> void;
    auto load_seed(
        const SeedListRowID& id,
        SeedListSortKey& name,
        crypto::SeedStyle& type) const noexcept(false) -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_seed(Message&& in) noexcept -> void;
    auto process_seed(const crypto::SeedID& id) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
