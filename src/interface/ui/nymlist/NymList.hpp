// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using NymListList = List<
    NymListExternalInterface,
    NymListInternalInterface,
    NymListRowID,
    NymListRowInterface,
    NymListRowInternal,
    NymListRowBlank,
    NymListSortKey,
    NymListPrimaryID>;

class NymList final : public NymListList, Worker<NymList>
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }

    NymList(const api::session::Client& api, const SimpleCallback& cb) noexcept;
    NymList() = delete;
    NymList(const NymList&) = delete;
    NymList(NymList&&) = delete;
    auto operator=(const NymList&) -> NymList& = delete;
    auto operator=(NymList&&) -> NymList& = delete;

    ~NymList() final;

private:
    friend Worker<NymList>;

    enum class Work : OTZMQWorkType {
        newnym = value(WorkType::NymCreated),
        nymchanged = value(WorkType::NymUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    auto construct_row(
        const NymListRowID& id,
        const NymListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto load() noexcept -> void;
    auto load(identifier::Nym&& id) noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_new_nym(Message&& in) noexcept -> void;
    auto process_nym_changed(Message&& in) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
