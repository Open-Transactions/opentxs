// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Mapped.hpp"  // IWYU pragma: associated

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/file/Index.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
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
    const ReadView& data,
    const WriteData& files,
    allocator_type monotonic) noexcept -> bool
{
    return Write(
        {std::addressof(data), 1_uz}, {std::addressof(files), 1_uz}, monotonic);
}

auto Mapped::Write(
    std::span<const ReadView> data,
    std::span<const WriteData> files,
    allocator_type monotonic) noexcept -> bool
{
    auto cb = [&] {
        auto out = Vector<SourceData>{monotonic};
        std::transform(
            data.begin(),
            data.end(),
            std::back_inserter(out),
            [](const auto& view) {
                return std::make_pair(
                    [&](auto&& writer) {
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
    const WriteData& files,
    allocator_type monotonic) noexcept -> bool
{
    return Write(
        {std::addressof(data), 1_uz}, {std::addressof(files), 1_uz}, monotonic);
}

auto Mapped::Write(
    std::span<const SourceData> data,
    std::span<const WriteData> files,
    allocator_type monotonic) noexcept -> bool
{
    const auto count = data.size();

    OT_ASSERT(files.size() == count);

    using namespace boost::iostreams;
    auto maps = Map<std::filesystem::path, mapped_file_sink>{monotonic};

    try {
        for (auto n = 0_uz; n < count; ++n) {
            const auto& [cb, size] = data[n];
            const auto& [filename, offset] = files[n];
            // TODO c++20
            auto& file = [&](const auto& f) -> auto&
            {
                if (auto i = maps.find(f); maps.end() != i) {

                    return i->second;
                } else {

                    return maps.try_emplace(f, f.string()).first->second;
                }
            }
            (filename);

            OT_ASSERT(file.is_open());
            OT_ASSERT(cb);

            auto* out = std::next(file.data(), offset);

            if (false == std::invoke(cb, preallocated(size, out))) {
                throw std::runtime_error{"write failed"};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_STATIC(Mapped))(e.what()).Flush();

        return false;
    }
}

auto Mapped::Write(
    lmdb::Transaction& tx,
    const Vector<std::size_t>& items) noexcept
    -> Vector<std::pair<Index, Location>>
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
