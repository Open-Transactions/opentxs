// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/asio/HTTP.hpp"  // IWYU pragma: associated

#include <boost/intrusive/list.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <exception>
#include <future>
#include <ratio>
#include <utility>

#include "api/network/asio/Context.hpp"
#include "network/asio/WebRequest.tpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep

namespace opentxs::network::asio
{
HTTP::HTTP(
    const std::string_view hostname,
    const std::string_view file,
    api::network::asio::Context& asio,
    Finish&& cb,
    allocator_type alloc) noexcept
    : WebRequest(hostname, file, asio, std::move(cb), std::move(alloc))
    , stream_(std::nullopt)
{
}

auto HTTP::get_stream() noexcept(false) -> Stream&
{
    if (false == stream_.has_value()) {
        auto& stream = stream_.emplace(asio_.get());
        stream.expires_after(10s);
    }

    return stream_.value();
}

auto HTTP::Start() noexcept -> void
{
    try {
        resolve("http");
    } catch (...) {
        promise_.set_exception(std::current_exception());

        return;
    }
}

HTTP::~HTTP() = default;
}  // namespace opentxs::network::asio
