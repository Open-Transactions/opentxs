// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/alloc/Logging.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
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

    auto handle = log_.lock();
    auto& log = *handle;

    if (log.is_open()) { std::invoke(op, log); }
}
}  // namespace opentxs::alloc

namespace opentxs::alloc
{
Logging::Logging(
    const std::filesystem::path& logfile,
    bool write,
    Resource* upstream) noexcept
    : file_(logfile)
    , total_()
    , current_()
    , upstream_(upstream)
    , log_([&] {
        auto out = std::ofstream{logfile, std::ios::out | std::ios::trunc};
        out << "action,change,current,total\n";

        return out;
    }())
    , write_(write)
{
    if (nullptr == upstream) { std::terminate(); }
}

auto Logging::close() noexcept -> void
{
    write(
        [this](auto& log) {
            log << "closed" << ',' << std::to_string(0) << ','
                << std::to_string(current_) << ',' << std::to_string(total_)
                << '\n';
            log.close();
        },
        true);
}

auto Logging::do_allocate(std::size_t bytes, std::size_t alignment) -> void*
{
    total_ += bytes;
    current_ += bytes;
    write([&, this](auto& log) {
        log << "allocate" << ',' << std::to_string(bytes) << ','
            << std::to_string(current_) << ',' << std::to_string(total_)
            << '\n';
    });

    return upstream_->allocate(bytes, alignment);
}

auto Logging::do_deallocate(void* p, std::size_t size, std::size_t alignment)
    -> void
{
    current_ -= size;
    write([&, this](auto& log) {
        log << "deallocate" << ',' << std::to_string(size) << ','
            << std::to_string(current_) << ',' << std::to_string(total_)
            << '\n';
    });

    return upstream_->deallocate(p, size, alignment);
}

auto Logging::do_is_equal(const Resource& other) const noexcept -> bool
{
    return std::addressof(other) == static_cast<const Resource*>(this);
}

auto Logging::set_name(std::string_view name) noexcept -> void
{
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
