// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace rpc
{
namespace response
{
class Message;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
auto OPENTXS_EXPORT Factory(ReadView protobuf) noexcept
    -> std::unique_ptr<Message>;
}  // namespace opentxs::rpc::response
