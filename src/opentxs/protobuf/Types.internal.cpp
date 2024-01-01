// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/Types.internal.hpp"  // IWYU pragma: associated

#include <google/protobuf/message_lite.h>  // IWYU pragma: keep
#include <opentxs/protobuf/Identifier.pb.h>
#include <limits>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::protobuf
{
using namespace std::literals;

auto operator==(const Identifier& lhs, const Identifier& rhs) noexcept -> bool
{
    return lhs.hash() == rhs.hash();
}

auto operator==(const MessageType& lhs, const MessageType& rhs) noexcept -> bool
{
    auto sLeft = ""s;
    auto sRight = ""s;
    lhs.SerializeToString(&sLeft);
    rhs.SerializeToString(&sRight);

    return sLeft == sRight;
}

auto to_string(const MessageType& input) -> UnallocatedCString
{
    auto output = ""s;

    if (write(input, writer(output))) { return output; }

    return {};
}

auto write(const MessageType& in, Writer&& out) noexcept -> bool
{
    const auto size = in.ByteSizeLong();

    if (std::numeric_limits<int>::max() < size) { return false; }

    auto dest = out.Reserve(size);

    if (false == dest.IsValid(size)) { return false; }

    return in.SerializeToArray(dest.data(), static_cast<int>(dest.size()));
}
}  // namespace opentxs::protobuf
