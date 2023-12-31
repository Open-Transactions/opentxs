// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <google/protobuf/message_lite.h>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Identifier;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf
{
using MessageType = ::google::protobuf::MessageLite;

auto operator==(const Identifier& lhs, const Identifier& rhs) noexcept -> bool;
auto operator==(const MessageType& lhs, const MessageType& rhs) noexcept
    -> bool;
auto to_string(const MessageType& input) -> UnallocatedCString;
auto write(const MessageType& in, Writer&& out) noexcept -> bool;
}  // namespace opentxs::protobuf
