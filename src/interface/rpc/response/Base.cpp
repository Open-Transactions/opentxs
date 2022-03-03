// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/interface/rpc/response/Base.hpp"  // IWYU pragma: associated

#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "interface/rpc/response/Base.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "opentxs/interface/rpc/CommandType.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/interface/rpc/response/GetAccountActivity.hpp"
#include "opentxs/interface/rpc/response/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/response/ListAccounts.hpp"
#include "opentxs/interface/rpc/response/ListNyms.hpp"
#include "opentxs/interface/rpc/response/SendPayment.hpp"
#include "serialization/protobuf/RPCResponse.pb.h"
#include "serialization/protobuf/RPCStatus.pb.h"
#include "serialization/protobuf/RPCTask.pb.h"

namespace opentxs::rpc::response
{
Base::Imp::Imp(
    const Base* parent,
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

Base::Imp::Imp(
    const Base* parent,
    const request::Base& request,
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

Base::Imp::Imp(
    const Base* parent,
    const request::Base& request,
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

Base::Imp::Imp(
    const Base* parent,
    const request::Base& request,
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

Base::Imp::Imp(const Base* parent, const proto::RPCResponse& in) noexcept
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

Base::Imp::Imp(const Base* parent) noexcept
    : Imp(parent, 0, {}, CommandType::error, {}, -1, {}, {})
{
}

Base::Base(std::unique_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
    assert(nullptr != imp_);
    assert(imp_->parent_ == this);
}

Base::Base() noexcept
    : Base(std::make_unique<Imp>(this))
{
}

auto Base::Imp::asGetAccountActivity() const noexcept
    -> const response::GetAccountActivity&
{
    static const auto blank = response::GetAccountActivity{};

    return blank;
}

auto Base::Imp::asGetAccountBalance() const noexcept
    -> const response::GetAccountBalance&
{
    static const auto blank = response::GetAccountBalance{};

    return blank;
}

auto Base::Imp::asListAccounts() const noexcept -> const response::ListAccounts&
{
    static const auto blank = response::ListAccounts{};

    return blank;
}

auto Base::Imp::asListNyms() const noexcept -> const response::ListNyms&
{
    static const auto blank = response::ListNyms{};

    return blank;
}

auto Base::Imp::asSendPayment() const noexcept -> const response::SendPayment&
{
    static const auto blank = response::SendPayment{};

    return blank;
}

auto Base::Imp::serialize(proto::RPCResponse& dest) const noexcept -> bool
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

auto Base::Imp::serialize(AllocateOutput dest) const noexcept -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::RPCResponse{};

            if (false == serialize(out)) {
                throw std::runtime_error{"serialization error"};
            }

            return out;
        }();

        return proto::write(proto, dest);
    } catch (...) {

        return false;
    }
}

auto Base::Imp::serialize_identifiers(proto::RPCResponse& dest) const noexcept
    -> void
{
    for (const auto& id : identifiers_) { dest.add_identifier(id); }
}

auto Base::Imp::serialize_tasks(proto::RPCResponse& dest) const noexcept -> void
{
    for (const auto& [index, id] : tasks_) {
        auto& task = *dest.add_task();
        task.set_version(task_version(type_, version_));
        task.set_index(index);
        task.set_id(id);
    }
}

auto Base::Imp::status_version(
    const CommandType& type,
    const VersionNumber parentVersion) noexcept -> VersionNumber
{
    return 2u;
}

auto Base::Imp::task_version(
    const CommandType& type,
    const VersionNumber parentVersion) noexcept -> VersionNumber
{
    return 1u;
}

auto Base::asGetAccountActivity() const noexcept -> const GetAccountActivity&
{
    return imp_->asGetAccountActivity();
}

auto Base::asGetAccountBalance() const noexcept -> const GetAccountBalance&
{
    return imp_->asGetAccountBalance();
}

auto Base::asListAccounts() const noexcept -> const ListAccounts&
{
    return imp_->asListAccounts();
}

auto Base::asListNyms() const noexcept -> const ListNyms&
{
    return imp_->asListNyms();
}

auto Base::asSendPayment() const noexcept -> const SendPayment&
{
    return imp_->asSendPayment();
}

auto Base::Cookie() const noexcept -> const UnallocatedCString&
{
    return imp_->cookie_;
}

auto Base::ResponseCodes() const noexcept -> const Responses&
{
    return imp_->responses_;
}

auto Base::Serialize(AllocateOutput dest) const noexcept -> bool
{
    return imp_->serialize(dest);
}

auto Base::Serialize(proto::RPCResponse& dest) const noexcept -> bool
{
    return imp_->serialize(dest);
}

auto Base::Session() const noexcept -> SessionIndex { return imp_->session_; }

auto Base::Type() const noexcept -> CommandType { return imp_->type_; }

auto Base::Version() const noexcept -> VersionNumber { return imp_->version_; }

Base::~Base() = default;
}  // namespace opentxs::rpc::response
