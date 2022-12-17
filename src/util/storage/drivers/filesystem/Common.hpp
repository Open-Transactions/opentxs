// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <ios>

#pragma once

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <chrono>
#include <filesystem>
#include <future>
#include <string_view>

#include "internal/util/Flag.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/Plugin.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace fs = std::filesystem;

namespace opentxs::storage::driver::filesystem
{
// Simple filesystem implementation of opentxs::storage
class Common : public implementation::Plugin
{
private:
    using ot_super = Plugin;

public:
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool override;
    auto LoadRoot() const -> UnallocatedCString override;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool override;

    void Cleanup() override;

    Common() = delete;
    Common(const Common&) = delete;
    Common(Common&&) = delete;
    auto operator=(const Common&) -> Common& = delete;
    auto operator=(Common&&) -> Common& = delete;

    ~Common() override;

protected:
    const fs::path folder_;
    OTFlag ready_;

    auto sync(const fs::path& path) const -> bool;

    Common(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const UnallocatedCString& folder,
        const Flag& bucket);

private:
    using DescriptorType = boost::iostreams::file_descriptor_sink;
    using File = boost::iostreams::stream<DescriptorType>;

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

    virtual auto calculate_path(
        std::string_view key,
        bool bucket,
        fs::path& directory) const noexcept -> fs::path = 0;
    virtual auto prepare_read(const UnallocatedCString& input) const
        -> UnallocatedCString;
    virtual auto prepare_write(const UnallocatedCString& input) const
        -> UnallocatedCString;
    auto read_file(const UnallocatedCString& filename) const
        -> UnallocatedCString;
    virtual auto root_filename() const -> fs::path = 0;
    void store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket,
        std::promise<bool>* promise) const override;
    auto sync(File& file) const -> bool;
    auto sync(DescriptorType::handle_type) const -> bool;
    auto write_file(
        const UnallocatedCString& directory,
        const UnallocatedCString& filename,
        const UnallocatedCString& contents) const -> bool;

    void Cleanup_Common();
    void Init_Common();
};
}  // namespace opentxs::storage::driver::filesystem
