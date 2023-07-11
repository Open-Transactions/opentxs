// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>

#include "BoostIostreams.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace file
{
class Index;
}  // namespace file

namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::file
{
class MappedPrivate final : public opentxs::implementation::Allocated
{
public:
    auto Read(const std::span<const Index> indices, allocator_type alloc)
        const noexcept -> Vector<ReadView>;

    auto Erase(const Index& index, lmdb::Transaction& tx) noexcept -> bool;
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto Write(lmdb::Transaction& tx, const Vector<std::size_t>& items) noexcept
        -> WriteParam;

    MappedPrivate(
        const std::filesystem::path& basePath,
        std::string_view filenamePrefix,
        lmdb::Database& lmdb,
        int positionTable,
        std::size_t positionKey,
        allocator_type alloc) noexcept(false);
    MappedPrivate(const MappedPrivate&) = delete;
    MappedPrivate(MappedPrivate&&) = delete;
    auto operator=(const MappedPrivate&) -> MappedPrivate& = delete;
    auto operator=(MappedPrivate&&) -> MappedPrivate& = delete;

    ~MappedPrivate() final;

private:
    class Data
    {
    public:
        auto Erase(const Index& index, lmdb::Transaction& tx) noexcept -> bool;
        auto Read(
            const std::span<const Index> indices,
            allocator_type alloc) noexcept -> Vector<ReadView>;
        auto Write(
            lmdb::Transaction& tx,
            const Vector<std::size_t>& items) noexcept -> WriteParam;

        Data(
            const std::filesystem::path& basePath,
            std::string_view filenamePrefix,
            lmdb::Database& lmdb,
            int positionTable,
            std::size_t positionKey,
            allocator_type alloc) noexcept(false);
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

    private:
        using FileCounter = std::size_t;

        const std::filesystem::path path_prefix_;
        const std::filesystem::path filename_prefix_;
        const int position_table_;
        const std::size_t position_key_;
        lmdb::Database& db_;
        FileCounter next_position_;
        Vector<MappedFileType> files_;

        static auto update_index(
            const std::size_t& next,
            std::size_t bytes,
            Index& out) noexcept -> void;

        auto calculate_file_name(const FileCounter index) const noexcept
            -> std::filesystem::path;
        auto can_read(const Index& index) const noexcept -> bool;

        auto check_file(FileCounter position) noexcept -> void;
        auto create_or_load(FileCounter file) noexcept -> void;
        auto init_files() noexcept -> void;
        auto init_position() noexcept -> void;
        auto update_next_position(
            std::size_t position,
            lmdb::Transaction& tx) noexcept -> bool;
    };

    mutable libguarded::plain_guarded<Data> data_;
};
}  // namespace opentxs::storage::file
