// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "internal/util/storage/file/Mapped.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
#include "util/storage/file/MappedPrivate.hpp"

namespace opentxs::storage::file
{
Mapped::Mapped(
    const std::filesystem::path& basePath,
    std::string_view filenamePrefix,
    lmdb::Database& lmdb,
    int positionTable,
    std::size_t positionKey,
    allocator_type alloc) noexcept(false)
    : lmdb_(lmdb)
    , mapped_private_([&] {
        // TODO c++20
        auto pmr = alloc::PMR<MappedPrivate>{alloc};
        auto* imp = pmr.allocate(1_uz);

        if (nullptr == imp) { throw std::runtime_error{"allocation failure"}; }

        pmr.construct(
            imp, basePath, filenamePrefix, lmdb_, positionTable, positionKey);

        return imp;
    }())
{
    OT_ASSERT(mapped_private_);
}

auto Mapped::Erase(const Index& index, lmdb::Transaction& tx) noexcept -> bool
{
    return mapped_private_->Erase(index, tx);
}

auto Mapped::get_allocator() const noexcept -> allocator_type
{
    return mapped_private_->get_allocator();
}

auto Mapped::Read(const std::span<const Index> indices, allocator_type alloc)
    const noexcept -> Vector<ReadView>
{
    return mapped_private_->Read(indices, alloc);
}

auto Mapped::Write(
    lmdb::Transaction& tx,
    const Vector<std::size_t>& items) noexcept
    -> Vector<std::pair<Index, WriteBuffer>>
{
    return mapped_private_->Write(tx, items);
}

Mapped::~Mapped()
{
    if (nullptr != mapped_private_) {
        // TODO c++20
        auto pmr = alloc::PMR<MappedPrivate>{get_allocator()};
        pmr.destroy(mapped_private_);
        pmr.deallocate(mapped_private_, 1_uz);
        mapped_private_ = nullptr;
    }
}
}  // namespace opentxs::storage::file
