// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "crypto/library/packetcrypt/PacketCrypt.hpp"  // IWYU pragma: associated

extern "C" {
#include <Validate-fixed.h>
#include <packetcrypt/PacketCrypt.h>
}

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "blockchain/block/pkt/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace be = boost::endian;

namespace opentxs::crypto::implementation
{
struct PacketCrypt::Imp {
    using PktBlock = blockchain::block::pkt::Block;
    using Context = std::unique_ptr<
        ::PacketCrypt_ValidateCtx_t,
        decltype(&::ValidateCtx_destroy)>;

    static thread_local Context context_;

    const HeaderOracle& headers_;

    auto Validate(const PktBlock& block) const noexcept -> bool
    {
        try {
            if (0 == block.size()) { throw std::runtime_error{"Empty block"}; }

            const auto pTX = block.at(0);

            if ((!pTX) || (0 == pTX->Inputs().size())) {
                throw std::runtime_error{"Invalid generation transaction"};
            }

            const auto& tx = *pTX;
            const auto coinbase = tx.Inputs().at(0).Coinbase();

            if (0 == coinbase.size()) {
                throw std::runtime_error{"Invalid coinbase"};
            }

            const auto height =
                blockchain::block::bitcoin::internal::DecodeBip34(
                    reader(coinbase));

            if (0 > height) {
                throw std::runtime_error{"Failed to decode coinbase"};
            }

            if (std::numeric_limits<std::uint32_t>::max() < height) {
                throw std::runtime_error{"Invalid height"};
            }

            static constexpr auto threshold = decltype(height){122622};

            if (threshold > height) {
                LogDetail()(OT_PRETTY_CLASS())(
                    ": Validation protocol for this block height not "
                    "supported. Assuming block is valid.")
                    .Flush();

                return true;
            }

            const auto& serializedProof = [&]() -> const auto&
            {
                static constexpr auto proofType = std::byte{0x01};

                for (const auto& [type, payload] : block.GetProofs()) {
                    if (proofType == type) { return payload; }
                }

                throw std::runtime_error{"Proof not found"};
            }
            ();
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
                auto i{out.data()};

                for (const auto& ann : headerAndProof.announcements) {
                    const auto height =
                        be::little_uint32_buf_t{ann.hdr.parentBlockHeight};
                    const auto hash = headers_.BestHash(height.value());

                    if (32u != hash->size()) {
                        throw std::runtime_error{
                            "Failed to load parent block hash"};
                    }

                    std::memcpy(i, hash->data(), hash->size());
                    std::advance(i, hash->size());
                }

                return out;
            }();
            using Commitment = ::PacketCrypt_Coinbase_t;
            auto commitment = [&]() -> Commitment {
                for (const auto& output : tx.Outputs()) {
                    const auto& script = output.Script();
                    using Pattern = blockchain::block::bitcoin::Script::Pattern;

                    if (Pattern::NullData != script.Type()) { continue; }

                    const auto& element = script.at(1);
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
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    Imp(const HeaderOracle& oracle) noexcept
        : headers_(oracle)
    {
    }
};

thread_local PacketCrypt::Imp::Context PacketCrypt::Imp::context_{
    ::ValidateCtx_create(),
    ::ValidateCtx_destroy};

PacketCrypt::PacketCrypt(const HeaderOracle& oracle) noexcept
    : imp_(std::make_unique<Imp>(oracle))
{
}

auto PacketCrypt::Validate(const BitcoinBlock& block) const noexcept -> bool
{
    const auto* p = dynamic_cast<const Imp::PktBlock*>(&block);

    if (nullptr == p) {
        LogError()(OT_PRETTY_CLASS())("Invalid block type").Flush();

        return false;
    }

    return imp_->Validate(*p);
}

PacketCrypt::~PacketCrypt() = default;
}  // namespace opentxs::crypto::implementation
