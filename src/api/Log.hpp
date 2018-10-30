// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Log : virtual public api::internal::Log
{
public:
    explicit Log(
        const opentxs::network::zeromq::Context& zmq,
        const std::string& endpoint);

    ~Log() = default;

private:
    friend api::Factory;

    const opentxs::network::zeromq::Context& zmq_;
    OTZMQListenCallback callback_;
    OTZMQPullSocket socket_;
    OTZMQPublishSocket publish_socket_;
    const bool publish_;

    void callback(opentxs::network::zeromq::Message& message);
    void print(
        const int level,
        const std::string& text,
        const std::string& thread);
#ifdef ANDROID
    void print_android(
        const int level,
        const std::string& text,
        const std::string& thread);
#endif

    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;
};
}  // namespace opentxs::api::implementation
