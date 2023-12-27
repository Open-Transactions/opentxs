// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/Proto.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <limits>

#include "internal/serialization/protobuf/Proto.tpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto operator==(const ProtobufType& lhs, const ProtobufType& rhs) noexcept
    -> bool
{
    auto sLeft = UnallocatedCString{};
    auto sRight = UnallocatedCString{};
    lhs.SerializeToString(&sLeft);
    rhs.SerializeToString(&sRight);

    return sLeft == sRight;
}
}  // namespace opentxs

namespace opentxs::proto
{
auto operator==(const Identifier& lhs, const Identifier& rhs) noexcept -> bool
{
    return lhs.hash() == rhs.hash();
}

auto ToString(const ProtobufType& input) -> UnallocatedCString
{
    auto output = UnallocatedCString{};

    if (write(input, writer(output))) { return output; }

    return {};
}

auto write(const ProtobufType& in, Writer&& out) noexcept -> bool
{
    const auto size = in.ByteSizeLong();

    if (std::numeric_limits<int>::max() < size) { return false; }

    auto dest = out.Reserve(size);

    if (false == dest.IsValid(size)) { return false; }

    return in.SerializeToArray(dest.data(), static_cast<int>(dest.size()));
}
}  // namespace opentxs::proto
