// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/rpc/request/Message.hpp"  // IWYU pragma: associated

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class RPCCommand;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class GetAccountActivity;
class GetAccountBalance;
class ListAccounts;
class ListNyms;
class SendPayment;
}  // namespace request
}  // namespace rpc

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::request
{
struct Message::Imp {
    const Message* parent_;
    const VersionNumber version_;
    const UnallocatedCString cookie_;
    const CommandType type_;
    const SessionIndex session_;
    const AssociateNyms associate_nym_;
    const UnallocatedCString owner_;
    const UnallocatedCString notary_;
    const UnallocatedCString unit_;
    const Identifiers identifiers_;

    static auto check_dups(const Identifiers& data, const char* type) noexcept(
        false) -> void;

    virtual auto asGetAccountActivity() const noexcept
        -> const GetAccountActivity&;
    virtual auto asGetAccountBalance() const noexcept
        -> const GetAccountBalance&;
    virtual auto asListAccounts() const noexcept -> const ListAccounts&;
    virtual auto asListNyms() const noexcept -> const ListNyms&;
    virtual auto asSendPayment() const noexcept -> const SendPayment&;
    auto check_identifiers() const noexcept(false) -> void;
    auto check_session() const noexcept(false) -> void;
    virtual auto serialize(protobuf::RPCCommand& dest) const noexcept -> bool;
    auto serialize(Writer&& dest) const noexcept -> bool;
    auto serialize_identifiers(protobuf::RPCCommand& dest) const noexcept
        -> void;
    auto serialize_notary(protobuf::RPCCommand& dest) const noexcept -> void;
    auto serialize_owner(protobuf::RPCCommand& dest) const noexcept -> void;
    auto serialize_unit(protobuf::RPCCommand& dest) const noexcept -> void;

    Imp(const Message* parent) noexcept;
    Imp(const Message* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const AssociateNyms& nyms) noexcept;
    Imp(const Message* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const Identifiers& identifiers,
        const AssociateNyms& nyms) noexcept;
    Imp(const Message* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const UnallocatedCString& owner,
        const UnallocatedCString& notary,
        const UnallocatedCString& unit,
        const AssociateNyms& nyms) noexcept;
    Imp(const Message* parent, const protobuf::RPCCommand& serialized) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    virtual ~Imp() = default;

private:
    static auto make_cookie() noexcept -> UnallocatedCString;

    Imp(const Message* parent,
        VersionNumber version,
        const UnallocatedCString& cookie,
        const CommandType& type,
        SessionIndex session,
        const AssociateNyms& nyms,
        const UnallocatedCString& owner,
        const UnallocatedCString& notary,
        const UnallocatedCString& unit,
        const Identifiers identifiers) noexcept;
};
}  // namespace opentxs::rpc::request
