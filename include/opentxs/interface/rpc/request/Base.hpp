// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
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
auto Factory(ReadView serialized) noexcept -> std::unique_ptr<Base>;
auto Factory(const proto::RPCCommand& serialized) noexcept
    -> std::unique_ptr<Base>;

class OPENTXS_EXPORT Base
{
public:
    using SessionIndex = int;
    using Identifiers = UnallocatedVector<UnallocatedCString>;
    using AssociateNyms = Identifiers;

    struct Imp;

    auto asGetAccountActivity() const noexcept -> const GetAccountActivity&;
    auto asGetAccountBalance() const noexcept -> const GetAccountBalance&;
    auto asListAccounts() const noexcept -> const ListAccounts&;
    auto asListNyms() const noexcept -> const ListNyms&;
    auto asSendPayment() const noexcept -> const SendPayment&;

    auto AssociatedNyms() const noexcept -> const AssociateNyms&;
    auto Cookie() const noexcept -> const UnallocatedCString&;
    auto Serialize(Writer&& dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Serialize(proto::RPCCommand& dest) const noexcept
        -> bool;
    auto Session() const noexcept -> SessionIndex;
    auto Type() const noexcept -> CommandType;
    auto Version() const noexcept -> VersionNumber;

    Base() noexcept;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    virtual ~Base();

protected:
    std::unique_ptr<Imp> imp_;

    Base(std::unique_ptr<Imp> imp) noexcept;
};
}  // namespace opentxs::rpc::request
