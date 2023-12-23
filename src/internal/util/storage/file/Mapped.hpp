// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>

#include "internal/util/storage/file/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace file
{
class Index;
class MappedPrivate;
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
class Mapped : virtual public opentxs::Allocated
{
public:
    static auto Write(
        const ReadView& data,
        const Location& file,
        allocator_type monotonic) noexcept -> bool;
    static auto Write(
        std::span<const ReadView> data,
        std::span<const Location> files,
        allocator_type monotonic) noexcept -> bool;
    static auto Write(
        const SourceData& data,
        const Location& file,
        allocator_type monotonic) noexcept -> bool;
    static auto Write(
        std::span<const SourceData> data,
        std::span<const Location> files,
        allocator_type monotonic) noexcept -> bool;

    auto get_allocator() const noexcept -> allocator_type final;
    auto Read(const std::span<const Index> indices, allocator_type alloc)
        const noexcept -> Vector<ReadView>;

    auto Erase(const Index& index, lmdb::Transaction& tx) noexcept -> bool;
    auto Write(lmdb::Transaction& tx, const Vector<std::size_t>& items) noexcept
        -> WriteParam;

    Mapped(const Mapped&) = delete;
    Mapped(Mapped&&) = delete;
    auto operator=(const Mapped&) -> Mapped& = delete;
    auto operator=(Mapped&&) -> Mapped& = delete;

    ~Mapped() override;

protected:
    lmdb::Database& lmdb_;

    Mapped(
        const std::filesystem::path& basePath,
        std::string_view filenamePrefix,
        lmdb::Database& lmdb,
        int positionTable,
        std::size_t positionKey,
        allocator_type alloc) noexcept(false);

private:
    friend MappedPrivate;

    MappedPrivate* mapped_private_;

    static auto preload(std::span<ReadView> bytes) noexcept -> void;
    static auto preload_platform(std::span<ReadView> bytes) noexcept -> void;
};
}  // namespace opentxs::storage::file
