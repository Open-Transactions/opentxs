// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/network/asio/WebRequest.hpp"
#include "network/asio/WebRequest.tpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace ip = boost::asio::ip;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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

namespace network
{
namespace asio
{
class HTTP;
}  // namespace asio
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::network::asio::HTTP final : public WebRequest<HTTP>
{
public:
    auto Start() noexcept -> void;

    HTTP(
        const std::string_view hostname,
        const std::string_view file,
        api::network::asio::Context& asio,
        Finish&& cb,
        allocator_type alloc = {})
    noexcept;
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
