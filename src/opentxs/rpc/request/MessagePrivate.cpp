// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/request/MessagePrivate.hpp"  // IWYU pragma: associated

#include <RPCCommand.pb.h>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/Types.internal.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/request/ListNyms.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"
#include "util/Random.hpp"

namespace opentxs::rpc::request
{
Message::Imp::Imp(
    const Message* parent,
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

Message::Imp::Imp(const Message* parent, const proto::RPCCommand& in) noexcept
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

Message::Imp::Imp(const Message* parent) noexcept
    : Imp(parent, 0, {}, CommandType::error, -1, {}, {}, {}, {}, {})
{
}

Message::Imp::Imp(
    const Message* parent,
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

Message::Imp::Imp(
    const Message* parent,
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

Message::Imp::Imp(
    const Message* parent,
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
    -> const GetAccountActivity&
{
    static const auto blank = GetAccountActivity{};

    return blank;
}

auto Message::Imp::asGetAccountBalance() const noexcept
    -> const GetAccountBalance&
{
    static const auto blank = GetAccountBalance{};

    return blank;
}

auto Message::Imp::asListAccounts() const noexcept -> const ListAccounts&
{
    static const auto blank = ListAccounts{};

    return blank;
}

auto Message::Imp::asListNyms() const noexcept -> const ListNyms&
{
    static const auto blank = ListNyms{};

    return blank;
}

auto Message::Imp::asSendPayment() const noexcept -> const SendPayment&
{
    static const auto blank = SendPayment{};

    return blank;
}

auto Message::Imp::check_dups(
    const Identifiers& data,
    const char* type) noexcept(false) -> void
{
    auto copy{data};
    dedup(copy);

    if (copy.size() != data.size()) {
        throw std::runtime_error{UnallocatedCString{"Duplicate "} + type};
    }
}

auto Message::Imp::check_identifiers() const noexcept(false) -> void
{
    for (const auto& id : identifiers_) {
        if (id.empty()) { throw std::runtime_error{"Empty identifier"}; }
    }

    if (0u == identifiers_.size()) {
        throw std::runtime_error{"Missing identifier"};
    }
}

auto Message::Imp::check_session() const noexcept(false) -> void
{
    if (0 > session_) { throw std::runtime_error{"Invalid session"}; }
}

auto Message::Imp::make_cookie() noexcept -> UnallocatedCString
{
    auto out = UnallocatedCString{};
    random_bytes_non_crypto(writer(out), 20u);

    return out;
}

auto Message::Imp::serialize(proto::RPCCommand& dest) const noexcept -> bool
{
    dest.set_version(version_);
    dest.set_cookie(cookie_);
    dest.set_type(translate(type_));
    dest.set_session(session_);

    for (const auto& nym : associate_nym_) { dest.add_associatenym(nym); }

    return true;
}

auto Message::Imp::serialize(Writer&& dest) const noexcept -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::RPCCommand{};

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

auto Message::Imp::serialize_identifiers(proto::RPCCommand& dest) const noexcept
    -> void
{
    for (const auto& id : identifiers_) { dest.add_identifier(id); }
}

auto Message::Imp::serialize_notary(proto::RPCCommand& dest) const noexcept
    -> void
{
    if (false == notary_.empty()) { dest.set_notary(notary_); }
}

auto Message::Imp::serialize_owner(proto::RPCCommand& dest) const noexcept
    -> void
{
    if (false == owner_.empty()) { dest.set_owner(owner_); }
}

auto Message::Imp::serialize_unit(proto::RPCCommand& dest) const noexcept
    -> void
{
    if (false == unit_.empty()) { dest.set_unit(unit_); }
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

auto Message::AssociatedNyms() const noexcept -> const AssociateNyms&
{
    return imp_->associate_nym_;
}

auto Message::Cookie() const noexcept -> const UnallocatedCString&
{
    return imp_->cookie_;
}

auto Message::Serialize(Writer&& dest) const noexcept -> bool
{
    return imp_->serialize(std::move(dest));
}

auto Message::Serialize(proto::RPCCommand& dest) const noexcept -> bool
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
}  // namespace opentxs::rpc::request
