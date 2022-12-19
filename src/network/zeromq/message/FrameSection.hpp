// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/message/FrameSection.hpp"

#include <cstddef>

#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"

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
class FrameSection::Imp : public internal::FrameSection
{
public:
    virtual auto at(const std::size_t index) const noexcept(false)
        -> const zeromq::Frame& = 0;
    virtual auto begin() const noexcept -> const zeromq::FrameIterator = 0;
    virtual auto end() const noexcept -> const zeromq::FrameIterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    virtual auto at(const std::size_t index) noexcept(false)
        -> zeromq::Frame& = 0;
    virtual auto begin() noexcept -> zeromq::FrameIterator = 0;
    virtual auto end() noexcept -> zeromq::FrameIterator = 0;

    Imp() = default;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&) -> Imp& = delete;

    ~Imp() override = default;
};
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
class FrameSection final : public zeromq::FrameSection::Imp
{
public:
    auto at(const std::size_t index) const noexcept(false)
        -> const zeromq::Frame& final;
    auto begin() const noexcept -> const zeromq::FrameIterator final;
    auto end() const noexcept -> const zeromq::FrameIterator final;
    auto size() const noexcept -> std::size_t final;

    auto at(const std::size_t index) noexcept(false) -> zeromq::Frame& final;
    auto begin() noexcept -> zeromq::FrameIterator final;
    auto end() noexcept -> zeromq::FrameIterator final;

    FrameSection(
        const Message* parent,
        std::size_t position,
        std::size_t size) noexcept;
    FrameSection() = delete;
    FrameSection(const FrameSection&) = delete;
    FrameSection(FrameSection&&) = delete;
    auto operator=(const FrameSection&) -> FrameSection& = delete;
    auto operator=(FrameSection&) -> FrameSection& = delete;

    ~FrameSection() final = default;

private:
    const Message* parent_;
    const std::size_t position_;
    const std::size_t size_;
};
}  // namespace opentxs::network::zeromq::implementation
