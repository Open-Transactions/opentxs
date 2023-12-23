// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>

#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
/** A set of metadata associated with a stored object
 *  * Hash: hash
 *  * string: alias
 *  * uint64: revision
 *  * bool:   private
 */
using Metadata = std::tuple<Hash, UnallocatedCString, std::uint64_t, bool>;
/** Maps a logical id to the stored metadata for the object
 *  * string: id of the stored object
 *  * Metadata: metadata for the stored object
 */
using Index = UnallocatedMap<identifier::Generic, Metadata>;

struct GCParams {
    bool running_{};
    Time last_{};
    Hash root_{};
    Bucket from_{};
};

// WARNING update print function if new values are added or removed
enum class Job : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    finished = OT_ZMQ_INTERNAL_SIGNAL + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(Job) noexcept -> std::string_view;
}  // namespace opentxs::storage::tree
