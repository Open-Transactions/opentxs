// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/asio/HTTPS.hpp"  // IWYU pragma: associated

#include <boost/intrusive/list.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <openssl/err.h>
#include <openssl/ssl3.h>
#include <chrono>
#include <compare>
#include <exception>
#include <future>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <utility>

#include "api/network/asio/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/asio/WebRequest.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::network::asio
{
using namespace std::literals;

HTTPS::HTTPS(
    const std::string_view hostname,
    const std::string_view file,
    api::network::asio::Context& asio,
    Finish&& cb,
    allocator_type alloc) noexcept
    : WebRequest(hostname, file, asio, std::move(cb), std::move(alloc))
    , ssl_([this] {
        // NOTE Initializing an ssl::context works 100% of the time with
        // versions of OpenSSL < 3, but randomly fails with versions of OpenSSL
        // >= 3 apparently due to some race condition in the SSL library
        // initialization ("context: library has no ciphers (SSL routines)")
        // which their documentation implies can't be a problem because the
        // library initialization happens automagically now.
        //
        // Alternately it might be an Asio / Beast incompatibility with
        // OpenSSL 3 instead of an OpenSSL bug.
        static constexpr auto max = 16s;
        auto retry = 1s;

        while (retry <= max) {
            try {
                auto out = ssl::context{ssl::context::tlsv13_client};
                // clang-format off
                out.set_options(
                    ssl::context::default_workarounds |
                    ssl::context::no_compression |
                    ssl::context::no_sslv2 |
                    ssl::context::no_sslv3 |
                    ssl::context::no_tlsv1 |
                    ssl::context::no_tlsv1_1 |
                    ssl::context::no_tlsv1_2);
                // clang-format on
                out.set_default_verify_paths();
                out.set_verify_mode(ssl::verify_peer);

                return out;
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            }

            Sleep(retry);
            retry *= 2;
        }

        LogAbort()(OT_PRETTY_CLASS())("failed to initialize ssl context")
            .Abort();
    }())
    , stream_(std::nullopt)
{
}

auto HTTPS::get_stream() noexcept(false) -> Stream&
{
    if (false == stream_.has_value()) {
        auto& stream = stream_.emplace(asio_.get(), ssl_);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
        // NOTE const_cast necessary to avoid -Wcast-qual warnings on android /
        // libressl. This should be removed when android environment is updated
        // to openssl-3
        const auto rc = ::SSL_set_tlsext_host_name(
            stream.native_handle(), const_cast<char*>(hostname_.c_str()));

        if (false == rc) {
            const auto ec = beast::error_code{
                static_cast<int>(::ERR_get_error()),
                boost::asio::error::get_ssl_category()};
            const auto error =
                CString{allocator_} +
                "Error calling SSL_set_tlsext_host_name for host: " +
                hostname_ + ", Error: " + ec.message().c_str();

            throw std::runtime_error{error.c_str()};
        }
#pragma GCC diagnostic pop

        static constexpr auto timeout = 10s;
        beast::get_lowest_layer(stream).expires_after(timeout);
    }

    return stream_.value();
}

auto HTTPS::get_stream_connect() noexcept(false) -> beast::tcp_stream&
{
    return beast::get_lowest_layer(get_stream());
}

auto HTTPS::handshake() noexcept -> void
{
    try {
        get_stream().async_handshake(
            ssl::stream_base::client,
            [job = shared_from_this()](const auto& ec) {
                if (ec) {
                    const auto error =
                        CString{job->allocator_} +
                        "Handshake failed to host: " + job->hostname_ +
                        ", Error: " + ec.message().c_str();
                    job->promise_.set_exception(std::make_exception_ptr(
                        std::runtime_error{error.c_str()}));
                    job->promise_is_set_ = true;
                } else {
                    job->request();
                }
            });
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;

        return;
    }
}

auto HTTPS::load_root_certificates() noexcept(false) -> void
{
    auto ec = boost::system::error_code{};

    for (const auto& cert : ssl_certs()) {
        ssl_.add_certificate_authority(
            boost::asio::buffer(cert.data(), cert.size()), ec);
    }

    if (ec) {
        const auto error =
            CString{allocator_} +
            "Error loading root certificates, Error: " + ec.message().c_str();

        throw std::runtime_error{error.c_str()};
    }
}

auto HTTPS::Start() noexcept -> void
{
    try {
        load_root_certificates();
        resolve("https");
    } catch (...) {
        promise_.set_exception(std::current_exception());
        promise_is_set_ = true;

        return;
    }
}

HTTPS::~HTTPS()
{
    try {
        if (stream_.has_value()) {
            stream_.value().shutdown();
            stream_ = std::nullopt;
        }
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}
}  // namespace opentxs::network::asio
