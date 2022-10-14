// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <google/protobuf/message_lite.h>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using ProtoValidationVerbosity = bool;
static const ProtoValidationVerbosity SILENT = true;
static const ProtoValidationVerbosity VERBOSE = false;

using ProtobufType = ::google::protobuf::MessageLite;

auto operator==(const ProtobufType& lhs, const ProtobufType& rhs) noexcept
    -> bool;

}  // namespace opentxs

namespace opentxs::proto
{
auto write(const ProtobufType& in, Writer&& out) noexcept -> bool;
}  // namespace opentxs::proto
