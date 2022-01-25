// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/network/p2p/PublishContract.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Proto.tpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Identifier.pb.h"

namespace opentxs::factory
{
auto BlockchainSyncPublishContract() noexcept -> network::p2p::PublishContract
{
    using ReturnType = network::p2p::PublishContract;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncPublishContract(const identity::Nym& payload) noexcept
    -> network::p2p::PublishContract
{
    using ReturnType = network::p2p::PublishContract;

    try {
        return std::make_unique<ReturnType::Imp>(
                   contract::Type::nym,
                   payload.ID(),
                   [&] {
                       auto out = Space{};

                       if (false == payload.Serialize(writer(out))) {
                           throw std::runtime_error{"failed to serialize nym"};
                       }

                       return out;
                   }())
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return BlockchainSyncPublishContract();
    }
}

auto BlockchainSyncPublishContract(const contract::Server& payload) noexcept
    -> network::p2p::PublishContract
{
    using ReturnType = network::p2p::PublishContract;

    try {
        return std::make_unique<ReturnType::Imp>(
                   contract::Type::notary,
                   payload.ID(),
                   [&] {
                       auto out = Space{};

                       if (false == payload.Serialize(writer(out))) {
                           throw std::runtime_error{
                               "failed to serialize notary"};
                       }

                       return out;
                   }())
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return BlockchainSyncPublishContract();
    }
}

auto BlockchainSyncPublishContract(const contract::Unit& payload) noexcept
    -> network::p2p::PublishContract
{
    using ReturnType = network::p2p::PublishContract;

    try {
        return std::make_unique<ReturnType::Imp>(
                   contract::Type::unit,
                   payload.ID(),
                   [&] {
                       auto out = Space{};

                       if (false == payload.Serialize(writer(out))) {
                           throw std::runtime_error{"failed to serialize unit"};
                       }

                       return out;
                   }())
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return BlockchainSyncPublishContract();
    }
}

auto BlockchainSyncPublishContract_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::p2p::PublishContract>
{
    using ReturnType = network::p2p::PublishContract;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, type, id, payload).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::p2p
{
class PublishContract::Imp final : public Base::Imp
{
public:
    const contract::Type contract_type_;
    const OTIdentifier contract_id_;
    const Space payload_;
    PublishContract* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asPublishContract() const noexcept -> const PublishContract& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asPublishContract();
        }
    }
    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        using Buffer = boost::endian::little_uint32_buf_t;

        static_assert(sizeof(Buffer) == sizeof(contract_type_));

        out.AddFrame(Buffer{static_cast<std::uint32_t>(contract_type_)});
        out.Internal().AddFrame([&] {
            auto out = proto::Identifier{};
            contract_id_->Serialize(out);

            return out;
        }());
        out.AddFrame(payload_);

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , contract_type_(contract::Type::invalid)
        , contract_id_(Identifier::Factory())
        , payload_()
        , parent_(nullptr)
    {
    }
    Imp(const contract::Type type, OTIdentifier&& id, Space&& payload) noexcept
        : Base::Imp(MessageType::publish_contract)
        , contract_type_(type)
        , contract_id_(std::move(id))
        , payload_(std::move(payload))
        , parent_(nullptr)
    {
    }
    Imp(const contract::Type type,
        const Identifier& id,
        Space&& payload) noexcept
        : Imp(type, OTIdentifier{id}, std::move(payload))
    {
    }
    Imp(const api::Session& api,
        const contract::Type type,
        const ReadView id,
        const ReadView payload) noexcept
        : Imp(type,
              api.Factory().InternalSession().Identifier(
                  proto::Factory<proto::Identifier>(id.data(), id.size())),
              space(payload))
    {
    }

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

PublishContract::PublishContract(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto PublishContract::ID() const noexcept -> const Identifier&
{
    return Imp::get(imp_).contract_id_;
}

auto PublishContract::Payload() const noexcept -> ReadView
{
    return reader(Imp::get(imp_).payload_);
}

auto PublishContract::ContractType() const noexcept -> contract::Type
{
    return Imp::get(imp_).contract_type_;
}

PublishContract::~PublishContract()
{
    if (nullptr != PublishContract::imp_) {
        delete PublishContract::imp_;
        PublishContract::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::p2p
