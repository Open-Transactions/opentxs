// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/cashtoken/Types.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransactionOutput.pb.h>
#include <opentxs/protobuf/Cashtoken.pb.h>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/OP.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/cashtoken/Capability.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
{
auto Value::Bytes() const noexcept -> std::size_t
{
    return bytes(nullptr, nullptr);
}

auto Value::bytes(
    network::blockchain::bitcoin::CompactSize* commitment,
    network::blockchain::bitcoin::CompactSize* amount) const noexcept
    -> std::size_t
{
    using namespace opentxs::network::blockchain::bitcoin;
    static constexpr auto marker = sizeof(
        std::underlying_type_t<protocol::bitcoin::base::block::script::OP>);
    static constexpr auto category = decltype(category_)::payload_size_;
    static constexpr auto bitfield = sizeof(marker_type);
    static_assert(marker + category + bitfield == 34_uz);
    const auto cSize = [&, this]() -> std::size_t {
        if (has_commitment()) {
            auto cs = CompactSize{commitment_->size()};
            auto out = cs.Total();

            if (nullptr != commitment) { *commitment = std::move(cs); }

            return out;
        } else {

            return 0_uz;
        }
    }();
    const auto aSize = [&, this]() -> std::size_t {
        if (has_amount()) {
            auto cs = CompactSize{amount_->Internal().ExtractUInt64()};
            auto out = cs.Size();

            if (nullptr != amount) { *amount = std::move(cs); }

            return out;
        } else {

            return 0_uz;
        }
    }();

    return marker + category + bitfield + cSize + aSize;
}

auto Value::has_amount() const noexcept -> bool { return amount_.has_value(); }

auto Value::has_commitment() const noexcept -> bool
{
    return commitment_.has_value();
}

auto Value::Serialize(Writer&& out) const noexcept(false) -> void
{
    using namespace opentxs::network::blockchain::bitcoin;
    using protocol::bitcoin::base::block::script::OP;
    static constexpr auto marker =
        static_cast<std::underlying_type_t<OP>>(OP::PREFIX_TOKEN);
    const auto bitfield = [this] {
        auto bf = marker_type{};

        if (commitment_.has_value() && (0_uz < commitment_->size())) {
            bf |= commitment_mask_;
        }

        if (nft_) {
            bf |= nft_mask_;
            bf |= static_cast<std::underlying_type_t<decltype(capability_)>>(
                capability_);
        }

        if (amount_.has_value()) { bf |= amount_mask_; }

        return bf;
    }();
    auto cSize = CompactSize{};
    auto aSize = CompactSize{};
    const auto size = bytes(std::addressof(cSize), std::addressof(aSize));
    auto buf = reserve(std::move(out), size, "cashtoken");
    check_exactly(buf, size, "cashtoken");
    serialize_object(marker, buf, "marker");
    copy(category_.Bytes(), buf, "category");
    serialize_object(bitfield, buf, "bitfield");

    if (has_commitment()) {
        serialize_compact_size(cSize, buf, "commitment size");
        copy(commitment_->Bytes(), buf, "commitment");
    }

    if (has_amount()) { serialize_compact_size(aSize, buf, "amount"); }
}

auto Value::Serialize(protobuf::BlockchainTransactionOutput& out) const noexcept
    -> void
{
    static constexpr auto version = VersionNumber{1};
    auto& proto = *out.mutable_cashtoken();
    proto.set_version(version);
    proto.set_category(category_.data(), category_.size());
    proto.set_nft(nft_);
    proto.set_capability(
        static_cast<std::underlying_type_t<decltype(capability_)>>(
            capability_));

    if (has_commitment()) {
        proto.set_commitment(commitment_->data(), commitment_->size());
    }

    if (has_amount()) { amount_->Serialize(writer(proto.mutable_amount())); }
}

auto Value::View() const noexcept -> cashtoken::View
{
    return {
        category_.Bytes(),
        nft_,
        capability_,
        has_commitment() ? commitment_->Bytes() : ReadView{},
        has_amount() ? std::addressof(*amount_) : nullptr};
}
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken

namespace opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
{
auto deserialize(const protobuf::BlockchainTransactionOutput& in) noexcept(
    false) -> std::optional<Value>
{
    if (false == in.has_cashtoken()) { return std::nullopt; }

    const auto& proto = in.cashtoken();
    auto out = Value{};

    if (false == out.category_.Assign(proto.category())) {

        throw std::runtime_error{"invalid category"};
    }

    out.nft_ = proto.nft();
    using enum Capability;

    switch (proto.capability()) {
        case 0u: {
            out.capability_ = none;
        } break;
        case 1u: {
            out.capability_ = mutable_token;
        } break;
        case 2u: {
            out.capability_ = minting;
        } break;
        default: {

            throw std::runtime_error{"invalid capability"};
        }
    }

    if (proto.has_commitment()) { out.commitment_.emplace(proto.commitment()); }

    if (proto.has_amount()) {
        out.amount_.emplace(factory::Amount(proto.amount()));
    }

    return out;
}

auto deserialize(ReadView& in, std::optional<Value>& out) noexcept(false)
    -> void
{
    using protocol::bitcoin::base::block::script::OP;
    static constexpr auto marker =
        static_cast<std::underlying_type_t<OP>>(OP::PREFIX_TOKEN);
    out.reset();

    if (in.size() < sizeof(marker)) { return; }

    if (0 != std::memcmp(&marker, in.data(), sizeof(marker))) { return; }

    in.remove_prefix(sizeof(marker));
    auto& val = out.emplace();
    const auto rc = val.category_.Assign(
        extract_prefix(in, val.category_.size(), "category"));

    if (false == rc) { throw std::runtime_error{"failed to assign category"}; }

    auto bitfield = Value::marker_type{};
    deserialize_object(in, bitfield, "bitfield");
    const auto hasNft = 0u != (bitfield & Value::nft_mask_);
    const auto hasCommitment = 0u != (bitfield & Value::commitment_mask_);
    const auto hasAmount = 0u != (bitfield & Value::amount_mask_);
    val.capability_ = [&] {
        using enum Capability;

        if (hasNft) {
            val.nft_ = true;

            switch (bitfield & Value::capability_mask_) {
                case 1u: {

                    return mutable_token;
                }
                case 2u: {

                    return minting;
                }
                default: {

                    return none;
                }
            }
        } else {

            return none;
        }
    }();
    using namespace opentxs::network::blockchain::bitcoin;

    if (hasCommitment) {
        const auto size = DecodeCompactSize(in);

        if (false == size.has_value()) {

            throw std::runtime_error{"unable to decode commitment size"};
        }

        auto& commitment = val.commitment_.emplace();
        commitment.Assign(extract_prefix(in, *size, "commitment"));
    }

    if (hasAmount) {
        const auto value = DecodeCompactSize(in);

        if (false == value.has_value()) {

            throw std::runtime_error{"unable to decode amount"};
        }

        val.amount_.emplace(*value);
    }
}
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
