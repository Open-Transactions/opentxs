// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/alloc/Logging.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_set.h>
#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::alloc
{
template <typename Operation>
auto Logging::write(Operation op, bool close) noexcept -> void
{
    if (close) {
        if (false == write_.exchange(false)) { return; }
    } else {
        if (false == write_.load()) { return; }
    }

    auto handle = data_.lock();
    auto& data = *handle;
    auto& opt = data.log_;

    if (false == opt.has_value()) {
        auto& log = opt.emplace(file_, std::ios::out | std::ios::trunc);
        log << "action,change,current,max,total\n";
    }

    std::invoke(op, data);
}
}  // namespace opentxs::alloc

namespace opentxs::alloc
{
Logging::Logging(
    const std::filesystem::path& logfile,
    bool write,
    Resource* upstream) noexcept
    : file_(logfile)
    , upstream_(upstream)
    , write_(write)
    , data_()
{
    if (nullptr == upstream) { std::terminate(); }

    this->write([](auto&) {});
}

auto Logging::close() noexcept -> void
{
    write(
        [](auto& data) {
            const auto& current = data.current_;
            const auto& max = data.max_;
            const auto& total = data.total_;
            auto& log = *data.log_;
            log << "closed" << ',' << std::to_string(0) << ','
                << std::to_string(current) << ',' << std::to_string(max) << ','
                << std::to_string(total) << '\n';
            log.close();
            data.log_.reset();
        },
        true);
}

auto Logging::do_allocate(std::size_t bytes, std::size_t alignment) -> void*
{
    write([&](auto& data) {
        auto& current = data.current_;
        auto& max = data.max_;
        auto& total = data.total_;
        auto& log = *data.log_;
        total += bytes;
        current += bytes;
        max = std::max(max, current);
        log << "allocate" << ',' << std::to_string(bytes) << ','
            << std::to_string(current) << ',' << std::to_string(max) << ','
            << std::to_string(total) << '\n';
    });

    return upstream_->allocate(bytes, alignment);
}

auto Logging::do_deallocate(void* p, std::size_t size, std::size_t alignment)
    -> void
{
    write([&](auto& data) {
        auto& current = data.current_;
        const auto& max = data.max_;
        const auto& total = data.total_;
        auto& log = *data.log_;
        current -= size;
        log << "deallocate" << ',' << std::to_string(size) << ','
            << std::to_string(current) << ',' << std::to_string(max) << ','
            << std::to_string(total) << '\n';
    });

    return upstream_->deallocate(p, size, alignment);
}

auto Logging::do_is_equal(const Resource& other) const noexcept -> bool
{
    return std::addressof(other) == static_cast<const Resource*>(this);
}

auto Logging::set_name(std::string_view name) noexcept -> void
{
    if (false == write_.load()) { return; }

    constexpr auto replace = frozen::make_unordered_set<char>(
        {'<', '>', ':', '"', '/', '\\', '|', '?', '*'});
    auto link = UnallocatedCString{};
    std::transform(
        name.begin(), name.end(), std::back_inserter(link), [&](const auto c) {
            if (0_uz < replace.count(c)) {

                return '_';
            } else {

                return c;
            }
        });
    const auto symlink =
        (file_.parent_path() / link).replace_extension(file_.extension());
    std::filesystem::remove_all(symlink);
    std::filesystem::create_symlink(file_, symlink);
}

Logging::~Logging() { close(); }
}  // namespace opentxs::alloc
