// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/interface/rpc/CommandType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/ResponseCode.hpp"

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace response
{
class Base;
class GetAccountActivity;
class GetAccountBalance;
class ListAccounts;
class ListNyms;
class SendPayment;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
auto OPENTXS_EXPORT Factory(ReadView serialized) noexcept
    -> std::unique_ptr<Base>;

class OPENTXS_EXPORT Base
{
public:
    using SessionIndex = int;
    using Identifiers = UnallocatedVector<UnallocatedCString>;
    using RequestIndex = std::uint32_t;
    using Status = std::pair<RequestIndex, ResponseCode>;
    using Responses = UnallocatedVector<Status>;
    using TaskID = UnallocatedCString;
    using Queued = std::pair<RequestIndex, TaskID>;
    using Tasks = UnallocatedVector<Queued>;

    struct Imp;

    auto asGetAccountActivity() const noexcept -> const GetAccountActivity&;
    auto asGetAccountBalance() const noexcept -> const GetAccountBalance&;
    auto asListAccounts() const noexcept -> const ListAccounts&;
    auto asListNyms() const noexcept -> const ListNyms&;
    auto asSendPayment() const noexcept -> const SendPayment&;
    auto Cookie() const noexcept -> const UnallocatedCString&;
    auto ResponseCodes() const noexcept -> const Responses&;
    auto Serialize(AllocateOutput dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Serialize(proto::RPCResponse& dest) const noexcept
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
}  // namespace opentxs::rpc::response
