// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "opentxs/Export.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Secret;

using OTSecret = Pimpl<Secret>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
OPENTXS_EXPORT auto operator==(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator==(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<(const OTSecret& lhs, const ReadView& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator>(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator>(const OTSecret& lhs, const ReadView& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(const OTSecret& lhs, const Secret& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator+=(OTSecret& lhs, const Secret& rhs) noexcept
    -> Secret&;
OPENTXS_EXPORT auto operator+=(OTSecret& lhs, const ReadView rhs) noexcept
    -> Secret&;

class OPENTXS_EXPORT Secret
{
public:
    enum class Mode : bool { Mem = true, Text = false };

    virtual auto operator==(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator==(const ReadView rhs) const noexcept -> bool = 0;
    virtual auto operator!=(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator!=(const ReadView rhs) const noexcept -> bool = 0;
    virtual auto operator<(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator<(const ReadView rhs) const noexcept -> bool = 0;
    virtual auto operator>(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator>(const ReadView rhs) const noexcept -> bool = 0;
    virtual auto operator<=(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator<=(const ReadView& rhs) const noexcept -> bool = 0;
    virtual auto operator>=(const Secret& rhs) const noexcept -> bool = 0;
    virtual auto operator>=(const ReadView& rhs) const noexcept -> bool = 0;
    virtual auto Bytes() const noexcept -> ReadView = 0;
    virtual auto data() const noexcept -> const std::byte* = 0;
    virtual auto empty() const noexcept -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    virtual auto operator+=(const Secret& rhs) noexcept -> Secret& = 0;
    virtual auto operator+=(const ReadView rhs) noexcept -> Secret& = 0;

    virtual void Assign(const Secret& source) noexcept = 0;
    virtual void Assign(const ReadView source) noexcept = 0;
    virtual void Assign(const void* data, const std::size_t& size) noexcept = 0;
    virtual void AssignText(const ReadView source) noexcept = 0;
    virtual void clear() noexcept = 0;
    virtual void Concatenate(const Secret& source) noexcept = 0;
    virtual void Concatenate(const ReadView data) noexcept = 0;
    virtual void Concatenate(
        const void* data,
        const std::size_t size) noexcept = 0;
    virtual auto data() noexcept -> std::byte* = 0;
    virtual auto Randomize(const std::size_t bytes) noexcept -> std::size_t = 0;
    virtual auto Resize(const std::size_t size) noexcept -> std::size_t = 0;
    virtual auto WriteInto(const std::optional<Mode> = {}) noexcept
        -> AllocateOutput = 0;

    Secret(const Secret& rhs) = delete;
    Secret(Secret&& rhs) = delete;
    auto operator=(const Secret& rhs) -> Secret& = delete;
    auto operator=(Secret&& rhs) -> Secret& = delete;

    virtual ~Secret() = default;

protected:
    Secret() = default;

private:
    friend OTSecret;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Secret* = 0;
};
}  // namespace opentxs

namespace std
{
template <>
struct OPENTXS_EXPORT less<opentxs::OTSecret> {
    auto operator()(const opentxs::OTSecret& lhs, const opentxs::OTSecret& rhs)
        const -> bool;
};
}  // namespace std
