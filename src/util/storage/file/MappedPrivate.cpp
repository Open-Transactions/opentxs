// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/file/MappedPrivate.hpp"  // IWYU pragma: associated

#include <boost/iostreams/device/mapped_file.hpp>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "util/FileSize.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::storage::file
{
constexpr auto get_file_count(const std::size_t bytes) noexcept -> std::size_t
{
    return std::max(
        1_uz,
        ((bytes + 1_uz) / mapped_file_size()) +
            std::min(1_uz, (bytes + 1_uz) % mapped_file_size()));
}

using Offset = std::pair<std::size_t, std::size_t>;

constexpr auto get_offset(const std::size_t in) noexcept -> Offset
{
    return Offset{in / mapped_file_size(), in % mapped_file_size()};
}

constexpr auto get_start_position(const std::size_t file) noexcept
    -> std::size_t
{
    return file * mapped_file_size();
}
}  // namespace opentxs::storage::file

namespace opentxs::storage::file
{
MappedPrivate::Data::Data(
    const std::filesystem::path& basePath,
    std::string_view filenamePrefix,
    lmdb::Database& lmdb,
    int positionTable,
    std::size_t positionKey,
    allocator_type alloc) noexcept(false)
    : path_prefix_(basePath)
    , filename_prefix_(filenamePrefix)
    , position_table_(positionTable)
    , position_key_(positionKey)
    , db_(lmdb)
    , next_position_()
    , files_(alloc)
{
    init_position();
    init_files();
    static_assert(1_uz == get_file_count(0_uz));
    static_assert(1_uz == get_file_count(1_uz));
    static_assert(1_uz == get_file_count(mapped_file_size() - 1_uz));
    static_assert(2_uz == get_file_count(mapped_file_size()));
    static_assert(2_uz == get_file_count(mapped_file_size() + 1_uz));
    static_assert(4_uz == get_file_count(3_uz * mapped_file_size()));
    static_assert(Offset{0_uz, 0_uz} == get_offset(0_uz));
    static_assert(
        Offset{0_uz, mapped_file_size() - 1_uz} ==
        get_offset(mapped_file_size() - 1_uz));
    static_assert(Offset{1_uz, 0_uz} == get_offset(mapped_file_size()));
    static_assert(Offset{1_uz, 1_uz} == get_offset(mapped_file_size() + 1_uz));
    static_assert(0_uz == get_start_position(0_uz));
    static_assert(mapped_file_size() == get_start_position(1_uz));
}

auto MappedPrivate::Data::calculate_file_name(
    const FileCounter index) const noexcept -> std::filesystem::path
{
    auto number = std::to_string(index);

    while (5_uz > number.size()) { number.insert(0, 1, '0'); }

    const auto filename = std::filesystem::path{filename_prefix_} +=
        number + ".dat";

    return std::filesystem::path{path_prefix_.string().c_str()} /
           std::filesystem::path{filename.string().c_str()};
}

auto MappedPrivate::Data::can_read(const Index& index) const noexcept -> bool
{
    const auto size = index.ItemSize();

    if (0_uz == size) {
        LogTrace()(OT_PRETTY_CLASS())("empty index").Flush();

        return false;
    }

    const auto target = index.MemoryPosition() + index.ItemSize() - 1_uz;

    if (target < next_position_) {

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("attempting to read past end of file")
            .Flush();

        return false;
    }
}

auto MappedPrivate::Data::check_file(const FileCounter position) noexcept
    -> void
{
    while (files_.size() < (position + 1_uz)) { create_or_load(files_.size()); }
}

auto MappedPrivate::Data::create_or_load(FileCounter file) noexcept -> void
{
    auto params = boost::iostreams::mapped_file_params{
        calculate_file_name(file).string()};
    params.flags = boost::iostreams::mapped_file::readwrite;
    const auto& path = params.path;
    LogTrace()(OT_PRETTY_CLASS())("initializing file ")(path).Flush();

    try {
        if (std::filesystem::exists(path)) {
            if (mapped_file_size() == std::filesystem::file_size(path)) {
                params.new_file_size = 0;
            } else {
                LogError()(OT_PRETTY_CLASS())("Incorrect size for ")(path)
                    .Flush();
                std::filesystem::remove(path);
                params.new_file_size = mapped_file_size();
            }
        } else {
            params.new_file_size = mapped_file_size();
        }
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }

    LogInsane()(OT_PRETTY_CLASS())("new_file_size: ")(params.new_file_size)
        .Flush();

    try {
        files_.emplace_back(params);
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(e.what()).Abort();
    }
}

auto MappedPrivate::Data::Erase(
    const Index& index,
    lmdb::Transaction& tx) noexcept -> bool
{
    if (const auto pos = index.MemoryPosition(); pos < next_position_) {

        return update_next_position(pos, tx);
    } else {
        LogError()(OT_PRETTY_CLASS())("position ")(pos)(" is already deleted")
            .Flush();

        return false;
    }
}

auto MappedPrivate::Data::init_files() noexcept -> void
{
    const auto target = get_file_count(next_position_);
    files_.reserve(target);
    files_.clear();

    for (auto i = FileCounter{0_uz}; i < target; ++i) { create_or_load(i); }
}

auto MappedPrivate::Data::init_position() noexcept -> void
{
    if (db_.Exists(position_table_, tsv(position_key_))) {
        auto cb = [this](const auto in) {
            if (sizeof(next_position_) != in.size()) { return; }

            std::memcpy(&next_position_, in.data(), in.size());
        };
        db_.Load(position_table_, tsv(position_key_), cb);
    } else {
        db_.Store(position_table_, tsv(position_key_), tsv(next_position_));
    }
}

auto MappedPrivate::Data::Read(
    const std::span<const Index> indices,
    allocator_type alloc) noexcept -> Vector<ReadView>
{
    auto out = Vector<ReadView>{alloc};
    out.reserve(indices.size());
    out.clear();

    for (const auto& index : indices) {
        if (can_read(index)) {
            const auto [file, offset] = get_offset(index.MemoryPosition());
            check_file(file);
            out.emplace_back(
                files_.at(file).const_data() + offset, index.ItemSize());
        } else {
            out.emplace_back();
        }
    }

    OT_ASSERT(out.size() == indices.size());

    return out;
}

auto MappedPrivate::Data::update_index(
    const std::size_t& next,
    std::size_t bytes,
    Index& out) noexcept -> void
{
    OT_ASSERT(0_uz < bytes);

    const auto position = [&] {
        const auto start = get_offset(next).first;
        const auto end = get_offset(next + (bytes - 1_uz)).first;

        // NOTE This check prevents writing past end of file
        if (end == start) {

            return next;
        } else {
            OT_ASSERT(end > start);

            return get_start_position(end);
        }
    }();

    out.SetMemoryPosition(position);
    out.SetItemSize(bytes);
}

auto MappedPrivate::Data::update_next_position(
    std::size_t position,
    lmdb::Transaction& tx) noexcept -> bool
{
    auto result =
        db_.Store(position_table_, tsv(position_key_), tsv(position), tx);

    if (false == result.first) {
        LogError()(OT_PRETTY_CLASS())("Failed to next write position").Flush();

        return false;
    }

    next_position_ = position;

    return true;
}

auto MappedPrivate::Data::Write(
    lmdb::Transaction& tx,
    const Vector<std::size_t>& items) noexcept
    -> Vector<std::pair<Index, WriteBuffer>>
{
    const auto count = items.size();
    using Output = Vector<std::pair<Index, WriteBuffer>>;
    auto out = Output{count, items.get_allocator()};
    auto post = ScopeGuard{[&] { OT_ASSERT(out.size() == items.size()); }};

    if (items.empty()) { return out; }

    auto next{next_position_};

    for (auto n = 0_uz; n < count; ++n) {
        const auto& size = items.at(n);
        auto& [index, view] = out.at(n);

        if (0_uz == size) { continue; }

        update_index(next, size, index);
        next += size;
        const auto [file, offset] = get_offset(index.MemoryPosition());
        check_file(file);
        auto* ptr = reinterpret_cast<std::byte*>(files_.at(file).data());
        view = std::span<std::byte>{std::next(ptr, offset), size};
    }

    if (false == update_next_position(next, tx)) {
        LogError()(OT_PRETTY_CLASS())(
            "failed to update next write position to database")
            .Flush();
        out = Output{count, items.get_allocator()};
    }

    return out;
}
}  // namespace opentxs::storage::file

namespace opentxs::storage::file
{
MappedPrivate::MappedPrivate(
    const std::filesystem::path& basePath,
    std::string_view filenamePrefix,
    lmdb::Database& lmdb,
    int positionTable,
    std::size_t positionKey,
    allocator_type alloc) noexcept(false)
    : Allocated(std::move(alloc))
    , data_(
          basePath,
          filenamePrefix,
          lmdb,
          positionTable,
          positionKey,
          get_allocator())
{
}

auto MappedPrivate::Erase(const Index& index, lmdb::Transaction& tx) noexcept
    -> bool
{
    return data_.lock()->Erase(index, tx);
}

auto MappedPrivate::Read(
    const std::span<const Index> indices,
    allocator_type alloc) const noexcept -> Vector<ReadView>
{
    return data_.lock()->Read(indices, alloc);
}

auto MappedPrivate::Write(
    lmdb::Transaction& tx,
    const Vector<std::size_t>& items) noexcept
    -> Vector<std::pair<Index, WriteBuffer>>
{
    return data_.lock()->Write(tx, items);
}

MappedPrivate::~MappedPrivate() = default;
}  // namespace opentxs::storage::file
