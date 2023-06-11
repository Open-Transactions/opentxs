// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/message/EnvelopePrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "internal/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "util/Container.hpp"

namespace opentxs::network::zeromq
{
EnvelopePrivate::EnvelopePrivate(
    const Message& in,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , data_([&] {
        const auto header = in.Internal().Envelope();
        auto out = decltype(data_){alloc};
        out.reserve(header.size());
        out.clear();
        std::copy(header.begin(), header.end(), std::back_inserter(out));

        return out;
    }())
{
}

EnvelopePrivate::EnvelopePrivate(Message&& in, allocator_type alloc) noexcept
    : Allocated(alloc)
    , data_([&] {
        const auto header = in.Internal().Envelope();
        auto out = decltype(data_){alloc};
        out.reserve(header.size());
        out.clear();
        std::move(header.begin(), header.end(), std::back_inserter(out));

        return out;
    }())
{
}

EnvelopePrivate::EnvelopePrivate(
    std::span<Frame> in,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , data_(move_construct<Frame>(in))
{
}

EnvelopePrivate::EnvelopePrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
    , data_(alloc)
{
    data_.clear();
}

EnvelopePrivate::EnvelopePrivate(
    const EnvelopePrivate& rhs,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , data_(rhs.data_, alloc)
{
}

EnvelopePrivate::~EnvelopePrivate() = default;
}  // namespace opentxs::network::zeromq
