// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/iostreams/detail/wrap_unwrap.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/filesystem/Parent.hpp"  // IWYU pragma: associated

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <ios>
#include <memory>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

#define PATH_SEPERATOR "/"

namespace opentxs::storage::driver::filesystem
{
Filesystem::Common(
    const api::session::Storage& storage,
    const storage::Config& config,
    const Digest& hash,
    const Random& random,
    const UnallocatedCString& folder,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , folder_(folder)
    , path_seperator_(PATH_SEPERATOR)
    , ready_(Flag::Factory(false))
{
    Init_Filesystem();
}

void Filesystem::Cleanup() { Cleanup_Filesystem(); }

void Filesystem::Cleanup_Filesystem()
{
    // future cleanup actions go here
}

void Filesystem::Init_Filesystem()
{
    // future init actions go here
}

auto Filesystem::LoadFromBucket(
    const UnallocatedCString& key,
    UnallocatedCString& value,
    const bool bucket) const -> bool
{
    value.clear();
    UnallocatedCString directory{};
    const auto filename = calculate_path(key, bucket, directory);
    boost::system::error_code ec{};

    if (false == boost::filesystem::exists(filename, ec)) { return false; }

    if (ready_.get() && false == folder_.empty()) {
        value = read_file(filename);
    }

    return false == value.empty();
}

auto Filesystem::LoadRoot() const -> UnallocatedCString
{
    if (ready_.get() && false == folder_.empty()) {

        return read_file(root_filename());
    }

    return "";
}

auto Filesystem::prepare_read(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    return input;
}

auto Filesystem::prepare_write(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    return input;
}

auto Filesystem::read_file(const UnallocatedCString& filename) const
    -> UnallocatedCString
{
    boost::system::error_code ec{};

    if (false == boost::filesystem::exists(filename, ec)) { return {}; }

    std::ifstream file(
        filename, std::ios::in | std::ios::ate | std::ios::binary);

    if (file.good()) {
        std::ifstream::pos_type pos = file.tellg();

        if ((0 >= pos) || (0xFFFFFFFF <= pos)) { return {}; }

        auto size(pos);
        file.seekg(0, std::ios::beg);
        UnallocatedVector<char> bytes(size);
        file.read(&bytes[0], size);

        return prepare_read(UnallocatedCString(&bytes[0], size));
    }

    return {};
}

void Filesystem::store(
    const bool,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (ready_.get() && false == folder_.empty()) {
        UnallocatedCString directory{};
        const auto filename = calculate_path(key, bucket, directory);
        promise->set_value(write_file(directory, filename, value));
    } else {
        promise->set_value(false);
    }
}

auto Filesystem::StoreRoot(const bool, const UnallocatedCString& hash) const
    -> bool
{
    if (ready_.get() && false == folder_.empty()) {

        return write_file(folder_, root_filename(), hash);
    }

    return false;
}

auto Filesystem::sync(const UnallocatedCString& path) const -> bool
{
    class FileDescriptor
    {
    public:
        FileDescriptor(const UnallocatedCString& path)
            : fd_(::open(path.c_str(), O_DIRECTORY | O_RDONLY))
        {
        }

        operator bool() const { return good(); }
        operator int() const { return fd_; }

        ~FileDescriptor()
        {
            if (good()) { ::close(fd_); }
        }

    private:
        int fd_{-1};

        auto good() const -> bool { return (-1 != fd_); }

        FileDescriptor() = delete;
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&&) = delete;
        auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;
        auto operator=(FileDescriptor&&) -> FileDescriptor& = delete;
    };

    FileDescriptor fd(path);

    if (!fd) {
        LogError()(OT_PRETTY_CLASS())("Failed to open ")(path)(".").Flush();

        return false;
    }

    return sync(fd);
}

auto Filesystem::sync(File& file) const -> bool { return sync(file->handle()); }

auto Filesystem::sync(int fd) const -> bool
{
#if defined(__APPLE__)
    // This is a Mac OS X system which does not implement
    // fsync as such.
    return 0 == ::fcntl(fd, F_FULLFSYNC);
#else
    return 0 == ::fsync(fd);
#endif
}

auto Filesystem::write_file(
    const UnallocatedCString& directory,
    const UnallocatedCString& filename,
    const UnallocatedCString& contents) const -> bool
{
    if (false == filename.empty()) {
        boost::filesystem::path filePath(filename);
        File file(filePath);
        const auto data = prepare_write(contents);

        if (file.good()) {
            file.write(data.c_str(), data.size());

            if (false == sync(file)) {
                LogError()(OT_PRETTY_CLASS())("Failed to sync file ")(
                    filename)(".")
                    .Flush();
            }

            if (false == sync(directory)) {
                LogError()(OT_PRETTY_CLASS())("Failed to sync directory ")(
                    directory)(".")
                    .Flush();
            }

            file.close();

            return true;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to write file.").Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to write empty filename.")
            .Flush();
    }

    return false;
}

Filesystem::~Filesystem() { Cleanup_Filesystem(); }

}  // namespace opentxs::storage::driver::filesystem
