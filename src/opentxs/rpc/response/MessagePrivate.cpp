// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/MessagePrivate.hpp"  // IWYU pragma: associated

#include <RPCResponse.pb.h>
#include <RPCStatus.pb.h>
#include <RPCTask.pb.h>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/Types.internal.hpp"
#include "opentxs/rpc/request/MessagePrivate.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "opentxs/rpc/response/ListNyms.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::rpc::response
{
Message::Imp::Imp(
    const Message* parent,
    VersionNumber version,
    const UnallocatedCString& cookie,
    const CommandType& type,
    const Responses responses,
    SessionIndex session,
    Identifiers&& identifiers,
    Tasks&& tasks) noexcept
    : parent_(parent)
    , version_(version)
    , cookie_(cookie)
    , type_(type)
    , responses_(responses)
    , session_(session)
    , identifiers_(std::move(identifiers))
    , tasks_(std::move(tasks))
{
}

Message::Imp::Imp(
    const Message* parent,
    const request::Message& request,
    Responses&& response) noexcept
    : Imp(parent,
          request.Version(),
          request.Cookie(),
          request.Type(),
          std::move(response),
          request.Session(),
          {},
          {})
{
}

Message::Imp::Imp(
    const Message* parent,
    const request::Message& request,
    Responses&& response,
    Identifiers&& identifiers) noexcept
    : Imp(parent,
          request.Version(),
          request.Cookie(),
          request.Type(),
          std::move(response),
          request.Session(),
          std::move(identifiers),
          {})
{
}

Message::Imp::Imp(
    const Message* parent,
    const request::Message& request,
    Responses&& response,
    Tasks&& tasks) noexcept
    : Imp(parent,
          request.Version(),
          request.Cookie(),
          request.Type(),
          std::move(response),
          request.Session(),
          {},
          std::move(tasks))
{
}

Message::Imp::Imp(const Message* parent, const proto::RPCResponse& in) noexcept
    : Imp(
          parent,
          in.version(),
          in.cookie(),
          translate(in.type()),
          [&] {
              auto out = Responses{};

              for (const auto& status : in.status()) {
                  out.emplace_back(status.index(), translate(status.code()));
              }

              return out;
          }(),
          in.session(),
          [&] {
              auto out = Identifiers{};

              for (const auto& id : in.identifier()) { out.emplace_back(id); }

              return out;
          }(),
          [&] {
              auto out = Tasks{};

              for (const auto& task : in.task()) {
                  out.emplace_back(task.index(), task.id());
              }

              return out;
          }())
{
}

Message::Imp::Imp(const Message* parent) noexcept
    : Imp(parent, 0, {}, CommandType::error, {}, -1, {}, {})
{
}

Message::Message(std::unique_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
    assert(nullptr != imp_);
    assert(imp_->parent_ == this);
}

Message::Message() noexcept
    : Message(std::make_unique<Imp>(this))
{
}

auto Message::Imp::asGetAccountActivity() const noexcept
    -> const response::GetAccountActivity&
{
    static const auto blank = response::GetAccountActivity{};

    return blank;
}

auto Message::Imp::asGetAccountBalance() const noexcept
    -> const response::GetAccountBalance&
{
    static const auto blank = response::GetAccountBalance{};

    return blank;
}

auto Message::Imp::asListAccounts() const noexcept
    -> const response::ListAccounts&
{
    static const auto blank = response::ListAccounts{};

    return blank;
}

auto Message::Imp::asListNyms() const noexcept -> const response::ListNyms&
{
    static const auto blank = response::ListNyms{};

    return blank;
}

auto Message::Imp::asSendPayment() const noexcept
    -> const response::SendPayment&
{
    static const auto blank = response::SendPayment{};

    return blank;
}

auto Message::Imp::serialize(proto::RPCResponse& dest) const noexcept -> bool
{
    dest.set_version(version_);
    dest.set_cookie(cookie_);
    dest.set_type(translate(type_));

    for (const auto& [index, code] : responses_) {
        auto& status = *dest.add_status();
        status.set_version(status_version(type_, version_));
        status.set_index(index);
        status.set_code(translate(code));
    }

    dest.set_session(session_);

    return true;
}

auto Message::Imp::serialize(Writer&& dest) const noexcept -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::RPCResponse{};

            if (false == serialize(out)) {
                throw std::runtime_error{"serialization error"};
            }

            return out;
        }();

        return proto::write(proto, std::move(dest));
    } catch (...) {

        return false;
    }
}

auto Message::Imp::serialize_identifiers(
    proto::RPCResponse& dest) const noexcept -> void
{
    for (const auto& id : identifiers_) { dest.add_identifier(id); }
}

auto Message::Imp::serialize_tasks(proto::RPCResponse& dest) const noexcept
    -> void
{
    for (const auto& [index, id] : tasks_) {
        auto& task = *dest.add_task();
        task.set_version(task_version(type_, version_));
        task.set_index(index);
        task.set_id(id);
    }
}

auto Message::Imp::status_version(
    const CommandType& type,
    const VersionNumber parentVersion) noexcept -> VersionNumber
{
    return 2u;
}

auto Message::Imp::task_version(
    const CommandType& type,
    const VersionNumber parentVersion) noexcept -> VersionNumber
{
    return 1u;
}

auto Message::asGetAccountActivity() const noexcept -> const GetAccountActivity&
{
    return imp_->asGetAccountActivity();
}

auto Message::asGetAccountBalance() const noexcept -> const GetAccountBalance&
{
    return imp_->asGetAccountBalance();
}

auto Message::asListAccounts() const noexcept -> const ListAccounts&
{
    return imp_->asListAccounts();
}

auto Message::asListNyms() const noexcept -> const ListNyms&
{
    return imp_->asListNyms();
}

auto Message::asSendPayment() const noexcept -> const SendPayment&
{
    return imp_->asSendPayment();
}

auto Message::Cookie() const noexcept -> const UnallocatedCString&
{
    return imp_->cookie_;
}

auto Message::ResponseCodes() const noexcept -> const Responses&
{
    return imp_->responses_;
}

auto Message::Serialize(Writer&& dest) const noexcept -> bool
{
    return imp_->serialize(std::move(dest));
}

auto Message::Serialize(proto::RPCResponse& dest) const noexcept -> bool
{
    return imp_->serialize(dest);
}

auto Message::Session() const noexcept -> SessionIndex
{
    return imp_->session_;
}

auto Message::Type() const noexcept -> CommandType { return imp_->type_; }

auto Message::Version() const noexcept -> VersionNumber
{
    return imp_->version_;
}

Message::~Message() = default;
}  // namespace opentxs::rpc::response
