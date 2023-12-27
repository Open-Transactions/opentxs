// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Context.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/Signals.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/rpc/response/Message.hpp"  // IWYU pragma: keep

namespace opentxs::api
{
Context::Context(internal::Context* imp) noexcept
    : imp_(imp)
{
}

auto Context::Asio() const noexcept -> const network::Asio&
{
    return imp_->Asio();
}

auto Context::Cancel(TaskID task) const noexcept -> bool
{
    return imp_->Cancel(task);
}

auto Context::ClientSession(const int instance) const noexcept(false)
    -> const api::session::Client&
{
    return imp_->ClientSession(instance);
}

auto Context::ClientSessionCount() const noexcept -> std::size_t
{
    return imp_->ClientSessionCount();
}

auto Context::Config(const std::filesystem::path& path) const noexcept
    -> const api::Settings&
{
    return imp_->Config(path);
}

auto Context::Crypto() const noexcept -> const api::Crypto&
{
    return imp_->Crypto();
}

auto Context::Factory() const noexcept -> const api::Factory&
{
    return imp_->Factory();
}

auto Context::HandleSignals(SimpleCallback* callback) const noexcept -> void
{
    return imp_->HandleSignals(callback);
}

auto Context::Internal() const noexcept -> const internal::Context&
{
    return *imp_;
}

auto Context::Internal() noexcept -> internal::Context& { return *imp_; }

auto Context::NotarySession(const int instance) const noexcept(false)
    -> const session::Notary&
{
    return imp_->NotarySession(instance);
}

auto Context::NotarySessionCount() const noexcept -> std::size_t
{
    return imp_->NotarySessionCount();
}

auto Context::Options() const noexcept -> const opentxs::Options&
{
    return imp_->Options();
}

auto Context::PrepareSignalHandling() noexcept -> void { Signals::Block(); }

auto Context::ProfileId() const noexcept -> std::string_view
{
    return imp_->ProfileId();
}

auto Context::QtRootObject(QObject* parent) const noexcept -> QObject*
{
    return imp_->QtRootObject(parent);
}

auto Context::Reschedule(TaskID task, std::chrono::seconds interval)
    const noexcept -> bool
{
    return imp_->Reschedule(task, interval);
}

auto Context::RPC(const rpc::request::Message& command) const noexcept
    -> std::unique_ptr<rpc::response::Message>
{
    return imp_->RPC(command);
}

auto Context::RPC(const ReadView command, Writer&& response) const noexcept
    -> bool
{
    return imp_->RPC(command, std::move(response));
}

auto Context::Schedule(
    std::chrono::seconds interval,
    opentxs::SimpleCallback task) const noexcept -> TaskID
{
    return imp_->Schedule(interval, task);
}

auto Context::Schedule(
    std::chrono::seconds interval,
    opentxs::SimpleCallback task,
    std::chrono::seconds last) const noexcept -> TaskID
{
    return imp_->Schedule(interval, task, last);
}

auto Context::StartClientSession(
    const opentxs::Options& args,
    const int instance) const -> const api::session::Client&
{
    return imp_->StartClientSession(args, instance);
}

auto Context::StartClientSession(const int instance) const
    -> const api::session::Client&
{
    return imp_->StartClientSession(instance);
}

auto Context::StartClientSession(
    const opentxs::Options& args,
    const int instance,
    std::string_view recoverWords,
    std::string_view recoverPassphrase) const -> const api::session::Client&
{
    return imp_->StartClientSession(
        args, instance, recoverWords, recoverPassphrase);
}

auto Context::StartNotarySession(
    const opentxs::Options& args,
    const int instance) const -> const session::Notary&
{
    return imp_->StartNotarySession(args, instance);
}

auto Context::StartNotarySession(const int instance) const
    -> const session::Notary&
{
    return imp_->StartNotarySession(instance);
}

auto Context::SuggestFolder(std::string_view appName) noexcept
    -> std::filesystem::path
{
    return internal::Paths::SuggestFolder(appName);
}
auto Context::ZAP() const noexcept -> const api::network::ZAP&
{
    return imp_->ZAP();
}

auto Context::ZMQ() const noexcept -> const opentxs::network::zeromq::Context&
{
    return imp_->ZMQ();
}

Context::~Context() = default;
}  // namespace opentxs::api
