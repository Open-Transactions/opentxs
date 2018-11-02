// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/CurveServer.hpp"

#include "network/zeromq/socket/Socket.hpp"

#include <mutex>

namespace opentxs::network::zeromq::curve::implementation
{
class Server : virtual public zeromq::CurveServer
{
public:
    bool SetDomain(const std::string& domain) const override;
    bool SetPrivateKey(const OTPassword& key) const override;
    bool SetPrivateKey(const std::string& z85) const override;

protected:
    bool set_private_key(const void* key, const std::size_t keySize) const;

    Server(zeromq::socket::implementation::Socket& socket);

    virtual ~Server() = default;

private:
    zeromq::socket::implementation::Socket& parent_;

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace opentxs::network::zeromq::curve::implementation
