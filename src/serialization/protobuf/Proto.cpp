// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "internal/serialization/protobuf/Proto.tpp"  // IWYU pragma: associated

#include <google/protobuf/repeated_field.h>  // IWYU pragma: keep
#include <limits>

#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/util/Bytes.hpp"

template class google::protobuf::RepeatedField<unsigned int>;
template class google::protobuf::RepeatedField<int>;
template class google::protobuf::RepeatedField<unsigned long>;

// TODO I have no idea why those lines above are necessary to fix linking errors
// on Windows but they are.

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
auto ToString(const ProtobufType& input) -> UnallocatedCString
{
    auto output = UnallocatedCString{};

    if (write(input, writer(output))) { return output; }

    return {};
}

auto write(const ProtobufType& in, const AllocateOutput out) noexcept -> bool
{
    if (false == bool(out)) { return false; }

    const auto size = static_cast<std::size_t>(in.ByteSize());

    if (std::numeric_limits<int>::max() < size) { return false; }

    auto dest = out(size);

    if (false == dest.valid(size)) { return false; }

    return in.SerializeToArray(dest.data(), static_cast<int>(dest.size()));
}
}  // namespace opentxs::proto
