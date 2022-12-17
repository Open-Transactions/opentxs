// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::Session

#include "opentxs/network/otdht/PushTransactionReply.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BlockchainSyncPushTransactionReply() noexcept
    -> network::otdht::PushTransactionReply
{
    using ReturnType = network::otdht::PushTransactionReply;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncPushTransactionReply(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::TransactionHash& id,
    const bool success) noexcept -> network::otdht::PushTransactionReply
{
    using ReturnType = network::otdht::PushTransactionReply;

    return std::make_unique<ReturnType::Imp>(chain, id, success).release();
}

auto BlockchainSyncPushTransactionReply_p(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::otdht::PushTransactionReply>
{
    using ReturnType = network::otdht::PushTransactionReply;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, chain, id, success).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class PushTransactionReply::Imp final : public Base::Imp
{
public:
    static constexpr auto success_byte_ = std::byte{0x00};
    static constexpr auto fail_byte_ = std::byte{0x01};

    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::block::TransactionHash txid_;
    const bool success_;
    PushTransactionReply* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asPushTransactionReply() const noexcept
        -> const PushTransactionReply& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asPushTransactionReply();
        }
    }
    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        using Buffer = boost::endian::little_uint32_buf_t;
        static constexpr auto type = MessageType::pushtx;

        static_assert(sizeof(Buffer) == sizeof(type));
        static_assert(sizeof(Buffer) == sizeof(chain_));

        out.AddFrame(Buffer{static_cast<std::uint32_t>(type)});
        out.AddFrame(Buffer{static_cast<std::uint32_t>(chain_)});
        opentxs::copy(txid_.Bytes(), out.AppendBytes());
        out.AddFrame([this] {
            if (success_) {

                return success_byte_;
            } else {

                return fail_byte_;
            }
        }());

        return true;
    }

    Imp() noexcept
        : Base::Imp()
        , chain_(opentxs::blockchain::Type::Unknown)
        , txid_()
        , success_(false)
        , parent_(nullptr)
    {
    }
    Imp(const opentxs::blockchain::Type chain,
        opentxs::blockchain::block::TransactionHash id,
        bool success) noexcept
        : Base::Imp(MessageType::pushtx_reply)
        , chain_(chain)
        , txid_(std::move(id))
        , success_(success)
        , parent_(nullptr)
    {
    }
    Imp(const api::Session& api,
        const opentxs::blockchain::Type chain,
        const ReadView id,
        const ReadView success) noexcept
        : Imp(chain, {id}, [&]() -> bool {
            if (0u == success.size()) { return false; }

            return success_byte_ ==
                   *reinterpret_cast<const std::byte*>(success.data());
        }())
    {
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

PushTransactionReply::PushTransactionReply(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto PushTransactionReply::Chain() const noexcept -> opentxs::blockchain::Type
{
    return Imp::get(imp_).chain_;
}

auto PushTransactionReply::ID() const noexcept
    -> const opentxs::blockchain::block::TransactionHash&
{
    return Imp::get(imp_).txid_;
}

auto PushTransactionReply::Success() const noexcept -> bool
{
    return Imp::get(imp_).success_;
}

PushTransactionReply::~PushTransactionReply()
{
    if (nullptr != PushTransactionReply::imp_) {
        delete PushTransactionReply::imp_;
        PushTransactionReply::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht
