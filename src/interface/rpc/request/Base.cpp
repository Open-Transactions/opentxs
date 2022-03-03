// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "opentxs/interface/rpc/request/Base.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "interface/rpc/request/Base.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "opentxs/interface/rpc/CommandType.hpp"
#include "opentxs/interface/rpc/request/GetAccountActivity.hpp"
#include "opentxs/interface/rpc/request/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/request/ListAccounts.hpp"
#include "opentxs/interface/rpc/request/ListNyms.hpp"
#include "opentxs/interface/rpc/request/SendPayment.hpp"
#include "serialization/protobuf/RPCCommand.pb.h"
#include "util/Container.hpp"
#include "util/Random.hpp"

namespace opentxs::rpc::request
{
Base::Imp::Imp(
    const Base* parent,
    VersionNumber version,
    const UnallocatedCString& cookie,
    const CommandType& type,
    SessionIndex session,
    const AssociateNyms& nyms,
    const UnallocatedCString& owner,
    const UnallocatedCString& notary,
    const UnallocatedCString& unit,
    const Identifiers identifiers) noexcept
    : parent_(parent)
    , version_(version)
    , cookie_(cookie)
    , type_(type)
    , session_(session)
    , associate_nym_(nyms)
    , owner_(owner)
    , notary_(notary)
    , unit_(unit)
    , identifiers_(identifiers)
{
    check_dups(identifiers_, "identifier");
    check_dups(associate_nym_, "associated nym");
}

Base::Imp::Imp(const Base* parent, const proto::RPCCommand& in) noexcept
    : Imp(
          parent,
          in.version(),
          in.cookie(),
          translate(in.type()),
          in.session(),
          [&] {
              auto out = AssociateNyms{};

              for (const auto& id : in.associatenym()) { out.emplace_back(id); }

              return out;
          }(),
          in.owner(),
          in.notary(),
          in.unit(),
          [&] {
              auto out = Identifiers{};

              for (const auto& id : in.identifier()) { out.emplace_back(id); }

              return out;
          }())
{
}

Base::Imp::Imp(const Base* parent) noexcept
    : Imp(parent, 0, {}, CommandType::error, -1, {}, {}, {}, {}, {})
{
}

Base::Imp::Imp(
    const Base* parent,
    const CommandType& command,
    VersionNumber version,
    SessionIndex session,
    const AssociateNyms& nyms) noexcept
    : Imp(parent,
          version,
          make_cookie(),
          command,
          session,
          nyms,
          {},
          {},
          {},
          {})
{
}

Base::Imp::Imp(
    const Base* parent,
    const CommandType& command,
    VersionNumber version,
    SessionIndex session,
    const Identifiers& identifiers,
    const AssociateNyms& nyms) noexcept
    : Imp(parent,
          version,
          make_cookie(),
          command,
          session,
          nyms,
          {},
          {},
          {},
          identifiers)
{
}

Base::Imp::Imp(
    const Base* parent,
    const CommandType& command,
    VersionNumber version,
    SessionIndex session,
    const UnallocatedCString& owner,
    const UnallocatedCString& notary,
    const UnallocatedCString& unit,
    const AssociateNyms& nyms) noexcept
    : Imp(parent,
          version,
          make_cookie(),
          command,
          session,
          nyms,
          owner,
          notary,
          unit,
          {})
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
    -> const GetAccountActivity&
{
    static const auto blank = GetAccountActivity{};

    return blank;
}

auto Base::Imp::asGetAccountBalance() const noexcept -> const GetAccountBalance&
{
    static const auto blank = GetAccountBalance{};

    return blank;
}

auto Base::Imp::asListAccounts() const noexcept -> const ListAccounts&
{
    static const auto blank = ListAccounts{};

    return blank;
}

auto Base::Imp::asListNyms() const noexcept -> const ListNyms&
{
    static const auto blank = ListNyms{};

    return blank;
}

auto Base::Imp::asSendPayment() const noexcept -> const SendPayment&
{
    static const auto blank = SendPayment{};

    return blank;
}

auto Base::Imp::check_dups(const Identifiers& data, const char* type) noexcept(
    false) -> void
{
    auto copy{data};
    dedup(copy);

    if (copy.size() != data.size()) {
        throw std::runtime_error{UnallocatedCString{"Duplicate "} + type};
    }
}

auto Base::Imp::check_identifiers() const noexcept(false) -> void
{
    for (const auto& id : identifiers_) {
        if (id.empty()) { throw std::runtime_error{"Empty identifier"}; }
    }

    if (0u == identifiers_.size()) {
        throw std::runtime_error{"Missing identifier"};
    }
}

auto Base::Imp::check_session() const noexcept(false) -> void
{
    if (0 > session_) { throw std::runtime_error{"Invalid session"}; }
}

auto Base::Imp::make_cookie() noexcept -> UnallocatedCString
{
    auto out = UnallocatedCString{};
    random_bytes_non_crypto(writer(out), 20u);

    return out;
}

auto Base::Imp::serialize(proto::RPCCommand& dest) const noexcept -> bool
{
    dest.set_version(version_);
    dest.set_cookie(cookie_);
    dest.set_type(translate(type_));
    dest.set_session(session_);

    for (const auto& nym : associate_nym_) { dest.add_associatenym(nym); }

    return true;
}

auto Base::Imp::serialize(AllocateOutput dest) const noexcept -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::RPCCommand{};

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

auto Base::Imp::serialize_identifiers(proto::RPCCommand& dest) const noexcept
    -> void
{
    for (const auto& id : identifiers_) { dest.add_identifier(id); }
}

auto Base::Imp::serialize_notary(proto::RPCCommand& dest) const noexcept -> void
{
    if (false == notary_.empty()) { dest.set_notary(notary_); }
}

auto Base::Imp::serialize_owner(proto::RPCCommand& dest) const noexcept -> void
{
    if (false == owner_.empty()) { dest.set_owner(owner_); }
}

auto Base::Imp::serialize_unit(proto::RPCCommand& dest) const noexcept -> void
{
    if (false == unit_.empty()) { dest.set_unit(unit_); }
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

auto Base::AssociatedNyms() const noexcept -> const AssociateNyms&
{
    return imp_->associate_nym_;
}

auto Base::Cookie() const noexcept -> const UnallocatedCString&
{
    return imp_->cookie_;
}

auto Base::Serialize(AllocateOutput dest) const noexcept -> bool
{
    return imp_->serialize(dest);
}

auto Base::Serialize(proto::RPCCommand& dest) const noexcept -> bool
{
    return imp_->serialize(dest);
}

auto Base::Session() const noexcept -> SessionIndex { return imp_->session_; }

auto Base::Type() const noexcept -> CommandType { return imp_->type_; }

auto Base::Version() const noexcept -> VersionNumber { return imp_->version_; }

Base::~Base() = default;
}  // namespace opentxs::rpc::request
