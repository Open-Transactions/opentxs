// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/ZeroMQ.hpp"  // IWYU pragma: associated

#include "opentxs/api/session/ZeroMQ.internal.hpp"

namespace opentxs::api::session
{
ZeroMQ::ZeroMQ(internal::ZeroMQ* imp) noexcept
    : imp_(imp)
{
}

auto ZeroMQ::Context() const noexcept
    -> const opentxs::network::zeromq::Context&
{
    return imp_->Context();
}

auto ZeroMQ::DefaultAddressType() const noexcept -> AddressType
{
    return imp_->DefaultAddressType();
}

auto ZeroMQ::Internal() const noexcept -> const internal::ZeroMQ&
{
    return *imp_;
}

auto ZeroMQ::Internal() noexcept -> internal::ZeroMQ& { return *imp_; }

auto ZeroMQ::KeepAlive() const noexcept -> std::chrono::seconds
{
    return imp_->KeepAlive();
}

auto ZeroMQ::KeepAlive(const std::chrono::seconds duration) const noexcept
    -> void
{
    return imp_->KeepAlive(duration);
}

auto ZeroMQ::Linger() const noexcept -> std::chrono::seconds
{
    return imp_->Linger();
}

auto ZeroMQ::ReceiveTimeout() const noexcept -> std::chrono::seconds
{
    return imp_->ReceiveTimeout();
}

auto ZeroMQ::Running() const noexcept -> const Flag& { return imp_->Running(); }

auto ZeroMQ::RefreshConfig() const noexcept -> void
{
    return imp_->RefreshConfig();
}

auto ZeroMQ::SendTimeout() const noexcept -> std::chrono::seconds
{
    return imp_->SendTimeout();
}

auto ZeroMQ::Server(const identifier::Notary& id) const noexcept(false)
    -> opentxs::network::ServerConnection&
{
    return imp_->Server(id);
}

auto ZeroMQ::SetSocksProxy(const UnallocatedCString& proxy) const noexcept
    -> bool
{
    return imp_->SetSocksProxy(proxy);
}

auto ZeroMQ::SocksProxy() const noexcept -> UnallocatedCString
{
    return imp_->SocksProxy();
}

auto ZeroMQ::SocksProxy(UnallocatedCString& proxy) const noexcept -> bool
{
    return imp_->SocksProxy(proxy);
}

auto ZeroMQ::Status(const identifier::Notary& id) const noexcept
    -> opentxs::network::ConnectionState
{
    return imp_->Status(id);
}

ZeroMQ::~ZeroMQ()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::session
