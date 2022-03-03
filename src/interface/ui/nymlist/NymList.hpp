// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/ui/NymList.hpp"
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
    NymList(const api::session::Client& api, const SimpleCallback& cb) noexcept;

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
    auto load(OTNymID&& id) noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_new_nym(Message&& in) noexcept -> void;
    auto process_nym_changed(Message&& in) noexcept -> void;
    auto startup() noexcept -> void;

    NymList() = delete;
    NymList(const NymList&) = delete;
    NymList(NymList&&) = delete;
    auto operator=(const NymList&) -> NymList& = delete;
    auto operator=(NymList&&) -> NymList& = delete;
};
}  // namespace opentxs::ui::implementation
