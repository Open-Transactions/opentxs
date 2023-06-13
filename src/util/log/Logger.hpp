// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace internal
{
class Logger;
}  // namespace internal
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{

auto GetLogger() noexcept -> std::shared_ptr<internal::Logger>;
}  // namespace opentxs

namespace opentxs::internal
{
class Logger
{
public:
    using Source = std::pair<std::string, network::zeromq::socket::Raw>;

    std::atomic_int verbosity_{-1};

    auto Session() const noexcept -> int;

    auto Register(const std::thread::id id) noexcept
        -> std::pair<int, std::shared_ptr<Source>>;
    auto Start() noexcept -> void;
    auto Stop() noexcept -> void;
    auto Unregister(const std::thread::id id) noexcept -> void;

private:
    struct Data {
        bool disabled_{true};
        int session_counter_{-1};
        Map<std::thread::id, std::shared_ptr<Source>> map_{};
    };

    libguarded::shared_guarded<Data, std::shared_mutex> data_{};
};
}  // namespace opentxs::internal
