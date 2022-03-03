// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/network/asio/Acceptor.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/asio/Endpoint.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace ip = boost::asio::ip;

namespace opentxs::api::network::asio
{
struct Acceptor::Imp {
    static constexpr auto backlog_size_{8};

    const opentxs::network::asio::Endpoint& endpoint_;
    const Callback cb_;
    mutable std::mutex lock_;
    bool running_;
    internal::Asio& asio_;
    boost::asio::io_context& ios_;
    ip::tcp::acceptor acceptor_;
    ip::tcp::socket next_socket_;

    auto Start() noexcept -> void
    {
        auto lock = Lock{lock_};
        start(lock);
    }
    auto Stop() noexcept -> void
    {
        auto lock = Lock{lock_};

        if (running_) {
            LogTrace()(OT_PRETTY_CLASS())("shutting down ")(endpoint_.str())
                .Flush();
            auto ec = boost::system::error_code{};
            acceptor_.cancel(ec);
            acceptor_.close(ec);
            running_ = false;
            LogTrace()(OT_PRETTY_CLASS())(endpoint_.str())(" closed").Flush();
        }
    }

    Imp(const opentxs::network::asio::Endpoint& endpoint,
        internal::Asio& asio,
        boost::asio::io_context& ios,
        Callback&& cb) noexcept(false)
        : endpoint_(endpoint)
        , cb_(std::move(cb))
        , lock_()
        , running_(false)
        , asio_(asio)
        , ios_(ios)
        , acceptor_(ios_, endpoint_.GetInternal().data_.protocol())
        , next_socket_(ios_)
    {
        if (!cb_) { throw std::runtime_error{"Invalid callback"}; }

        acceptor_.bind(endpoint_.GetInternal().data_);
        acceptor_.listen(backlog_size_);
    }

    ~Imp() { Stop(); }

private:
    auto handler(const boost::system::error_code& ec) noexcept -> void
    {
        if (ec) {
            using Error = boost::system::errc::errc_t;
            const auto error = ec.value();

            switch (error) {
                case Error::operation_canceled: {
                } break;
                default: {
                    LogError()(OT_PRETTY_CLASS())("error ")(error)(", ")(
                        ec.message())
                        .Flush();
                }
            }

            return;
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("incoming connection request on ")(
                endpoint_.str())
                .Flush();
        }

        auto lock = Lock{lock_};
        const auto bytes = [&] {
            const auto address = next_socket_.remote_endpoint().address();

            if (address.is_v4()) {
                const auto bytes = address.to_v4().to_bytes();
                auto* it = reinterpret_cast<const std::byte*>(bytes.data());

                return Space{it, std::next(it, bytes.size())};
            } else {
                const auto bytes = address.to_v6().to_bytes();
                auto* it = reinterpret_cast<const std::byte*>(bytes.data());

                return Space{it, std::next(it, bytes.size())};
            }
        }();
        auto endpoint = opentxs::network::asio::Endpoint{
            endpoint_.GetType(), reader(bytes), endpoint_.GetPort()};
        cb_(opentxs::network::asio::Socket{
            std::make_unique<opentxs::network::asio::Socket::Imp>(
                asio_, std::move(endpoint), std::move(next_socket_))
                .release()});
        next_socket_ = ip::tcp::socket{ios_};
        start(lock);
    }
    auto start(const Lock&) noexcept -> void
    {
        acceptor_.async_accept(
            next_socket_, [this](const auto& ec) { handler(ec); });
        running_ = true;
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Acceptor::Acceptor(
    const opentxs::network::asio::Endpoint& endpoint,
    internal::Asio& asio,
    boost::asio::io_context& ios,
    Callback&& cb)
    : imp_(std::make_unique<Imp>(endpoint, asio, ios, std::move(cb)).release())
{
}

auto Acceptor::Start() noexcept -> void { imp_->Start(); }

auto Acceptor::Stop() noexcept -> void { imp_->Stop(); }

Acceptor::~Acceptor()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network::asio
