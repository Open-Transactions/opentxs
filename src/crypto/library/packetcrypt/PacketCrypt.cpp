// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/packetcrypt/PacketCrypt.hpp"  // IWYU pragma: associated

// NOTE: this is necessary for g++ to parse packetcrypt/PacketCrypt.h
#ifndef _Static_assert
#define OPENTXS_STATIC_ASSERT_WORKAROUND_FOR_PACKETCRYPT
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-identifier"
// NOLINTNEXTLINE
#define _Static_assert static_assert
#pragma GCC diagnostic pop
#endif

extern "C" {
#include <Validate-fixed.h>
#include <packetcrypt/PacketCrypt.h>
}

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>

#include "blockchain/protocol/bitcoin/pkt/block/block/Imp.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/pkt/block/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Block.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::crypto::implementation
{
struct PacketCrypt::Imp {
    using PktBlock =
        blockchain::protocol::bitcoin::pkt::block::implementation::Block;
    using Context = std::unique_ptr<
        ::PacketCrypt_ValidateCtx_t,
        decltype(&::ValidateCtx_destroy)>;

    static thread_local Context context_;

    const blockchain::node::HeaderOracle& headers_;

    auto Validate(const PktBlock& block) const noexcept -> bool
    {
        try {
            if (0 == block.size()) { throw std::runtime_error{"Empty block"}; }

            const auto& tx = block.get()[0].asBitcoin();
            const auto inputs = tx.Inputs();

            if ((false == tx.IsValid()) || inputs.empty()) {
                throw std::runtime_error{"Invalid generation transaction"};
            }

            const auto coinbase = inputs[0].Coinbase();

            if (0 == coinbase.size()) {
                throw std::runtime_error{"Invalid coinbase"};
            }

            const auto height = blockchain::protocol::bitcoin::base::block::
                internal::DecodeBip34(coinbase);

            if (0 > height) {
                throw std::runtime_error{"Failed to decode coinbase"};
            }

            if (std::numeric_limits<std::uint32_t>::max() < height) {
                throw std::runtime_error{"Invalid height"};
            }

            static constexpr auto threshold = decltype(height){122622};

            if (threshold > height) {
                LogDetail()()(": Validation protocol for this block height not "
                              "supported. Assuming block is valid.")
                    .Flush();

                return true;
            }

            const auto& serializedProof = [&]() -> const auto& {
                static constexpr auto proofType =
                    blockchain::protocol::bitcoin::pkt::block::ProofType{0x01};

                for (const auto& [type, payload] : block.GetProofs()) {
                    if (proofType == type) { return payload; }
                }

                throw std::runtime_error{"Proof not found"};
            }();
            const auto sBytes = serializedProof.size();
            const auto hap = [&] {
                static constexpr auto headerSize = sizeof(
                    std::declval<PacketCrypt_HeaderAndProof_t>().blockHeader);
                static constexpr auto padSize =
                    sizeof(std::declval<PacketCrypt_HeaderAndProof_t>()._pad);
                auto out = space(sBytes + headerSize + padSize);

                if (std::numeric_limits<std::uint32_t>::max() < out.size()) {
                    throw std::runtime_error{"Proof too large"};
                }

                auto* i = out.data();

                if (!block.Header().Serialize(preallocated(headerSize, i))) {
                    throw std::runtime_error{
                        "Failed to serialize block header"};
                }

                std::advance(i, headerSize);
                static const auto pad = be::little_uint32_buf_t{0};
                static_assert(sizeof(pad) == padSize);
                std::memcpy(i, &pad, padSize);
                std::advance(i, padSize);
                std::memcpy(i, serializedProof.data(), sBytes);

                return out;
            }();
            const auto& headerAndProof =
                *reinterpret_cast<const ::PacketCrypt_HeaderAndProof_t*>(
                    hap.data());
            const auto hashes = [&] {
                auto out =
                    std::array<std::uint8_t, PacketCrypt_NUM_ANNS * 32>{};
                auto* i{out.data()};

                for (const auto& ann : headerAndProof.announcements) {
                    const auto parent =
                        be::little_uint32_buf_t{ann.hdr.parentBlockHeight};
                    const auto hash = headers_.BestHash(parent.value());

                    if (hash.IsNull()) {
                        throw std::runtime_error{
                            "Failed to load parent block hash"};
                    }

                    std::memcpy(i, hash.data(), hash.size());
                    std::advance(i, hash.size());
                }

                return out;
            }();
            using Commitment = ::PacketCrypt_Coinbase_t;
            auto commitment = [&]() -> Commitment {
                for (const auto& output : tx.Outputs()) {
                    const auto& script = output.Script();
                    using enum blockchain::protocol::bitcoin::base::block::
                        script::Pattern;

                    if (NullData != script.Type()) { continue; }

                    const auto& element = script.get()[1];
                    const auto& data = element.data_.value();
                    static constexpr auto target = sizeof(Commitment);

                    if (data.size() != target) { continue; }

                    const auto& out =
                        *reinterpret_cast<const Commitment*>(data.data());
                    const auto magic = be::little_uint32_buf_t{out.magic};
                    static constexpr auto expected =
                        std::uint32_t{PacketCrypt_Coinbase_MAGIC};

                    if (expected != magic.value()) { continue; }

                    return out;
                }

                throw std::runtime_error{"Commitment missing"};
            }();
            const auto rc = Validate_checkBlock_fixed(
                &headerAndProof,
                static_cast<std::uint32_t>(hap.size()),
                static_cast<std::uint32_t>(height),
                &commitment,
                hashes.data(),
                context_.get());

            if (0 == rc) {
                LogDetail()("PacketCrypt validation successful for block ")(
                    height)
                    .Flush();

                return true;
            } else {
                LogError()("PacketCrypt validation failed for block ")(height)
                    .Flush();

                return false;
            }
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            return false;
        }
    }

    Imp(const blockchain::node::HeaderOracle& oracle) noexcept
        : headers_(oracle)
    {
    }
};

thread_local PacketCrypt::Imp::Context PacketCrypt::Imp::context_{
    ::ValidateCtx_create(),
    ::ValidateCtx_destroy};

PacketCrypt::PacketCrypt(const blockchain::node::HeaderOracle& oracle) noexcept
    : imp_(std::make_unique<Imp>(oracle))
{
}

auto PacketCrypt::Validate(
    const blockchain::protocol::bitcoin::base::block::Block& block)
    const noexcept -> bool
{
    const auto* p = dynamic_cast<const Imp::PktBlock*>(&block);

    if (nullptr == p) {
        LogError()()("Invalid block type").Flush();

        return false;
    }

    return imp_->Validate(*p);
}

PacketCrypt::~PacketCrypt() = default;
}  // namespace opentxs::crypto::implementation

#if defined OPENTXS_STATIC_ASSERT_WORKAROUND_FOR_PACKETCRYPT
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#undef _Static_assert
#pragma GCC diagnostic pop
#endif
