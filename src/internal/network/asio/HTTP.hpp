// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "BoostAsio.hpp"
#include "internal/network/asio/WebRequest.hpp"
#include "internal/util/PMR.hpp"
#include "network/asio/WebRequest.tpp"

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace ip = boost::asio::ip;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace asio
{
class Context;
}  // namespace asio
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::asio
{
class HTTP final : public WebRequest<HTTP>
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto Start() noexcept -> void;

    HTTP(
        const std::string_view hostname,
        const std::string_view file,
        api::network::asio::Context& asio,
        Finish&& cb,
        allocator_type alloc = {}) noexcept;
    HTTP(const HTTP&) = delete;
    HTTP(HTTP&) = delete;
    auto operator=(const HTTP&) -> HTTP& = delete;
    auto operator=(HTTP&) -> HTTP& = delete;

    ~HTTP() final;

private:
    friend WebRequest<HTTP>;
    using Stream = beast::tcp_stream;

    std::optional<Stream> stream_;

    auto get_stream() noexcept(false) -> Stream&;
    auto get_stream_connect() noexcept(false) -> Stream&
    {
        return get_stream();
    }
    auto step_after_connect() noexcept -> void { request(); }
};
}  // namespace opentxs::network::asio
