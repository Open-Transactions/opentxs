// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace file
{
class Reader;
class ReaderPrivate;
}  // namespace file
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::file
{
auto swap(Reader&, Reader&) noexcept -> void;
}  // namespace opentxs::storage::file

namespace opentxs::storage::file
{
class Reader final : public opentxs::Allocated
{
public:
    auto get() const noexcept -> ReadView;
    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto swap(Reader& rhs) noexcept -> void;

    Reader(ReaderPrivate* imp) noexcept;
    Reader() = delete;
    Reader(const Reader& rhs, allocator_type alloc = {}) noexcept;
    Reader(Reader&& rhs) noexcept;
    Reader(Reader&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Reader&) noexcept -> Reader& = delete;
    auto operator=(Reader&& rhs) noexcept -> Reader&;

    ~Reader() final;

private:
    ReaderPrivate* imp_;
};
}  // namespace opentxs::storage::file
