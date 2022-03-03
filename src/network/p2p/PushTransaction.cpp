// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/network/p2p/PushTransaction.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainSyncPushTransaction() noexcept -> network::p2p::PushTransaction
{
    using ReturnType = network::p2p::PushTransaction;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncPushTransaction(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::bitcoin::Transaction& payload) noexcept
    -> network::p2p::PushTransaction
{
    using ReturnType = network::p2p::PushTransaction;

    try {
        return std::make_unique<ReturnType::Imp>(
                   chain,
                   payload.ID(),
                   [&] {
                       auto out = Space{};
                       const auto rc =
                           payload.Internal().Serialize(writer(out));

                       if (false == rc.has_value()) {
                           throw std::runtime_error{"failed to serialize nym"};
                       }

                       return out;
                   }())
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return BlockchainSyncPushTransaction();
    }
}
auto BlockchainSyncPushTransaction_p(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::p2p::PushTransaction>
{
    using ReturnType = network::p2p::PushTransaction;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, chain, id, payload).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::p2p
{
class PushTransaction::Imp final : public Base::Imp
{
public:
    const opentxs::blockchain::Type chain_;
    const OTData txid_;
    const Space payload_;
    PushTransaction* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asPushTransaction() const noexcept -> const PushTransaction& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asPushTransaction();
        }
    }
    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        using Buffer = boost::endian::little_uint32_buf_t;

        static_assert(sizeof(Buffer) == sizeof(chain_));

        out.AddFrame(Buffer{static_cast<std::uint32_t>(chain_)});
        opentxs::copy(txid_->Bytes(), out.AppendBytes());
        out.AddFrame(payload_);

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , chain_(opentxs::blockchain::Type::Unknown)
        , txid_(opentxs::Data::Factory())
        , payload_()
        , parent_(nullptr)
    {
    }
    Imp(const opentxs::blockchain::Type chain,
        OTData&& id,
        Space&& payload) noexcept
        : Base::Imp(MessageType::pushtx)
        , chain_(chain)
        , txid_(std::move(id))
        , payload_(std::move(payload))
        , parent_(nullptr)
    {
    }
    Imp(const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Txid& id,
        Space&& payload) noexcept
        : Imp(chain, OTData{id}, std::move(payload))
    {
    }
    Imp(const api::Session& api,
        const opentxs::blockchain::Type chain,
        const ReadView id,
        const ReadView payload) noexcept
        : Imp(chain, api.Factory().Data(id), space(payload))
    {
    }

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

PushTransaction::PushTransaction(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto PushTransaction::Chain() const noexcept -> opentxs::blockchain::Type
{
    return Imp::get(imp_).chain_;
}

auto PushTransaction::ID() const noexcept
    -> const opentxs::blockchain::block::Txid&
{
    return Imp::get(imp_).txid_;
}

auto PushTransaction::Payload() const noexcept -> ReadView
{
    return reader(Imp::get(imp_).payload_);
}

PushTransaction::~PushTransaction()
{
    if (nullptr != PushTransaction::imp_) {
        delete PushTransaction::imp_;
        PushTransaction::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::p2p
