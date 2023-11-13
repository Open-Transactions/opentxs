// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class EnvelopePrivate final : public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> EnvelopePrivate*
    {
        return pmr::clone(this, {alloc});
    }

    auto get() const noexcept -> std::span<const Frame> { return data_; }
    auto IsValid() const noexcept -> bool { return false == data_.empty(); }

    auto get() noexcept -> std::span<Frame> { return data_; }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    EnvelopePrivate(const Message& in, allocator_type alloc) noexcept;
    EnvelopePrivate(std::span<Frame> in, allocator_type alloc) noexcept;
    EnvelopePrivate(Message&& in, allocator_type alloc) noexcept;
    EnvelopePrivate(allocator_type alloc = {}) noexcept;
    EnvelopePrivate(const EnvelopePrivate&, allocator_type alloc = {}) noexcept;
    EnvelopePrivate(EnvelopePrivate&&) = delete;
    auto operator=(const EnvelopePrivate&) -> EnvelopePrivate& = delete;
    auto operator=(EnvelopePrivate&&) -> EnvelopePrivate& = delete;

    ~EnvelopePrivate() final;

private:
    Vector<Frame> data_;
};
}  // namespace opentxs::network::zeromq
