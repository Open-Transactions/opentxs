// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/network/p2p/PublishContractReply.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "Proto.tpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "serialization/protobuf/Identifier.pb.h"

namespace opentxs::factory
{
auto BlockchainSyncPublishContractReply() noexcept
    -> network::p2p::PublishContractReply
{
    using ReturnType = network::p2p::PublishContractReply;

    return std::make_unique<ReturnType::Imp>().release();
}

auto BlockchainSyncPublishContractReply(
    const Identifier& id,
    const bool success) noexcept -> network::p2p::PublishContractReply
{
    using ReturnType = network::p2p::PublishContractReply;

    return std::make_unique<ReturnType::Imp>(id, success).release();
}

auto BlockchainSyncPublishContractReply_p(
    const api::Session& api,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::p2p::PublishContractReply>
{
    using ReturnType = network::p2p::PublishContractReply;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(api, id, success).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::p2p
{
class PublishContractReply::Imp final : public Base::Imp
{
public:
    static constexpr auto success_byte_ = std::byte{0x00};
    static constexpr auto fail_byte_ = std::byte{0x01};

    const OTIdentifier contract_id_;
    const bool success_;
    PublishContractReply* parent_;

    static auto get(const Imp* imp) noexcept -> const Imp&
    {
        if (nullptr == imp) {
            static const auto blank = Imp{};

            return blank;
        } else {

            return *imp;
        }
    }

    auto asPublishContractReply() const noexcept
        -> const PublishContractReply& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asPublishContractReply();
        }
    }
    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        if (false == serialize_type(out)) { return false; }

        using Buffer = boost::endian::little_uint32_buf_t;
        static constexpr auto type = MessageType::publish_contract;

        static_assert(sizeof(Buffer) == sizeof(type));

        out.AddFrame(Buffer{static_cast<std::uint32_t>(type)});
        out.Internal().AddFrame([&] {
            auto out = proto::Identifier{};
            contract_id_->Serialize(out);

            return out;
        }());
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
        , contract_id_(Identifier::Factory())
        , success_(false)
        , parent_(nullptr)
    {
    }
    Imp(OTIdentifier&& id, bool success) noexcept
        : Base::Imp(MessageType::publish_ack)
        , contract_id_(std::move(id))
        , success_(success)
        , parent_(nullptr)
    {
    }
    Imp(const api::Session& api,
        const ReadView id,
        const ReadView success) noexcept
        : Imp(api.Factory().InternalSession().Identifier(
                  proto::Factory<proto::Identifier>(id.data(), id.size())),
              [&]() -> bool {
                  if (0u == success.size()) { return false; }

                  return success_byte_ ==
                         *reinterpret_cast<const std::byte*>(success.data());
              }())
    {
    }

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

PublishContractReply::PublishContractReply(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto PublishContractReply::ID() const noexcept -> const Identifier&
{
    return Imp::get(imp_).contract_id_;
}

auto PublishContractReply::Success() const noexcept -> bool
{
    return Imp::get(imp_).success_;
}

PublishContractReply::~PublishContractReply()
{
    if (nullptr != PublishContractReply::imp_) {
        delete PublishContractReply::imp_;
        PublishContractReply::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::p2p
