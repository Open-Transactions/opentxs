// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
class Unit;
}  // namespace contract

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Account;
class AccountVisitor;
class Amount;
class PasswordPrompt;
class Writer;

using OTUnitDefinition = SharedPimpl<contract::Unit>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract
{
class Unit : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::UnitDefinition;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    virtual auto AddAccountRecord(
        const UnallocatedCString& dataFolder,
        const Account& theAccount) const -> bool = 0;
    virtual auto DisplayStatistics(String& strContents) const -> bool = 0;
    virtual auto EraseAccountRecord(
        const UnallocatedCString& dataFolder,
        const identifier::Generic& theAcctID) const -> bool = 0;
    using Signable::Serialize;
    virtual auto Serialize(SerializedType&, bool includeNym = false) const
        -> bool = 0;
    virtual auto Serialize(Writer&& destination, bool includeNym = false) const
        -> bool = 0;
    virtual auto Type() const -> contract::UnitType = 0;
    virtual auto UnitOfAccount() const -> opentxs::UnitType = 0;
    virtual auto VisitAccountRecords(
        const UnallocatedCString& dataFolder,
        AccountVisitor& visitor,
        const PasswordPrompt& reason) const -> bool = 0;

    virtual void InitAlias(const UnallocatedCString& alias) = 0;

    Unit(const Unit&) = delete;
    Unit(Unit&&) = delete;
    auto operator=(const Unit&) -> Unit& = delete;
    auto operator=(Unit&&) -> Unit& = delete;

    ~Unit() override = default;

protected:
    Unit() noexcept = default;

private:
    friend OTUnitDefinition;

#ifndef _WIN32
    auto clone() const noexcept -> Unit* override = 0;
#endif
};
}  // namespace opentxs::contract