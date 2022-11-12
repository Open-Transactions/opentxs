// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/iostreams/detail/wrap_unwrap.hpp>

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

#include <boost/core/typeinfo.hpp>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <fstream>
#include <ios>
#include <system_error>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::storage::driver::filesystem
{
Common::Common(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const UnallocatedCString& folder,
    const Flag& bucket)
    : ot_super(crypto, asio, storage, config, bucket)
    , folder_(folder)
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
    const UnallocatedCString& key,
    UnallocatedCString& value,
    const bool bucket) const -> bool
{
    value.clear();
    auto directory = fs::path{};
    const auto filename = calculate_path(key, bucket, directory);
    auto ec = std::error_code{};

    if (false == fs::exists(filename, ec)) { return false; }

    if (ready_.get() && false == folder_.empty()) {
        value = read_file(filename.string());
    }

    return false == value.empty();
}

auto Common::LoadRoot() const -> UnallocatedCString
{
    if (ready_.get() && false == folder_.empty()) {

        return read_file(root_filename().string());
    }

    return "";
}

auto Common::prepare_read(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    return input;
}

auto Common::prepare_write(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    return input;
}

auto Common::read_file(const UnallocatedCString& filename) const
    -> UnallocatedCString
{
    try {
        auto ec = std::error_code{};

        if (false == fs::exists(filename, ec)) { return {}; }

        std::ifstream file(
            filename, std::ios::in | std::ios::ate | std::ios::binary);

        if (file.good()) {
            const auto pos = file.tellg();

            if ((0 >= pos) || (0xFFFFFFFF <= pos)) { return {}; }

            const auto size = convert_to_size(pos);
            file.seekg(0, std::ios::beg);
            UnallocatedVector<char> bytes(size);
            file.read(&bytes[0], size);

            return prepare_read(UnallocatedCString(&bytes[0], size));
        }

        return {};
    } catch (...) {

        return {};
    }
}

void Common::store(
    const bool,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (ready_.get() && false == folder_.empty()) {
        auto directory = fs::path{};
        const auto filename = calculate_path(key, bucket, directory);
        promise->set_value(
            write_file(directory.string(), filename.string(), value));
    } else {
        promise->set_value(false);
    }
}

auto Common::StoreRoot(const bool, const UnallocatedCString& hash) const -> bool
{
    if (ready_.get() && false == folder_.empty()) {

        return write_file(folder_.string(), root_filename().string(), hash);
    }

    return false;
}

auto Common::sync(const fs::path& path) const -> bool
{
    auto fd = FileDescriptor{path};

    if (!fd) {
        LogError()(OT_PRETTY_CLASS())("Failed to open ")(path).Flush();

        return false;
    }

    return sync(fd);
}

auto Common::sync(File& file) const -> bool { return sync(file->handle()); }

auto Common::write_file(
    const UnallocatedCString& directory,
    const UnallocatedCString& filename,
    const UnallocatedCString& contents) const -> bool
{
    if (false == filename.empty()) {
        fs::path filePath(filename);
        File file(filePath.string());
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
