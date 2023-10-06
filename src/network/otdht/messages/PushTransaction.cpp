// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::Session

#include "opentxs/network/otdht/PushTransaction.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/network/otdht/Block.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BlockchainSyncPushTransaction() noexcept -> network::otdht::PushTransaction
{
    using ReturnType = network::otdht::PushTransaction;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncPushTransaction(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Transaction& payload) noexcept
    -> network::otdht::PushTransaction
{
    using ReturnType = network::otdht::PushTransaction;

    try {
        return std::make_unique<ReturnType::Imp>(
                   chain,
                   payload.ID(),
                   [&] {
                       auto out = Space{};
                       const auto rc = payload.Internal().asBitcoin().Serialize(
                           writer(out));

                       if (false == rc.has_value()) {
                           throw std::runtime_error{"failed to serialize nym"};
                       }

                       return out;
                   }())
            .release();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return BlockchainSyncPushTransaction();
    }
}
auto BlockchainSyncPushTransaction_p(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::otdht::PushTransaction>
{
    using ReturnType = network::otdht::PushTransaction;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, chain, id, payload).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class PushTransaction::Imp final : public Base::Imp
{
public:
    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::block::TransactionHash txid_;
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
        opentxs::copy(txid_.Bytes(), out.AppendBytes());
        out.AddFrame(payload_);

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , chain_(opentxs::blockchain::Type::UnknownBlockchain)
        , txid_()
        , payload_()
        , parent_(nullptr)
    {
    }
    Imp(const opentxs::blockchain::Type chain,
        opentxs::blockchain::block::TransactionHash id,
        Space&& payload) noexcept
        : Base::Imp(MessageType::pushtx)
        , chain_(chain)
        , txid_(std::move(id))
        , payload_(std::move(payload))
        , parent_(nullptr)
    {
    }
    Imp(const api::Session& api,
        const opentxs::blockchain::Type chain,
        const ReadView id,
        const ReadView payload) noexcept
        : Imp(chain, {id}, space(payload))
    {
    }
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
    -> const opentxs::blockchain::block::TransactionHash&
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
}  // namespace opentxs::network::otdht
