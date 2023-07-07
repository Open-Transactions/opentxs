// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ProtocolVersion
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <ContractEnums.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdint>
#include <string_view>
#include <utility>

#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitType.hpp"          // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Account;
class AccountVisitor;
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::blank
{
template <typename IDType>
struct Signable : virtual public opentxs::contract::Signable<IDType> {
    auto Alias() const noexcept -> UnallocatedCString final { return {}; }
    auto Alias(alloc::Strategy alloc) const noexcept -> CString final
    {
        return CString{alloc.result_};
    }
    auto ID() const noexcept -> const IDType& final { return id_; }
    auto Name() const noexcept -> std::string_view final { return {}; }
    auto Signer() const noexcept -> Nym_p final { return {}; }
    auto Terms() const noexcept -> std::string_view final { return terms_; }
    auto Serialize(Writer&&) const noexcept -> bool final { return {}; }
    auto Validate() const noexcept -> bool final { return {}; }
    auto Version() const noexcept -> VersionNumber final { return 0; }

    auto SetAlias(std::string_view) noexcept -> bool final { return {}; }

    Signable(const api::Session& api)
        : api_(api)
        , id_()
        , terms_()
    {
    }

    ~Signable() override = default;

protected:
    const api::Session& api_;
    const IDType id_;
    const UnallocatedCString terms_;

    Signable(const Signable& rhs)
        : api_(rhs.api_)
        , id_(rhs.id_)
        , terms_(rhs.terms_)
    {
    }
};

struct Unit final : virtual public opentxs::contract::Unit,
                    public blank::Signable<identifier::UnitDefinition> {
    auto AddAccountRecord(const UnallocatedCString&, const Account&) const
        -> bool final
    {
        return {};
    }
    auto DisplayStatistics(String&) const -> bool final { return {}; }
    auto EraseAccountRecord(
        const UnallocatedCString&,
        const identifier::Account&) const -> bool final
    {
        return {};
    }
    using Signable::Serialize;
    auto Serialize(Writer&& destination, bool includeNym = false) const
        -> bool final
    {
        return write(proto::UnitDefinition{}, std::move(destination));
    }
    auto Serialize(proto::UnitDefinition& output, bool includeNym = false) const
        -> bool final;
    auto Type() const -> contract::UnitType final { return {}; }
    auto UnitOfAccount() const -> opentxs::UnitType final { return {}; }
    auto VisitAccountRecords(
        const UnallocatedCString&,
        AccountVisitor&,
        const PasswordPrompt&) const -> bool final
    {
        return {};
    }

    void InitAlias(std::string_view) final {}

    Unit(const api::Session& api)
        : Signable(api)
    {
    }

    ~Unit() override = default;

private:
    Unit(const Unit& rhs)
        : Signable(rhs)
    {
    }
};

struct Server final : virtual public opentxs::contract::Server,
                      public blank::Signable<identifier::Notary> {
    auto ConnectInfo(
        UnallocatedCString&,
        std::uint32_t&,
        AddressType&,
        const AddressType&) const -> bool final
    {
        return {};
    }
    auto EffectiveName() const -> UnallocatedCString final { return {}; }
    using Signable::Serialize;
    auto Serialize(Writer&& destination, bool includeNym = false) const
        -> bool final
    {
        return write(proto::ServerContract{}, std::move(destination));
    }
    auto Serialize(proto::ServerContract& output, bool includeNym = false) const
        -> bool final;
    auto Statistics(String&) const -> bool final { return {}; }
    auto TransportKey() const -> const Data& final { return id_; }
    auto TransportKey(Data&, const PasswordPrompt&) const -> Secret final
    {
        return api_.Factory().Secret(0);
    }

    void InitAlias(std::string_view) final {}

    Server(const api::Session& api)
        : Signable(api)
    {
    }

    ~Server() final = default;

private:
    Server(const Server& rhs)
        : Signable(rhs)
    {
    }
};
}  // namespace opentxs::contract::blank

namespace opentxs
{
auto translate(const contract::ProtocolVersion in) noexcept
    -> proto::ProtocolVersion;
auto translate(const contract::UnitType in) noexcept -> proto::UnitType;
auto translate(const proto::ProtocolVersion in) noexcept
    -> contract::ProtocolVersion;
auto translate(const proto::UnitType in) noexcept -> contract::UnitType;
}  // namespace opentxs
