// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Envelope;
class EnvelopePrivate;
class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::network::zeromq::Envelope> {
    using is_avalanching = void;

    auto operator()(const opentxs::network::zeromq::Envelope& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto swap(Envelope& lhs, Envelope& rhs) noexcept -> void;
OPENTXS_EXPORT auto operator==(const Envelope&, const Envelope&) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Envelope&, const Envelope&) noexcept
    -> std::strong_ordering;
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT Envelope : public Allocated
{
public:
    operator bool() const noexcept { return IsValid(); }

    auto get() const noexcept -> std::span<const Frame>;
    auto get_allocator() const noexcept -> allocator_type final;
    auto IsValid() const noexcept -> bool;

    auto get() noexcept -> std::span<Frame>;
    auto get_deleter() noexcept -> delete_function final;
    auto swap(Envelope& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Envelope(EnvelopePrivate* imp) noexcept;
    OPENTXS_NO_EXPORT Envelope(
        std::span<Frame> frames,
        allocator_type alloc = {}) noexcept;
    Envelope(allocator_type alloc = {}) noexcept;
    Envelope(const Envelope&, allocator_type alloc = {}) noexcept;
    Envelope(Envelope&&) noexcept;
    Envelope(Envelope&&, allocator_type alloc) noexcept;
    auto operator=(const Envelope&) noexcept -> Envelope&;
    auto operator=(Envelope&&) noexcept -> Envelope&;

    ~Envelope() override;

private:
    EnvelopePrivate* imp_;
};
}  // namespace opentxs::network::zeromq
