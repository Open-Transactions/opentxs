// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <ios>

#pragma once

#include <cs_shared_guarded.h>
#include <filesystem>
#include <shared_mutex>
#include <string_view>

#include "BoostIostreams.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/drivers/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace fs = std::filesystem;

namespace opentxs::storage::driver::filesystem
{
class Common : public storage::implementation::Driver
{
public:
    auto Load(const Log& logger, const Hash& key, Search order, Writer& value)
        const noexcept -> bool final;
    auto LoadRoot() const noexcept -> Hash final;

    auto Commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> bool final;
    auto EmptyBucket(Bucket bucket) const noexcept -> bool final;
    auto Store(Transaction data, Bucket bucket) const noexcept -> bool final;

    Common() = delete;
    Common(const Common&) = delete;
    Common(Common&&) = delete;
    auto operator=(const Common&) -> Common& = delete;
    auto operator=(Common&&) -> Common& = delete;

    ~Common() override;

protected:
    struct Data {
        const fs::path folder_;

        Data(const fs::path& folder) noexcept
            : folder_(folder)
        {
        }
        Data() = delete;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using DescriptorType = boost::iostreams::file_descriptor_sink;
    using File = boost::iostreams::stream<DescriptorType>;

    static auto finalize_write(
        const fs::path& directory,
        const fs::path& filename,
        File& file,
        ReadView data) noexcept(false) -> void;
    static auto sync(const fs::path& path) noexcept -> bool;

    auto init() noexcept(false) -> void;
    virtual auto init(Data& data) noexcept(false) -> void;

    Common(
        const api::Crypto& crypto,
        const storage::Config& config,
        const std::filesystem::path& folder) noexcept;

private:
    class FileDescriptor
    {
    public:
        operator bool() const noexcept { return good(); }
        operator DescriptorType::handle_type() const noexcept { return fd_; }

        FileDescriptor(const fs::path& path) noexcept;
        FileDescriptor() = delete;
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&&) = delete;
        auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;
        auto operator=(FileDescriptor&&) -> FileDescriptor& = delete;

        ~FileDescriptor();

    private:
        DescriptorType::handle_type fd_;

        auto good() const noexcept -> bool;
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    mutable GuardedData data_;

    static auto sync(DescriptorType::handle_type) noexcept -> bool;
    static auto sync(File& file) noexcept -> bool;

    virtual auto calculate_path(
        const Data& data,
        std::string_view key,
        Bucket bucket,
        fs::path& directory) const noexcept -> fs::path = 0;
    virtual auto do_write(
        const fs::path& directory,
        const fs::path& filename,
        File& file,
        ReadView data) const noexcept(false) -> void;
    virtual auto empty_bucket(const Data& data, Bucket bucket) const
        noexcept(false) -> bool = 0;
    virtual auto finalize_read(UnallocatedCString&& input) const noexcept(false)
        -> UnallocatedCString;
    auto read_file(const fs::path& filename) const noexcept
        -> UnallocatedCString;
    auto read_file(const fs::path& filename, Writer&& out) const noexcept
        -> bool;
    virtual auto root_filename(const Data& data) const noexcept -> fs::path = 0;
    auto store(Data& data, Transaction values, Bucket bucket) const noexcept
        -> bool;
    auto write_file(
        const fs::path& directory,
        const fs::path& filename,
        ReadView contents) const noexcept -> bool;
};
}  // namespace opentxs::storage::driver::filesystem
