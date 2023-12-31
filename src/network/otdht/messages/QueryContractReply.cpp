// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/QueryContractReply.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <opentxs/protobuf/Identifier.pb.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/contract/ContractType.hpp"  // IWYU pragma: keep
#include "opentxs/contract/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Nym.hpp"     // IWYU pragma: keep
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/otdht/Block.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BlockchainSyncQueryContractReply() noexcept
    -> network::otdht::QueryContractReply
{
    using ReturnType = network::otdht::QueryContractReply;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncQueryContractReply(const identifier::Generic& id) noexcept
    -> network::otdht::QueryContractReply
{
    using ReturnType = network::otdht::QueryContractReply;

    return std::make_unique<ReturnType::Imp>(translate(id.Type()), id, Space{})
        .release();
}

auto BlockchainSyncQueryContractReply(const identity::Nym& payload) noexcept
    -> network::otdht::QueryContractReply
{
    using ReturnType = network::otdht::QueryContractReply;

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
        LogError()()(e.what()).Flush();

        return BlockchainSyncQueryContractReply();
    }
}

auto BlockchainSyncQueryContractReply(const contract::Server& payload) noexcept
    -> network::otdht::QueryContractReply
{
    using ReturnType = network::otdht::QueryContractReply;

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
        LogError()()(e.what()).Flush();

        return BlockchainSyncQueryContractReply();
    }
}

auto BlockchainSyncQueryContractReply(const contract::Unit& payload) noexcept
    -> network::otdht::QueryContractReply
{
    using ReturnType = network::otdht::QueryContractReply;

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
        LogError()()(e.what()).Flush();

        return BlockchainSyncQueryContractReply();
    }
}

auto BlockchainSyncQueryContractReply_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::otdht::QueryContractReply>
{
    using ReturnType = network::otdht::QueryContractReply;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, type, id, payload).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class QueryContractReply::Imp final : public Base::Imp
{
public:
    const contract::Type contract_type_;
    const identifier::Generic contract_id_;
    const Space payload_;
    QueryContractReply* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asQueryContractReply() const noexcept
        -> const QueryContractReply& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asQueryContractReply();
        }
    }

    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        {
            using Buffer = boost::endian::little_uint32_buf_t;

            static_assert(sizeof(Buffer) == sizeof(MessageType));

            out.AddFrame(Buffer{
                static_cast<std::uint32_t>(MessageType::contract_query)});
        }
        {
            using Buffer = boost::endian::little_uint32_buf_t;

            static_assert(sizeof(Buffer) == sizeof(contract_type_));

            out.AddFrame(Buffer{static_cast<std::uint32_t>(contract_type_)});
        }

        out.Internal().AddFrame([&] {
            auto data = protobuf::Identifier{};
            contract_id_.Internal().Serialize(data);

            return data;
        }());
        out.AddFrame(payload_);

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , contract_type_(contract::Type::invalid)
        , contract_id_()
        , payload_()
        , parent_(nullptr)
    {
    }
    Imp(const contract::Type type,
        identifier::Generic&& id,
        Space&& payload) noexcept
        : Base::Imp(MessageType::contract)
        , contract_type_(type)
        , contract_id_(std::move(id))
        , payload_(std::move(payload))
        , parent_(nullptr)
    {
    }
    Imp(const contract::Type type,
        const identifier::Generic& id,
        Space&& payload) noexcept
        : Imp(type, identifier::Generic{id}, std::move(payload))
    {
    }
    Imp(const api::Session& api,
        const contract::Type type,
        const ReadView id,
        const ReadView payload) noexcept
        : Imp(type,
              api.Factory().Internal().Session().Identifier(
                  protobuf::Factory<protobuf::Identifier>(
                      id.data(),
                      id.size())),
              space(payload))
    {
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

QueryContractReply::QueryContractReply(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto QueryContractReply::ID() const noexcept -> const identifier::Generic&
{
    return Imp::get(imp_).contract_id_;
}

auto QueryContractReply::Payload() const noexcept -> ReadView
{
    return reader(Imp::get(imp_).payload_);
}

auto QueryContractReply::ContractType() const noexcept -> contract::Type
{
    return Imp::get(imp_).contract_type_;
}

QueryContractReply::~QueryContractReply()
{
    if (nullptr != QueryContractReply::imp_) {
        delete QueryContractReply::imp_;
        QueryContractReply::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht
