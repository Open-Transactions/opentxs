// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class HasherPrivate
{
public:
    [[nodiscard]] auto operator()(ReadView data) noexcept -> bool;
    [[nodiscard]] auto operator()(Writer&& destination) noexcept -> bool;

    HasherPrivate() noexcept;
    HasherPrivate(const void* evp, const void* stage2) noexcept;
    HasherPrivate(const HasherPrivate&) = delete;
    HasherPrivate(HasherPrivate&&) = delete;
    auto operator=(const HasherPrivate&) -> HasherPrivate& = delete;
    auto operator=(HasherPrivate&&) noexcept -> HasherPrivate& = delete;

    ~HasherPrivate();

private:
    const std::size_t size_1_;
    const std::size_t size_2_;
    void* context_;
    const void* stage_2_;

    auto free_context() noexcept -> void;
};
}  // namespace opentxs::crypto
