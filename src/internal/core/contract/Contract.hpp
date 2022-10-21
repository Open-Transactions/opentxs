// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/AddressType.hpp"
// IWYU pragma: no_include "opentxs/core/contract/ProtocolVersion.hpp"
// IWYU pragma: no_include "opentxs/core/contract/UnitType.hpp"
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <ContractEnums.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdint>
#include <utility>

#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/BailmentNotice.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/core/contract/peer/ConnectionReply.hpp"
#include "internal/core/contract/peer/ConnectionRequest.hpp"
#include "internal/core/contract/peer/NoticeAcknowledgement.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
#include "internal/core/contract/peer/OutBailmentRequest.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
namespace blank
{
struct Acknowledgement;
struct Bailment;
struct Connection;
struct Outbailment;
}  // namespace blank
}  // namespace reply

namespace request
{
namespace blank
{
struct Bailment;
struct BailmentNotice;
struct Connection;
struct Outbailment;
struct StoreSecret;
}  // namespace blank
}  // namespace request
}  // namespace peer
}  // namespace contract

class Account;
class AccountVisitor;
class Data;
class PasswordPrompt;
class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::blank
{
struct Signable : virtual public opentxs::contract::Signable {
    auto Alias() const noexcept -> UnallocatedCString final { return {}; }
    auto ID() const noexcept -> identifier::Generic final { return id_; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Nym() const noexcept -> Nym_p final { return {}; }
    auto Terms() const noexcept -> const UnallocatedCString& final
    {
        return terms_;
    }
    auto Serialize() const noexcept -> ByteArray final
    {
        return ByteArray{id_};
    }
    auto Validate() const noexcept -> bool final { return {}; }
    auto Version() const noexcept -> VersionNumber final { return 0; }

    auto SetAlias(const UnallocatedCString&) noexcept -> bool final
    {
        return {};
    }

    Signable(const api::Session& api)
        : api_(api)
        , id_()
        , terms_()
    {
    }

    ~Signable() override = default;

protected:
    const api::Session& api_;
    const identifier::Generic id_;
    const UnallocatedCString terms_;

    Signable(const Signable& rhs)
        : api_(rhs.api_)
        , id_(rhs.id_)
        , terms_(rhs.terms_)
    {
    }
};

struct Unit final : virtual public opentxs::contract::Unit, public Signable {
    auto AddAccountRecord(const UnallocatedCString&, const Account&) const
        -> bool final
    {
        return {};
    }
    auto DisplayStatistics(String&) const -> bool final { return {}; }
    auto EraseAccountRecord(
        const UnallocatedCString&,
        const identifier::Generic&) const -> bool final
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

    void InitAlias(const UnallocatedCString&) final {}

    Unit(const api::Session& api)
        : Signable(api)
    {
    }

    ~Unit() override = default;

private:
    auto clone() const noexcept -> Unit* override { return new Unit(*this); }

    Unit(const Unit& rhs)
        : Signable(rhs)
    {
    }
};

struct Server final : virtual public opentxs::contract::Server,
                      public blank::Signable {
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

    void InitAlias(const UnallocatedCString&) final {}

    Server(const api::Session& api)
        : Signable(api)
    {
    }

    ~Server() final = default;

private:
    auto clone() const noexcept -> Server* final { return new Server(*this); }

    Server(const Server& rhs)
        : Signable(rhs)
    {
    }
};
}  // namespace opentxs::contract::blank

namespace opentxs::contract::peer::blank
{
struct Reply : virtual public opentxs::contract::peer::Reply,
               public contract::blank::Signable {
    auto asAcknowledgement() const noexcept
        -> const reply::Acknowledgement& final;
    auto asBailment() const noexcept -> const reply::Bailment& final;
    auto asConnection() const noexcept -> const reply::Connection& final;
    auto asOutbailment() const noexcept -> const reply::Outbailment& final;

    using Signable::Serialize;
    auto Serialize(SerializedType& output) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> PeerRequestType final
    {
        return PeerRequestType::Error;
    }

    Reply(const api::Session& api)
        : Signable(api)
        , server_()
    {
    }

    ~Reply() override = default;

protected:
    const identifier::Notary server_;

    auto clone() const noexcept -> Reply* override { return new Reply(*this); }

    Reply(const Reply& rhs)
        : Signable(rhs)
        , server_(rhs.server_)
    {
    }
};

struct Request : virtual public opentxs::contract::peer::Request,
                 public contract::blank::Signable {
    auto asBailment() const noexcept -> const peer::request::Bailment& final;
    auto asBailmentNotice() const noexcept
        -> const peer::request::BailmentNotice& final;
    auto asConnection() const noexcept
        -> const peer::request::Connection& final;
    auto asOutbailment() const noexcept
        -> const peer::request::Outbailment& final;
    auto asStoreSecret() const noexcept
        -> const peer::request::StoreSecret& final;

    auto Initiator() const -> const identifier::Nym& final { return nym_; }
    auto Recipient() const -> const identifier::Nym& final { return nym_; }
    using Signable::Serialize;
    auto Serialize(SerializedType& output) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Type() const -> PeerRequestType final
    {
        return PeerRequestType::Error;
    }

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

    auto clone() const noexcept -> Request* override
    {
        return new Request(*this);
    }

    Request(const Request& rhs)
        : Signable(rhs)
        , nym_(rhs.nym_)
        , server_(rhs.server_)
    {
    }
};
}  // namespace opentxs::contract::peer::blank

namespace opentxs::contract::peer::reply::blank
{
struct Acknowledgement final
    : virtual public opentxs::contract::peer::reply::Acknowledgement,
      public contract::peer::blank::Reply {

    Acknowledgement(const api::Session& api)
        : Reply(api)
    {
    }

    ~Acknowledgement() final = default;

private:
    auto clone() const noexcept -> Acknowledgement* final
    {
        return new Acknowledgement(*this);
    }

    Acknowledgement(const Acknowledgement& rhs)
        : Reply(rhs)
    {
    }
};

struct Bailment final : virtual public opentxs::contract::peer::reply::Bailment,
                        public contract::peer::blank::Reply {

    Bailment(const api::Session& api)
        : Reply(api)
    {
    }

    ~Bailment() final = default;

private:
    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }

    Bailment(const Bailment& rhs)
        : Reply(rhs)
    {
    }
};

struct Connection final
    : virtual public opentxs::contract::peer::reply::Connection,
      public contract::peer::blank::Reply {

    Connection(const api::Session& api)
        : Reply(api)
    {
    }

    ~Connection() final = default;

private:
    auto clone() const noexcept -> Connection* final
    {
        return new Connection(*this);
    }

    Connection(const Connection& rhs)
        : Reply(rhs)
    {
    }
};

struct Outbailment final
    : virtual public opentxs::contract::peer::reply::Outbailment,
      public contract::peer::blank::Reply {

    Outbailment(const api::Session& api)
        : Reply(api)
    {
    }

    ~Outbailment() final = default;

private:
    auto clone() const noexcept -> Outbailment* final
    {
        return new Outbailment(*this);
    }

    Outbailment(const Outbailment& rhs)
        : Reply(rhs)
    {
    }
};

}  // namespace opentxs::contract::peer::reply::blank

namespace opentxs::contract::peer::request::blank
{
struct Bailment final
    : virtual public opentxs::contract::peer::request::Bailment,
      public contract::peer::blank::Request {
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

    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }

    Bailment(const Bailment& rhs)
        : Request(rhs)
        , unit_(rhs.unit_)
    {
    }
};

struct BailmentNotice final
    : virtual public opentxs::contract::peer::request::BailmentNotice,
      public contract::peer::blank::Request {

    BailmentNotice(const api::Session& api)
        : Request(api)
    {
    }

    ~BailmentNotice() final = default;

private:
    auto clone() const noexcept -> BailmentNotice* final
    {
        return new BailmentNotice(*this);
    }

    BailmentNotice(const BailmentNotice& rhs)
        : Request(rhs)
    {
    }
};

struct Connection final
    : virtual public opentxs::contract::peer::request::Connection,
      public contract::peer::blank::Request {

    Connection(const api::Session& api)
        : Request(api)
    {
    }

    ~Connection() final = default;

private:
    auto clone() const noexcept -> Connection* final
    {
        return new Connection(*this);
    }

    Connection(const Connection& rhs)
        : Request(rhs)
    {
    }
};

struct Outbailment final
    : virtual public opentxs::contract::peer::request::Outbailment,
      public contract::peer::blank::Request {

    Outbailment(const api::Session& api)
        : Request(api)
    {
    }

    ~Outbailment() final = default;

private:
    auto clone() const noexcept -> Outbailment* final
    {
        return new Outbailment(*this);
    }

    Outbailment(const Outbailment& rhs)
        : Request(rhs)
    {
    }
};

struct StoreSecret final
    : virtual public opentxs::contract::peer::request::StoreSecret,
      public contract::peer::blank::Request {

    StoreSecret(const api::Session& api)
        : Request(api)
    {
    }

    ~StoreSecret() final = default;

private:
    auto clone() const noexcept -> StoreSecret* final
    {
        return new StoreSecret(*this);
    }

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
