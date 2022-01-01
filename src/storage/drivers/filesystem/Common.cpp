// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/iostreams/detail/wrap_unwrap.hpp>

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <ios>
#include <vector>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

#define PATH_SEPERATOR "/"

namespace opentxs::storage::driver::filesystem
{
Common::Common(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const std::string& folder,
    const Flag& bucket)
    : ot_super(crypto, asio, storage, config, bucket)
    , folder_(folder)
    , path_seperator_(PATH_SEPERATOR)
    , ready_(Flag::Factory(false))
{
    Init_Common();
}

void Common::Cleanup() { Cleanup_Common(); }

void Common::Cleanup_Common()
{
    // future cleanup actions go here
}

void Common::Init_Common()
{
    // future init actions go here
}

auto Common::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    value.clear();
    std::string directory{};
    const auto filename = calculate_path(key, bucket, directory);
    boost::system::error_code ec{};

    if (false == boost::filesystem::exists(filename, ec)) { return false; }

    if (ready_.get() && false == folder_.empty()) {
        value = read_file(filename);
    }

    return false == value.empty();
}

auto Common::LoadRoot() const -> std::string
{
    if (ready_.get() && false == folder_.empty()) {

        return read_file(root_filename());
    }

    return "";
}

auto Common::prepare_read(const std::string& input) const -> std::string
{
    return input;
}

auto Common::prepare_write(const std::string& input) const -> std::string
{
    return input;
}

auto Common::read_file(const std::string& filename) const -> std::string
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
        std::pmr::vector<char> bytes(size);
        file.read(&bytes[0], size);

        return prepare_read(std::string(&bytes[0], size));
    }

    return {};
}

void Common::store(
    const bool,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (ready_.get() && false == folder_.empty()) {
        std::string directory{};
        const auto filename = calculate_path(key, bucket, directory);
        promise->set_value(write_file(directory, filename, value));
    } else {
        promise->set_value(false);
    }
}

auto Common::StoreRoot(const bool, const std::string& hash) const -> bool
{
    if (ready_.get() && false == folder_.empty()) {

        return write_file(folder_, root_filename(), hash);
    }

    return false;
}

auto Common::sync(const std::string& path) const -> bool
{
    class FileDescriptor
    {
    public:
        FileDescriptor(const std::string& path)
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

auto Common::sync(File& file) const -> bool { return sync(file->handle()); }

auto Common::sync(int fd) const -> bool
{
#if defined(__APPLE__)
    // This is a Mac OS X system which does not implement
    // fsync as such.
    return 0 == ::fcntl(fd, F_FULLFSYNC);
#else
    return 0 == ::fsync(fd);
#endif
}

auto Common::write_file(
    const std::string& directory,
    const std::string& filename,
    const std::string& contents) const -> bool
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

Common::~Common() { Cleanup_Common(); }

}  // namespace opentxs::storage::driver::filesystem
