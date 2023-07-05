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
#include "internal/core/contract/peer/reply/Acknowledgement.hpp"
#include "internal/core/contract/peer/reply/Bailment.hpp"
#include "internal/core/contract/peer/reply/Base.hpp"
#include "internal/core/contract/peer/reply/Connection.hpp"
#include "internal/core/contract/peer/reply/Faucet.hpp"
#include "internal/core/contract/peer/reply/Outbailment.hpp"
#include "internal/core/contract/peer/request/Bailment.hpp"
#include "internal/core/contract/peer/request/BailmentNotice.hpp"
#include "internal/core/contract/peer/request/Base.hpp"
#include "internal/core/contract/peer/request/Connection.hpp"
#include "internal/core/contract/peer/request/Faucet.hpp"
#include "internal/core/contract/peer/request/Outbailment.hpp"
#include "internal/core/contract/peer/request/StoreSecret.hpp"
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
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
    auto Nym() const noexcept -> Nym_p final { return {}; }
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

namespace opentxs::contract::peer::reply::blank
{
struct Reply : virtual public reply::internal::Reply,
               public contract::blank::Signable<identifier::Generic> {
    auto asAcknowledgement() const noexcept
        -> const internal::Acknowledgement& final;
    auto asBailment() const noexcept -> const internal::Bailment& final;
    auto asConnection() const noexcept -> const internal::Connection& final;
    auto asFaucet() const noexcept -> const internal::Faucet& final;
    auto asOutbailment() const noexcept -> const internal::Outbailment& final;

    auto Initiator() const -> const identifier::Nym& final { return nym_; }
    auto Recipient() const -> const identifier::Nym& final { return nym_; }
    using Signable::Serialize;
    auto Serialize(SerializedType& output) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> RequestType final { return RequestType::Error; }

    Reply(const api::Session& api)
        : Signable(api)
        , nym_()
        , server_()
    {
    }

    ~Reply() override = default;

protected:
    const identifier::Nym nym_;
    const identifier::Notary server_;

    Reply(const Reply& rhs)
        : Signable(rhs)
        , nym_(rhs.nym_)
        , server_(rhs.server_)
    {
    }
};

struct Acknowledgement final : virtual public reply::internal::Acknowledgement,
                               public blank::Reply {

    Acknowledgement(const api::Session& api)
        : Reply(api)
    {
    }

    ~Acknowledgement() final = default;

private:
    Acknowledgement(const Acknowledgement& rhs)
        : Reply(rhs)
    {
    }
};

struct Bailment final : virtual public reply::internal::Bailment,
                        public blank::Reply {

    Bailment(const api::Session& api)
        : Reply(api)
    {
    }

    ~Bailment() final = default;

private:
    Bailment(const Bailment& rhs)
        : Reply(rhs)
    {
    }
};

struct Connection final : virtual public reply::internal::Connection,
                          public blank::Reply {

    Connection(const api::Session& api)
        : Reply(api)
    {
    }

    ~Connection() final = default;

private:
    Connection(const Connection& rhs)
        : Reply(rhs)
    {
    }
};

struct Faucet final : virtual public reply::internal::Faucet,
                      public blank::Reply {

    Faucet(const api::Session& api)
        : Reply(api)
    {
    }

    ~Faucet() final = default;

private:
    Faucet(const Faucet& rhs)
        : Reply(rhs)
    {
    }
};

struct Outbailment final : virtual public reply::internal::Outbailment,
                           public blank::Reply {

    Outbailment(const api::Session& api)
        : Reply(api)
    {
    }

    ~Outbailment() final = default;

private:
    Outbailment(const Outbailment& rhs)
        : Reply(rhs)
    {
    }
};

}  // namespace opentxs::contract::peer::reply::blank

namespace opentxs::contract::peer::request::blank
{
struct Request
    : virtual public opentxs::contract::peer::request::internal::Request,
      public contract::blank::Signable<identifier::Generic> {
    auto asBailment() const noexcept -> const internal::Bailment& final;
    auto asBailmentNotice() const noexcept
        -> const internal::BailmentNotice& final;
    auto asConnection() const noexcept -> const internal::Connection& final;
    auto asFaucet() const noexcept -> const internal::Faucet& final;
    auto asOutbailment() const noexcept -> const internal::Outbailment& final;
    auto asStoreSecret() const noexcept -> const internal::StoreSecret& final;

    auto Initiator() const -> const identifier::Nym& final { return nym_; }
    auto Recipient() const -> const identifier::Nym& final { return nym_; }
    using Signable::Serialize;
    auto Serialize(SerializedType& output) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> RequestType final { return RequestType::Error; }

    Request(const api::Session& api)
        : Signable(api)
        , nym_()
        , server_()
    {
    }

    ~Request() override = default;

protected:
    const identifier::Nym nym_;
    const identifier::Notary server_;

    Request(const Request& rhs)
        : Signable(rhs)
        , nym_(rhs.nym_)
        , server_(rhs.server_)
    {
    }
};

struct Bailment final
    : virtual public opentxs::contract::peer::request::internal::Bailment,
      public blank::Request {
    auto UnitID() const -> const identifier::UnitDefinition& final
    {
        return unit_;
    }
    auto ServerID() const -> const identifier::Notary& final { return server_; }

    Bailment(const api::Session& api)
        : Request(api)
        , unit_()
    {
    }

    ~Bailment() final = default;

private:
    const identifier::UnitDefinition unit_;

    Bailment(const Bailment& rhs)
        : Request(rhs)
        , unit_(rhs.unit_)
    {
    }
};

struct BailmentNotice final
    : virtual public opentxs::contract::peer::request::internal::BailmentNotice,
      public blank::Request {

    BailmentNotice(const api::Session& api)
        : Request(api)
    {
    }

    ~BailmentNotice() final = default;

private:
    BailmentNotice(const BailmentNotice& rhs)
        : Request(rhs)
    {
    }
};

struct Connection final
    : virtual public opentxs::contract::peer::request::internal::Connection,
      public blank::Request {

    Connection(const api::Session& api)
        : Request(api)
    {
    }

    ~Connection() final = default;

private:
    Connection(const Connection& rhs)
        : Request(rhs)
    {
    }
};

struct Faucet final
    : virtual public opentxs::contract::peer::request::internal::Faucet,
      public blank::Request {
    auto Address() const -> std::string_view final { return {}; }
    auto Currency() const -> opentxs::UnitType final { return {}; }

    Faucet(const api::Session& api)
        : Request(api)
    {
    }

    ~Faucet() final = default;

private:
    Faucet(const Faucet& rhs)
        : Request(rhs)
    {
    }
};

struct Outbailment final
    : virtual public opentxs::contract::peer::request::internal::Outbailment,
      public blank::Request {

    Outbailment(const api::Session& api)
        : Request(api)
    {
    }

    ~Outbailment() final = default;

private:
    Outbailment(const Outbailment& rhs)
        : Request(rhs)
    {
    }
};

struct StoreSecret final
    : virtual public opentxs::contract::peer::request::internal::StoreSecret,
      public blank::Request {

    StoreSecret(const api::Session& api)
        : Request(api)
    {
    }

    ~StoreSecret() final = default;

private:
    StoreSecret(const StoreSecret& rhs)
        : Request(rhs)
    {
    }
};
}  // namespace opentxs::contract::peer::request::blank

namespace opentxs
{
auto translate(const contract::ProtocolVersion in) noexcept
    -> proto::ProtocolVersion;
auto translate(const contract::UnitType in) noexcept -> proto::UnitType;
auto translate(const proto::ProtocolVersion in) noexcept
    -> contract::ProtocolVersion;
auto translate(const proto::UnitType in) noexcept -> contract::UnitType;
}  // namespace opentxs
