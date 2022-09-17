// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/network/otdht/QueryContract.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <memory>
#include <utility>

#include "Proto.tpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/otdht/MessageType.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::factory
{
auto BlockchainSyncQueryContract() noexcept -> network::otdht::QueryContract
{
    using ReturnType = network::otdht::QueryContract;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncQueryContract(const identifier::Generic& id) noexcept
    -> network::otdht::QueryContract
{
    using ReturnType = network::otdht::QueryContract;

    return std::make_unique<ReturnType::Imp>(identifier::Generic{id}).release();
}

auto BlockchainSyncQueryContract_p(
    const api::Session& api,
    const ReadView id) noexcept
    -> std::unique_ptr<network::otdht::QueryContract>
{
    using ReturnType = network::otdht::QueryContract;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, id).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class QueryContract::Imp final : public Base::Imp
{
public:
    const identifier::Generic contract_id_;
    QueryContract* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asQueryContract() const noexcept -> const QueryContract& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asQueryContract();
        }
    }

    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        out.Internal().AddFrame([&] {
            auto out = proto::Identifier{};
            contract_id_.Serialize(out);

            return out;
        }());

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , contract_id_()
        , parent_(nullptr)
    {
    }
    Imp(identifier::Generic&& id) noexcept
        : Base::Imp(MessageType::contract_query)
        , contract_id_(std::move(id))
        , parent_(nullptr)
    {
    }
    Imp(const api::Session& api, const ReadView id) noexcept
        : Imp(api.Factory().InternalSession().Identifier(
              proto::Factory<proto::Identifier>(id.data(), id.size())))
    {
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

QueryContract::QueryContract(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto QueryContract::ID() const noexcept -> const identifier::Generic&
{
    return Imp::get(imp_).contract_id_;
}

QueryContract::~QueryContract()
{
    if (nullptr != QueryContract::imp_) {
        delete QueryContract::imp_;
        QueryContract::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht