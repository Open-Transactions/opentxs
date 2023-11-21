// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Session.hpp"  // IWYU pragma: associated

#include "opentxs/api/Session.internal.hpp"

namespace opentxs::api
{
Session::Session(internal::Session* imp) noexcept
    : imp_(imp)
{
}

auto Session::Cancel(TaskID task) const noexcept -> bool
{
    return imp_->Cancel(task);
}

auto Session::Config() const noexcept -> const api::Settings&
{
    return imp_->Config();
}

auto Session::Crypto() const noexcept -> const session::Crypto&
{
    return imp_->Crypto();
}

auto Session::DataFolder() const noexcept -> const std::filesystem::path&
{
    return imp_->DataFolder();
}

auto Session::Endpoints() const noexcept -> const session::Endpoints&
{
    return imp_->Endpoints();
}

auto Session::Factory() const noexcept -> const session::Factory&
{
    return imp_->Factory();
}

auto Session::GetOptions() const noexcept -> const Options&
{
    return imp_->GetOptions();
}

auto Session::Instance() const noexcept -> int { return imp_->Instance(); }

auto Session::Internal() const noexcept -> const internal::Session&
{
    return *imp_;
}

auto Session::Internal() noexcept -> internal::Session& { return *imp_; }

auto Session::Network() const noexcept -> const network::Network&
{
    return imp_->Network();
}

auto Session::QtRootObject() const noexcept -> QObject*
{
    return imp_->QtRootObject();
}

auto Session::Reschedule(TaskID task, std::chrono::seconds interval)
    const noexcept -> bool
{
    return imp_->Reschedule(task, interval);
}

auto Session::Schedule(
    std::chrono::seconds interval,
    opentxs::SimpleCallback task) const noexcept -> TaskID
{
    return imp_->Schedule(interval, task);
}

auto Session::Schedule(
    std::chrono::seconds interval,
    opentxs::SimpleCallback task,
    std::chrono::seconds last) const noexcept -> TaskID
{
    return imp_->Schedule(interval, task, last);
}

auto Session::SetMasterKeyTimeout(
    const std::chrono::seconds& timeout) const noexcept -> void
{
    return imp_->SetMasterKeyTimeout(timeout);
}

auto Session::Storage() const noexcept -> const session::Storage&
{
    return imp_->Storage();
}

auto Session::Wallet() const noexcept -> const session::Wallet&
{
    return imp_->Wallet();
}

Session::~Session() = default;
}  // namespace opentxs::api
