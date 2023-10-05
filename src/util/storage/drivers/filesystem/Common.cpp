// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/iostreams/detail/wrap_unwrap.hpp>

#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

#include <array>
#include <fstream>
#include <memory>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <variant>

#include "BoostIostreams.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::storage::driver::filesystem
{
using namespace std::literals;

// NOTE Common::FileDescriptor defined in src/util/platform

Common::Common(
    const api::Crypto& crypto,
    const storage::Config& config,
    const std::filesystem::path& folder) noexcept
    : Driver(crypto, config)
    , data_(folder)
{
}

auto Common::Commit(const Hash& root, Transaction values, Bucket bucket)
    const noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (false == store(data, values, bucket)) { return false; }

    return write_file(
        data.folder_, root_filename(data), std::visit(EncodedView{}, root));
}

auto Common::do_write(
    const fs::path& directory,
    const fs::path& filename,
    File& file,
    ReadView data) const noexcept(false) -> void
{
    finalize_write(directory, filename, file, data);
}

auto Common::EmptyBucket(Bucket bucket) const noexcept -> bool
{
    try {

        return empty_bucket(*data_.lock(), bucket);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Common::finalize_read(UnallocatedCString&& input) const noexcept(false)
    -> UnallocatedCString
{
    return std::move(input);
}

auto Common::init() noexcept(false) -> void { init(*data_.lock()); }

auto Common::init(Data& data) noexcept(false) -> void
{
    if (data.folder_.empty()) {
        throw std::runtime_error{"invalid data folder"};
    }

    if (config_.fs_root_file_.empty()) {
        throw std::runtime_error{"invalid storage configuration"};
    }
}

auto Common::Load(
    const Log& logger,
    const Hash& key,
    Search order,
    Writer& value) const noexcept -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    const auto search = [&, this]() -> std::array<fs::path, 2> {
        using enum Bucket;
        using enum Search;
        auto notUsed = fs::path{};

        if (ltr == order) {

            return {
                calculate_path(
                    data, std::visit(EncodedView{}, key), left, notUsed),
                calculate_path(
                    data, std::visit(EncodedView{}, key), right, notUsed)};
        } else {

            return {
                calculate_path(
                    data, std::visit(EncodedView{}, key), right, notUsed),
                calculate_path(
                    data, std::visit(EncodedView{}, key), left, notUsed)};
        }
    }();

    for (const auto& file : search) {
        try {
            if (fs::exists(file)) { return read_file(file, std::move(value)); }
        } catch (const std::exception& e) {
            logger(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }

    return false;
}

auto Common::LoadRoot() const noexcept -> Hash
{
    return read(read_file(root_filename(*data_.lock_shared())));
}

auto Common::read_file(const fs::path& filename) const noexcept
    -> UnallocatedCString
{
    auto out = UnallocatedCString{};
    read_file(filename, writer(out));

    return out;
}

auto Common::read_file(const fs::path& filename, Writer&& value) const noexcept
    -> bool
{
    try {
        {
            auto ec = std::error_code{};

            if (false == fs::exists(filename, ec)) { return {}; }
        }

        auto file = std::ifstream{
            filename, std::ios::in | std::ios::ate | std::ios::binary};

        if (false == file.good()) {

            throw std::runtime_error{
                "unable to open "s + filename.string() + " for reading"};
        }

        const auto pos = file.tellg();

        if ((0 >= pos) || (0xFFFFFFFF <= pos)) {

            throw std::runtime_error{
                filename.string() +
                " size out of range: " + std::to_string(pos)};
        }

        file.seekg(0, std::ios::beg);
        auto out = ""s;
        out.resize(convert_to_size(pos));
        file.read(out.data(), out.size());
        copy(finalize_read(std::move(out)), std::move(value));

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Common::Store(Transaction values, Bucket bucket) const noexcept -> bool
{
    return store(*data_.lock(), values, bucket);
}

auto Common::store(Data& data, Transaction values, Bucket bucket) const noexcept
    -> bool
{
    if (values.first.empty()) { return true; }

    const auto count = values.first.size();

    for (auto n = 0_uz; n < count; ++n) {
        const auto& key = values.first[n];
        const auto& value = values.second[n];
        auto directory = fs::path{};
        const auto file = calculate_path(
            data, std::visit(EncodedView{}, key), bucket, directory);

        if (false == write_file(directory, file, value)) { return false; }
    }

    return true;
}

auto Common::sync(const fs::path& path) noexcept -> bool
{
    if (auto fd = FileDescriptor{path}; fd) {

        return sync(fd);
    } else {
        LogError()(OT_PRETTY_STATIC(Common))("Failed to open ")(path).Flush();

        return false;
    }
}

auto Common::sync(File& file) noexcept -> bool { return sync(file->handle()); }

// NOTE: Common::sync(DescriptorType::handle_type) defined in src/util/platform

auto Common::write_file(
    const fs::path& directory,
    const fs::path& filename,
    ReadView contents) const noexcept -> bool
{
    try {
        if (filename.empty()) { throw std::runtime_error{"empty filename"}; }

        auto file = File{filename.string()};

        if (false == file.good()) {

            throw std::runtime_error{
                "unable to open "s.append(filename.string())
                    .append(" for writing")};
        }

        do_write(directory, filename, file, contents);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Common::finalize_write(
    const fs::path& directory,
    const fs::path& filename,
    File& file,
    ReadView data) noexcept(false) -> void
{
    file.write(data.data(), data.size());

    if (false == sync(file)) {

        throw std::runtime_error{"error syncing "s.append(filename.string())};
    }

    if (false == sync(directory)) {

        throw std::runtime_error{"error syncing "s.append(directory.string())};
    }

    file.close();
}

Common::~Common() = default;
}  // namespace opentxs::storage::driver::filesystem
