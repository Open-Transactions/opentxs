// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Mapped.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/storage/file/Index.hpp"  // IWYU pragma: keep
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
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
    , mapped_private_(pmr::construct<MappedPrivate>(
          alloc,
          basePath,
          filenamePrefix,
          lmdb_,
          positionTable,
          positionKey))
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

auto Mapped::preload(std::span<ReadView> bytes) noexcept -> void
{
    preload_platform(bytes);
}

// NOTE: Mapped::preload_platform defined in src/util/platform

auto Mapped::Read(const std::span<const Index> indices, allocator_type alloc)
    const noexcept -> Vector<ReadView>
{
    return mapped_private_->Read(indices, alloc);
}

auto Mapped::Write(
    const ReadView& data,
    const Location& file,
    allocator_type monotonic) noexcept -> bool
{
    return Write(
        {std::addressof(data), 1_uz}, {std::addressof(file), 1_uz}, monotonic);
}

auto Mapped::Write(
    std::span<const ReadView> data,
    std::span<const Location> files,
    allocator_type monotonic) noexcept -> bool
{
    auto cb = [&] {
        auto out = Vector<SourceData>{monotonic};
        out.reserve(data.size());
        out.clear();
        std::ranges::transform(
            data, std::back_inserter(out), [](const auto& view) {
                return std::make_pair(
                    [&](Writer&& writer) {
                        return copy(view, std::move(writer));
                    },
                    view.size());
            });

        return out;
    }();

    return Write(cb, files, monotonic);
}

auto Mapped::Write(
    const SourceData& data,
    const Location& file,
    allocator_type monotonic) noexcept -> bool
{
    return Write(
        {std::addressof(data), 1_uz}, {std::addressof(file), 1_uz}, monotonic);
}

auto Mapped::Write(
    std::span<const SourceData> data,
    std::span<const Location> files,
    allocator_type monotonic) noexcept -> bool
{
    const auto count = data.size();

    OT_ASSERT(files.size() == count);

    auto maps = FileMap{monotonic};

    try {
        for (auto n = 0_uz; n < count; ++n) {
            file::Write(data[n], files[n], maps);
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_STATIC(Mapped))(e.what()).Flush();

        return false;
    }
}

auto Mapped::Write(
    lmdb::Transaction& tx,
    const Vector<std::size_t>& items) noexcept -> WriteParam
{
    return mapped_private_->Write(tx, items);
}

Mapped::~Mapped() { pmr::destroy(mapped_private_); }
}  // namespace opentxs::storage::file
