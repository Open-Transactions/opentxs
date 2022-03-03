// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Lockable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40
#define SHUTDOWN_SOCKET                                                        \
    {                                                                          \
        running_->Off();                                                       \
        Lock lock(lock_);                                                      \
        shutdown(lock);                                                        \
    }

namespace opentxs::network::zeromq::socket::implementation
{
class Socket : virtual public internal::Socket, public Lockable
{
public:
    using SocketCallback = std::function<bool(const Lock& lock)>;

    static auto send_message(
        const Lock& lock,
        void* socket,
        zeromq::Message&& message) noexcept -> bool;
    static auto receive_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message) noexcept -> bool;

    auto ID() const noexcept -> std::size_t final { return id_; }
    auto Type() const noexcept -> socket::Type final { return type_; }

    operator void*() const noexcept final;

    virtual auto apply_socket(SocketCallback&& cb) const noexcept -> bool;
    auto Close() const noexcept -> bool override;
    auto Context() const noexcept -> const zeromq::Context& final
    {
        return context_;
    }
    auto SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept -> bool final;
    auto Start(const std::string_view endpoint) const noexcept -> bool override;
    auto StartAsync(const std::string_view endpoint) const noexcept
        -> void final;

    auto get() -> Socket& { return *this; }

    ~Socket() override;

protected:
    struct Endpoints {
        std::mutex lock_{};
        std::queue<CString> queue_{};

        auto pop() noexcept -> Vector<CString>
        {
            auto output = Vector<CString>{};

            Lock lock{lock_};
            output.reserve(queue_.size());

            while (false == queue_.empty()) {
                output.emplace_back(queue_.front());
                queue_.pop();
            }

            return output;
        }
    };

    const zeromq::Context& context_;
    const Direction direction_;
    const std::size_t id_;
    mutable void* socket_;
    mutable std::atomic<int> linger_;
    mutable std::atomic<int> send_timeout_;
    mutable std::atomic<int> receive_timeout_;
    mutable std::mutex endpoint_lock_;
    mutable Set<CString> endpoints_;
    mutable OTFlag running_;
    mutable Endpoints endpoint_queue_;

    auto add_endpoint(const std::string_view endpoint) const noexcept -> void;
    auto apply_timeouts(const Lock& lock) const noexcept -> bool;
    auto bind(const Lock& lock, const std::string_view endpoint) const noexcept
        -> bool;
    auto connect(const Lock& lock, const std::string_view endpoint)
        const noexcept -> bool;
    auto receive_message(const Lock& lock, zeromq::Message& message)
        const noexcept -> bool;
    auto send_message(const Lock& lock, zeromq::Message&& message)
        const noexcept -> bool;
    auto set_socks_proxy(const UnallocatedCString& proxy) const noexcept
        -> bool;
    auto start(const Lock& lock, const std::string_view endpoint) const noexcept
        -> bool;

    virtual void init() noexcept {}
    virtual void shutdown(const Lock& lock) noexcept;

    explicit Socket(
        const zeromq::Context& context,
        const socket::Type type,
        const Direction direction) noexcept;

private:
    const socket::Type type_;

    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
