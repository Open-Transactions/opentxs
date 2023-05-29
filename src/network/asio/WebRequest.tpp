// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/asio/WebRequest.hpp"  // IWYU pragma: associated

#include "BoostAsio.hpp"
#include "api/network/asio/Context.hpp"  // IWYU pragma: keep

namespace opentxs::network::asio
{
template <typename CRTP>
WebRequest<CRTP>::WebRequest(
    const std::string_view hostname,
    const std::string_view file,
    api::network::asio::Context& asio,
    Finish&& cb,
    allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
    , hostname_([&] {
        auto out = CString{allocator_};
        out.assign(hostname);

        return out;
    }())
    , file_([&] {
        auto out = CString{allocator_};
        out.assign(file);

        return out;
    }())
    , asio_(asio)
    , promise_is_set_(false)
    , promise_()
    , cb_(std::move(cb))
    , resolver_(asio_.get())
    , resolved_(std::nullopt)
    , request_(std::nullopt)
    , buffer_()
    , response_()
{
}

template <typename CRTP>
auto WebRequest<CRTP>::connect() noexcept -> void
{
    try {
        downcast().get_stream_connect().async_connect(
            resolved_.value(), [job = sp()](const auto& ec, auto) {
                if (ec) {
                    const auto error =
                        CString{job->allocator_} +
                        ((beast::error::timeout == ec)
                             ? "Timed out connecting to host: " + job->hostname_
                             : "Failed to connect to host: " + job->hostname_ +
                                   ", Error: " + ec.message().c_str());
                    job->promise_.set_exception(std::make_exception_ptr(
                        std::runtime_error{error.c_str()}));
                    job->promise_is_set_ = true;
                } else {
                    job->downcast().step_after_connect();
                }
            });
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;
    }
}

template <typename CRTP>
auto WebRequest<CRTP>::finish() noexcept -> void
{
    if (false == promise_is_set_) {
        promise_.set_exception(std::make_exception_ptr(
            std::runtime_error{"asio operation interrupted"}));
        promise_is_set_ = true;
    }

    if (cb_) {
        cb_(promise_.get_future());
        cb_ = {};
    }
}

template <typename CRTP>
auto WebRequest<CRTP>::get_response() noexcept -> void
{
    try {
        http::async_read(
            downcast().get_stream(),
            buffer_,
            response_,
            [job = sp()](const auto& ec, auto) {
                if (ec) {
                    const auto error =
                        CString{job->allocator_} +
                        "Failed to receive GET response from host: " +
                        job->hostname_ + ", Error: " + ec.message().c_str();
                    job->promise_.set_exception(std::make_exception_ptr(
                        std::runtime_error{error.c_str()}));
                    job->promise_is_set_ = true;
                } else {
                    job->promise_.set_value(std::move(job->response_));
                    job->promise_is_set_ = true;
                }
            });
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;
    }
}

template <typename CRTP>
auto WebRequest<CRTP>::request() noexcept -> void
{
    try {
        auto& request = request_.emplace();
        request.version(11);
        request.method(http::verb::get);
        request.target(file_);
        request.set(http::field::host, hostname_);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::async_write(
            downcast().get_stream(),
            request,
            [job = sp()](const auto& ec, auto) {
                if (ec) {
                    const auto error = CString{job->allocator_} +
                                       "Failed to send GET request to host: " +
                                       job->hostname_ +
                                       ", Error: " + ec.message().c_str();
                    job->promise_.set_exception(std::make_exception_ptr(
                        std::runtime_error{error.c_str()}));
                    job->promise_is_set_ = true;
                } else {
                    job->get_response();
                }
            });
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;

        return;
    }
}

template <typename CRTP>
auto WebRequest<CRTP>::resolve(const std::string_view service) noexcept -> void
{
    try {
        resolver_.async_resolve(
            hostname_, service, [job = sp()](const auto& ec, auto results) {
                if (ec) {
                    const auto error =
                        CString{job->allocator_} +
                        "Failed to resolve host: " + job->hostname_ +
                        ", Error: " + ec.message().c_str();
                    job->promise_.set_exception(std::make_exception_ptr(
                        std::runtime_error{error.c_str()}));
                    job->promise_is_set_ = true;
                } else {
                    job->resolved_.emplace(std::move(results));
                    job->connect();
                }
            });
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;
    }
}

template <typename CRTP>
WebRequest<CRTP>::~WebRequest()
{
    finish();
}
}  // namespace opentxs::network::asio
