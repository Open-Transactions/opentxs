// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "BoostAsio.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/util/Types.hpp"

namespace asio = boost::asio;
namespace ip = asio::ip;

namespace opentxs::network::asio
{
struct Endpoint::Imp {
    using tcp = ip::tcp;

    const Type type_;
    const tcp::endpoint data_;
    const Space bytes_;

    Imp(Type type, ReadView raw, Port port) noexcept(false);
    Imp() noexcept;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp();
};
}  // namespace opentxs::network::asio
