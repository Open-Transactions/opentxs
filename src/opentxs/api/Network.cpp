// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Network.hpp"  // IWYU pragma: associated

#include "opentxs/api/Network.internal.hpp"

namespace opentxs::api
{
Network::Network(internal::Network* imp) noexcept
    : imp_(imp)
{
}

auto Network::Asio() const noexcept -> const network::Asio&
{
    return imp_->Asio();
}

auto Network::Blockchain() const noexcept -> const network::Blockchain&
{
    return imp_->Blockchain();
}

auto Network::Internal() const noexcept -> const internal::Network&
{
    return *imp_;
}

auto Network::Internal() noexcept -> internal::Network& { return *imp_; }

auto Network::OTDHT() const noexcept -> const network::OTDHT&
{
    return imp_->OTDHT();
}

auto Network::ZeroMQ() const noexcept -> const network::ZeroMQ&
{
    return imp_->ZeroMQ();
}

Network::~Network()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api
