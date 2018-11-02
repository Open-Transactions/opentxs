// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Socket.hpp"

#include <map>
#include <mutex>
#include <set>

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40
#define SHUTDOWN                                                               \
    {                                                                          \
        running_->Off();                                                       \
        Lock lock(lock_);                                                      \
        shutdown(lock);                                                        \
    }

namespace opentxs::network::zeromq::socket::implementation
{
class Socket : virtual public zeromq::Socket, public Lockable
{
public:
    static bool send_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message);
    static std::string random_inproc_endpoint();
    static bool receive_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message);

    SocketType Type() const override;

    operator void*() const override;
    operator std::mutex&() { return lock_; }

    bool Close() const override;
    const zeromq::Context& Context() const override { return context_; }
    bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const override;
    bool Start(const std::string& endpoint) const override;

    Socket& get() { return *this; }

    virtual ~Socket() = default;

protected:
    const zeromq::Context& context_;
    const Socket::Direction direction_;
    void* socket_{nullptr};
    mutable int linger_{0};
    mutable int send_timeout_{-1};
    mutable int receive_timeout_{-1};
    mutable std::set<std::string> endpoints_;
    OTFlag running_;

    bool apply_timeouts(const Lock& lock) const;
    bool bind(const Lock& lock, const std::string& endpoint) const;
    bool connect(const Lock& lock, const std::string& endpoint) const;
    bool receive_message(const Lock& lock, zeromq::Message& message) const;
    bool send_message(const Lock& lock, zeromq::Message& message) const;
    bool set_socks_proxy(const std::string& proxy) const;
    bool start(const Lock& lock, const std::string& endpoint) const;
    bool start_client(const Lock& lock, const std::string& endpoint) const;

    virtual void init() {}
    virtual void shutdown(const Lock& lock);

    explicit Socket(
        const zeromq::Context& context,
        const SocketType type,
        const Socket::Direction direction);

private:
    static const std::map<SocketType, int> types_;

    const SocketType type_{SocketType::Error};

    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
