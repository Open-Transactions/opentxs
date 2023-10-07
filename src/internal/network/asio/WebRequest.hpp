// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/intrusive/list.hpp>

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string_view>

#include "BoostAsio.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/util/Container.hpp"

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
template <typename CRTP>
class WebRequest : public opentxs::pmr::Allocated,
                   public std::enable_shared_from_this<CRTP>
{
public:
    using Response = http::response<http::string_body>;
    using Future = std::future<Response>;
    using Finish = std::function<void(Future&&)>;

    ~WebRequest() override;

protected:
    const CString hostname_;
    const CString file_;
    api::network::asio::Context& asio_;
    bool promise_is_set_;
    std::promise<Response> promise_;

    auto request() noexcept -> void;
    auto resolve(const std::string_view service) noexcept -> void;

    WebRequest(
        const std::string_view hostname,
        const std::string_view file,
        api::network::asio::Context& asio,
        Finish&& cb,
        allocator_type alloc) noexcept;

private:
    using Buffer = beast::flat_buffer;
    using Request = http::request<http::empty_body>;
    using Resolver = ip::tcp::resolver;

    Finish cb_;
    Resolver resolver_;
    std::optional<Resolver::results_type> resolved_;
    std::optional<Request> request_;
    Buffer buffer_;
    Response response_;

    auto connect() noexcept -> void;
    inline auto downcast() noexcept -> CRTP&
    {
        return static_cast<CRTP&>(*this);
    }
    auto finish() noexcept -> void;
    auto get_response() noexcept -> void;
    inline auto sp() noexcept { return this->shared_from_this(); }
};
#pragma GCC diagnostic pop
}  // namespace opentxs::network::asio
