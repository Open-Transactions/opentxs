// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <google/protobuf/message_lite.h>

#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
using ProtoValidationVerbosity = bool;
static const ProtoValidationVerbosity SILENT = true;
static const ProtoValidationVerbosity VERBOSE = false;

using ProtobufType = ::google::protobuf::MessageLite;

auto operator==(const ProtobufType& lhs, const ProtobufType& rhs) noexcept
    -> bool;

namespace proto
{
auto write(const ProtobufType& in, const AllocateOutput out) noexcept -> bool;
}  // namespace proto
}  // namespace opentxs
