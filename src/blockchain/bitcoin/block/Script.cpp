// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::OP
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "blockchain/bitcoin/block/Script.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <compare>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinScriptNullData(
    const blockchain::Type chain,
    const UnallocatedVector<ReadView>& data) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::RETURN));

    for (const auto& element : data) {
        elements.emplace_back(bb::internal::PushData(element));
    }

    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    const UnallocatedVector<
        const opentxs::crypto::asymmetric::key::EllipticCurve*>&
        publicKeys) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    if ((0u == M) || (16u < M)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid M").Flush();

        return {};
    }

    if ((0u == N) || (16u < N)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid N").Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(M + 80)));

    for (const auto& pKey : publicKeys) {
        if (nullptr == pKey) {
            LogError()("opentxs::factory::")(__func__)(": Invalid key").Flush();

            return {};
        }

        const auto& key = *pKey;
        elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    }

    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(N + 80)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKMULTISIG));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2PK(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace bb = blockchain::bitcoin::block;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::DUP));
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUALVERIFY));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to serialize script")
            .Flush();

        return {};
    }

    if (false == b::ScriptHash(crypto, chain, reader(bytes), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate script hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUAL));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to serialize script")
            .Flush();

        return {};
    }

    if (false ==
        b::ScriptHashSegwit(crypto, chain, reader(bytes), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate script hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    using Position = blockchain::bitcoin::block::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto BitcoinScript(
    const blockchain::Type chain,
    const ReadView bytes,
    const blockchain::bitcoin::block::Script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Script;
    auto elements = blockchain::bitcoin::block::ScriptElements{};

    if ((nullptr == bytes.data()) || (0 == bytes.size()) ||
        (ReturnType::Position::Coinbase == role)) {
        return std::make_unique<ReturnType>(
            chain, role, std::move(elements), 0);
    }

    elements.reserve(bytes.size());
    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());
    auto read = 0_uz;
    const auto target = bytes.size();
    const auto& logger = mute ? LogTrace() : LogVerbose();

    try {
        while (read < target) {
            auto& element = elements.emplace_back();
            auto& [opcode, invalid, size, data] = element;

            try {
                opcode = ReturnType::decode(*it);
            } catch (...) {
                if (allowInvalidOpcodes) {
                    opcode = blockchain::bitcoin::block::OP::INVALIDOPCODE;
                    invalid = *it;
                } else {
                    throw;
                }
            }

            read += 1;
            std::advance(it, 1);
            const auto direct = ReturnType::is_direct_push(opcode);

            if (direct.has_value()) {
                const auto& pushSize = direct.value();
                const auto remaining = target - read;
                const auto effectiveSize = allowInvalidOpcodes
                                               ? std::min(pushSize, remaining)
                                               : pushSize;

                if (((read + effectiveSize) > target)) {
                    logger("opentxs::factory::")(__func__)(
                        ": Incomplete direct data push")
                        .Flush();

                    return {};
                }

                data = space(effectiveSize);

                if (0u < effectiveSize) {
                    std::memcpy(data.value().data(), it, effectiveSize);
                }

                read += effectiveSize;
                std::advance(it, effectiveSize);

                continue;
            }

            const auto push = ReturnType::is_push(opcode);

            if (push.has_value()) {
                auto buf = be::little_uint32_buf_t{};

                {
                    const auto& sizeBytes = push.value();

                    OT_ASSERT(0 < sizeBytes);
                    OT_ASSERT(5 > sizeBytes);

                    const auto remaining = target - read;
                    const auto effectiveSize =
                        allowInvalidOpcodes ? std::min(sizeBytes, remaining)
                                            : sizeBytes;

                    if ((read + effectiveSize) > target) {
                        logger("opentxs::factory::")(__func__)(
                            ": Incomplete data push")
                            .Flush();

                        return {};
                    }

                    if (0u < effectiveSize) {
                        size = space(effectiveSize);
                        std::memcpy(size.value().data(), it, effectiveSize);
                        read += effectiveSize;
                        std::advance(it, effectiveSize);
                        std::memcpy(
                            static_cast<void*>(&buf),
                            size.value().data(),
                            effectiveSize);
                    }
                }

                {
                    const auto pushSize = std::size_t{buf.value()};
                    const auto remaining = target - read;
                    const auto effectiveSize =
                        allowInvalidOpcodes ? std::min(pushSize, remaining)
                                            : pushSize;

                    if ((read + effectiveSize) > target) {
                        logger("opentxs::factory::")(__func__)(
                            ": Data push bytes missing")
                            .Flush();

                        return {};
                    }

                    if (0u < effectiveSize) {
                        data = space(effectiveSize);
                        std::memcpy(data.value().data(), it, effectiveSize);
                    }

                    read += effectiveSize;
                    std::advance(it, effectiveSize);
                }

                continue;
            }
        }
    } catch (...) {
        logger("opentxs::factory::")(__func__)(": Unknown opcode").Flush();

        return {};
    }

    elements.shrink_to_fit();

    try {
        return std::make_unique<ReturnType>(
            chain, role, std::move(elements), bytes.size());
    } catch (const std::exception& e) {
        LogVerbose()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinScript(
    const blockchain::Type chain,
    blockchain::bitcoin::block::ScriptElements&& elements,
    const blockchain::bitcoin::block::Script::Position role) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Script;

    if (false == ReturnType::validate(elements)) {
        LogVerbose()("opentxs::factory::")(__func__)(": Invalid elements")
            .Flush();

        return {};
    }

    if ((0 == elements.size()) && (ReturnType::Position::Output == role)) {
        LogVerbose()("opentxs::factory::")(__func__)(": Empty input").Flush();

        return {};
    }

    elements.shrink_to_fit();

    return std::make_unique<ReturnType>(chain, role, std::move(elements));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::internal
{
auto Script::blank_signature(const blockchain::Type chain) noexcept
    -> const Space&
{
    static const auto output = space(72);

    return output;
}

auto Script::blank_pubkey(
    const blockchain::Type chain,
    const bool mode) noexcept -> const Space&
{
    static const auto compressed = space(33);
    static const auto uncompressed = space(65);

    return mode ? compressed : uncompressed;
}
}  // namespace opentxs::blockchain::bitcoin::block::internal

namespace opentxs::blockchain::bitcoin::block::implementation
{
Script::Script(
    const blockchain::Type chain,
    const Position role,
    ScriptElements&& elements,
    std::optional<std::size_t> size) noexcept
    : chain_(chain)
    , role_(role)
    , elements_(std::move(elements))
    , type_(get_type(role_, elements_))
    , size_(size)
{
}

Script::Script(const Script& rhs) noexcept
    : chain_(rhs.chain_)
    , role_(rhs.role_)
    , elements_(rhs.elements_)
    , type_(rhs.type_)
    , size_(rhs.size_)
{
}

auto Script::bytes(const value_type& element) noexcept -> std::size_t
{
    const auto& [opcode, invalid, bytes, data] = element;

    return (invalid.has_value() ? sizeof(invalid.value()) : sizeof(opcode)) +
           (bytes.has_value() ? bytes.value().size() : 0u) +
           (data.has_value() ? data.value().size() : 0u);
}

auto Script::bytes(const ScriptElements& script) noexcept -> std::size_t
{
    return std::accumulate(
        std::begin(script),
        std::end(script),
        0_uz,
        [](const std::size_t& lhs, const value_type& rhs) -> std::size_t {
            return lhs + bytes(rhs);
        });
}

auto Script::CalculateHash160(const api::Crypto& crypto, Writer&& output)
    const noexcept -> bool
{
    auto preimage = Space{};

    if (false == Serialize(writer(preimage))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

        return false;
    }

    return blockchain::ScriptHash(
        crypto, chain_, reader(preimage), std::move(output));
}

auto Script::CalculateSize() const noexcept -> std::size_t
{
    if (false == size_.has_value()) { size_ = bytes(elements_); }

    return size_.value();
}

auto Script::decode(const std::byte in) noexcept(false) -> OP
{
    using enum OP;
    static constexpr auto map = frozen::make_unordered_map<std::uint8_t, OP>({
        {static_cast<std::uint8_t>(ZERO), ZERO},
        {static_cast<std::uint8_t>(PUSHDATA_1), PUSHDATA_1},
        {static_cast<std::uint8_t>(PUSHDATA_2), PUSHDATA_2},
        {static_cast<std::uint8_t>(PUSHDATA_3), PUSHDATA_3},
        {static_cast<std::uint8_t>(PUSHDATA_4), PUSHDATA_4},
        {static_cast<std::uint8_t>(PUSHDATA_5), PUSHDATA_5},
        {static_cast<std::uint8_t>(PUSHDATA_6), PUSHDATA_6},
        {static_cast<std::uint8_t>(PUSHDATA_7), PUSHDATA_7},
        {static_cast<std::uint8_t>(PUSHDATA_8), PUSHDATA_8},
        {static_cast<std::uint8_t>(PUSHDATA_9), PUSHDATA_9},
        {static_cast<std::uint8_t>(PUSHDATA_10), PUSHDATA_10},
        {static_cast<std::uint8_t>(PUSHDATA_11), PUSHDATA_11},
        {static_cast<std::uint8_t>(PUSHDATA_12), PUSHDATA_12},
        {static_cast<std::uint8_t>(PUSHDATA_13), PUSHDATA_13},
        {static_cast<std::uint8_t>(PUSHDATA_14), PUSHDATA_14},
        {static_cast<std::uint8_t>(PUSHDATA_15), PUSHDATA_15},
        {static_cast<std::uint8_t>(PUSHDATA_16), PUSHDATA_16},
        {static_cast<std::uint8_t>(PUSHDATA_17), PUSHDATA_17},
        {static_cast<std::uint8_t>(PUSHDATA_18), PUSHDATA_18},
        {static_cast<std::uint8_t>(PUSHDATA_19), PUSHDATA_19},
        {static_cast<std::uint8_t>(PUSHDATA_20), PUSHDATA_20},
        {static_cast<std::uint8_t>(PUSHDATA_21), PUSHDATA_21},
        {static_cast<std::uint8_t>(PUSHDATA_22), PUSHDATA_22},
        {static_cast<std::uint8_t>(PUSHDATA_23), PUSHDATA_23},
        {static_cast<std::uint8_t>(PUSHDATA_24), PUSHDATA_24},
        {static_cast<std::uint8_t>(PUSHDATA_25), PUSHDATA_25},
        {static_cast<std::uint8_t>(PUSHDATA_26), PUSHDATA_26},
        {static_cast<std::uint8_t>(PUSHDATA_27), PUSHDATA_27},
        {static_cast<std::uint8_t>(PUSHDATA_28), PUSHDATA_28},
        {static_cast<std::uint8_t>(PUSHDATA_29), PUSHDATA_29},
        {static_cast<std::uint8_t>(PUSHDATA_30), PUSHDATA_30},
        {static_cast<std::uint8_t>(PUSHDATA_31), PUSHDATA_31},
        {static_cast<std::uint8_t>(PUSHDATA_32), PUSHDATA_32},
        {static_cast<std::uint8_t>(PUSHDATA_33), PUSHDATA_33},
        {static_cast<std::uint8_t>(PUSHDATA_34), PUSHDATA_34},
        {static_cast<std::uint8_t>(PUSHDATA_35), PUSHDATA_35},
        {static_cast<std::uint8_t>(PUSHDATA_36), PUSHDATA_36},
        {static_cast<std::uint8_t>(PUSHDATA_37), PUSHDATA_37},
        {static_cast<std::uint8_t>(PUSHDATA_38), PUSHDATA_38},
        {static_cast<std::uint8_t>(PUSHDATA_39), PUSHDATA_39},
        {static_cast<std::uint8_t>(PUSHDATA_40), PUSHDATA_40},
        {static_cast<std::uint8_t>(PUSHDATA_41), PUSHDATA_41},
        {static_cast<std::uint8_t>(PUSHDATA_42), PUSHDATA_42},
        {static_cast<std::uint8_t>(PUSHDATA_43), PUSHDATA_43},
        {static_cast<std::uint8_t>(PUSHDATA_44), PUSHDATA_44},
        {static_cast<std::uint8_t>(PUSHDATA_45), PUSHDATA_45},
        {static_cast<std::uint8_t>(PUSHDATA_46), PUSHDATA_46},
        {static_cast<std::uint8_t>(PUSHDATA_47), PUSHDATA_47},
        {static_cast<std::uint8_t>(PUSHDATA_48), PUSHDATA_48},
        {static_cast<std::uint8_t>(PUSHDATA_49), PUSHDATA_49},
        {static_cast<std::uint8_t>(PUSHDATA_50), PUSHDATA_50},
        {static_cast<std::uint8_t>(PUSHDATA_51), PUSHDATA_51},
        {static_cast<std::uint8_t>(PUSHDATA_52), PUSHDATA_52},
        {static_cast<std::uint8_t>(PUSHDATA_53), PUSHDATA_53},
        {static_cast<std::uint8_t>(PUSHDATA_54), PUSHDATA_54},
        {static_cast<std::uint8_t>(PUSHDATA_55), PUSHDATA_55},
        {static_cast<std::uint8_t>(PUSHDATA_56), PUSHDATA_56},
        {static_cast<std::uint8_t>(PUSHDATA_57), PUSHDATA_57},
        {static_cast<std::uint8_t>(PUSHDATA_58), PUSHDATA_58},
        {static_cast<std::uint8_t>(PUSHDATA_59), PUSHDATA_59},
        {static_cast<std::uint8_t>(PUSHDATA_60), PUSHDATA_60},
        {static_cast<std::uint8_t>(PUSHDATA_61), PUSHDATA_61},
        {static_cast<std::uint8_t>(PUSHDATA_62), PUSHDATA_62},
        {static_cast<std::uint8_t>(PUSHDATA_63), PUSHDATA_63},
        {static_cast<std::uint8_t>(PUSHDATA_64), PUSHDATA_64},
        {static_cast<std::uint8_t>(PUSHDATA_65), PUSHDATA_65},
        {static_cast<std::uint8_t>(PUSHDATA_66), PUSHDATA_66},
        {static_cast<std::uint8_t>(PUSHDATA_67), PUSHDATA_67},
        {static_cast<std::uint8_t>(PUSHDATA_68), PUSHDATA_68},
        {static_cast<std::uint8_t>(PUSHDATA_69), PUSHDATA_69},
        {static_cast<std::uint8_t>(PUSHDATA_70), PUSHDATA_70},
        {static_cast<std::uint8_t>(PUSHDATA_71), PUSHDATA_71},
        {static_cast<std::uint8_t>(PUSHDATA_72), PUSHDATA_72},
        {static_cast<std::uint8_t>(PUSHDATA_73), PUSHDATA_73},
        {static_cast<std::uint8_t>(PUSHDATA_74), PUSHDATA_74},
        {static_cast<std::uint8_t>(PUSHDATA_75), PUSHDATA_75},
        {static_cast<std::uint8_t>(PUSHDATA1), PUSHDATA1},
        {static_cast<std::uint8_t>(PUSHDATA2), PUSHDATA2},
        {static_cast<std::uint8_t>(PUSHDATA4), PUSHDATA4},
        {static_cast<std::uint8_t>(ONE_NEGATE), ONE_NEGATE},
        {static_cast<std::uint8_t>(RESERVED), RESERVED},
        {static_cast<std::uint8_t>(ONE), ONE},
        {static_cast<std::uint8_t>(TWO), TWO},
        {static_cast<std::uint8_t>(THREE), THREE},
        {static_cast<std::uint8_t>(FOUR), FOUR},
        {static_cast<std::uint8_t>(FIVE), FIVE},
        {static_cast<std::uint8_t>(SIX), SIX},
        {static_cast<std::uint8_t>(SEVEN), SEVEN},
        {static_cast<std::uint8_t>(EIGHT), EIGHT},
        {static_cast<std::uint8_t>(NINE), NINE},
        {static_cast<std::uint8_t>(TEN), TEN},
        {static_cast<std::uint8_t>(ELEVEN), ELEVEN},
        {static_cast<std::uint8_t>(TWELVE), TWELVE},
        {static_cast<std::uint8_t>(THIRTEEN), THIRTEEN},
        {static_cast<std::uint8_t>(FOURTEEN), FOURTEEN},
        {static_cast<std::uint8_t>(FIFTEEN), FIFTEEN},
        {static_cast<std::uint8_t>(SIXTEEN), SIXTEEN},
        {static_cast<std::uint8_t>(NOP), NOP},
        {static_cast<std::uint8_t>(VER), VER},
        {static_cast<std::uint8_t>(IF), IF},
        {static_cast<std::uint8_t>(NOTIF), NOTIF},
        {static_cast<std::uint8_t>(VERIF), VERIF},
        {static_cast<std::uint8_t>(VERNOTIF), VERNOTIF},
        {static_cast<std::uint8_t>(ELSE), ELSE},
        {static_cast<std::uint8_t>(ENDIF), ENDIF},
        {static_cast<std::uint8_t>(VERIFY), VERIFY},
        {static_cast<std::uint8_t>(RETURN), RETURN},
        {static_cast<std::uint8_t>(TOALTSTACK), TOALTSTACK},
        {static_cast<std::uint8_t>(FROMALTSTACK), FROMALTSTACK},
        {static_cast<std::uint8_t>(TWO_DROP), TWO_DROP},
        {static_cast<std::uint8_t>(TWO_DUP), TWO_DUP},
        {static_cast<std::uint8_t>(THREE_DUP), THREE_DUP},
        {static_cast<std::uint8_t>(TWO_OVER), TWO_OVER},
        {static_cast<std::uint8_t>(TWO_ROT), TWO_ROT},
        {static_cast<std::uint8_t>(TWO_SWAP), TWO_SWAP},
        {static_cast<std::uint8_t>(IFDUP), IFDUP},
        {static_cast<std::uint8_t>(DEPTH), DEPTH},
        {static_cast<std::uint8_t>(DROP), DROP},
        {static_cast<std::uint8_t>(DUP), DUP},
        {static_cast<std::uint8_t>(NIP), NIP},
        {static_cast<std::uint8_t>(OVER), OVER},
        {static_cast<std::uint8_t>(PICK), PICK},
        {static_cast<std::uint8_t>(ROLL), ROLL},
        {static_cast<std::uint8_t>(ROT), ROT},
        {static_cast<std::uint8_t>(SWAP), SWAP},
        {static_cast<std::uint8_t>(TUCK), TUCK},
        {static_cast<std::uint8_t>(CAT), CAT},
        {static_cast<std::uint8_t>(SUBSTR), SUBSTR},
        {static_cast<std::uint8_t>(LEFT), LEFT},
        {static_cast<std::uint8_t>(RIGHT), RIGHT},
        {static_cast<std::uint8_t>(SIZE), SIZE},
        {static_cast<std::uint8_t>(INVERT), INVERT},
        {static_cast<std::uint8_t>(AND), AND},
        {static_cast<std::uint8_t>(OR), OR},
        {static_cast<std::uint8_t>(XOR), XOR},
        {static_cast<std::uint8_t>(EQUAL), EQUAL},
        {static_cast<std::uint8_t>(EQUALVERIFY), EQUALVERIFY},
        {static_cast<std::uint8_t>(RESERVED1), RESERVED1},
        {static_cast<std::uint8_t>(RESERVED2), RESERVED2},
        {static_cast<std::uint8_t>(ONE_ADD), ONE_ADD},
        {static_cast<std::uint8_t>(ONE_SUB), ONE_SUB},
        {static_cast<std::uint8_t>(TWO_MUL), TWO_MUL},
        {static_cast<std::uint8_t>(TWO_DIV), TWO_DIV},
        {static_cast<std::uint8_t>(NEGATE), NEGATE},
        {static_cast<std::uint8_t>(ABS), ABS},
        {static_cast<std::uint8_t>(NOT), NOT},
        {static_cast<std::uint8_t>(ZERO_NOTEQUAL), ZERO_NOTEQUAL},
        {static_cast<std::uint8_t>(ADD), ADD},
        {static_cast<std::uint8_t>(SUB), SUB},
        {static_cast<std::uint8_t>(MUL), MUL},
        {static_cast<std::uint8_t>(DIV), DIV},
        {static_cast<std::uint8_t>(MOD), MOD},
        {static_cast<std::uint8_t>(LSHIFT), LSHIFT},
        {static_cast<std::uint8_t>(RSHIFT), RSHIFT},
        {static_cast<std::uint8_t>(BOOLAND), BOOLAND},
        {static_cast<std::uint8_t>(BOOLOR), BOOLOR},
        {static_cast<std::uint8_t>(NUMEQUAL), NUMEQUAL},
        {static_cast<std::uint8_t>(NUMEQUALVERIFY), NUMEQUALVERIFY},
        {static_cast<std::uint8_t>(NUMNOTEQUAL), NUMNOTEQUAL},
        {static_cast<std::uint8_t>(LESSTHAN), LESSTHAN},
        {static_cast<std::uint8_t>(GREATERTHAN), GREATERTHAN},
        {static_cast<std::uint8_t>(LESSTHANOREQUAL), LESSTHANOREQUAL},
        {static_cast<std::uint8_t>(GREATERTHANOREQUAL), GREATERTHANOREQUAL},
        {static_cast<std::uint8_t>(MIN), MIN},
        {static_cast<std::uint8_t>(MAX), MAX},
        {static_cast<std::uint8_t>(WITHIN), WITHIN},
        {static_cast<std::uint8_t>(RIPEMD160), RIPEMD160},
        {static_cast<std::uint8_t>(SHA1), SHA1},
        {static_cast<std::uint8_t>(SHA256), SHA256},
        {static_cast<std::uint8_t>(HASH160), HASH160},
        {static_cast<std::uint8_t>(HASH256), HASH256},
        {static_cast<std::uint8_t>(CODESEPARATOR), CODESEPARATOR},
        {static_cast<std::uint8_t>(CHECKSIG), CHECKSIG},
        {static_cast<std::uint8_t>(CHECKSIGVERIFY), CHECKSIGVERIFY},
        {static_cast<std::uint8_t>(CHECKMULTISIG), CHECKMULTISIG},
        {static_cast<std::uint8_t>(CHECKMULTISIGVERIFY), CHECKMULTISIGVERIFY},
        {static_cast<std::uint8_t>(NOP1), NOP1},
        {static_cast<std::uint8_t>(NOP2), NOP2},
        {static_cast<std::uint8_t>(NOP3), NOP3},
        {static_cast<std::uint8_t>(NOP4), NOP4},
        {static_cast<std::uint8_t>(NOP5), NOP5},
        {static_cast<std::uint8_t>(NOP6), NOP6},
        {static_cast<std::uint8_t>(NOP7), NOP7},
        {static_cast<std::uint8_t>(NOP8), NOP8},
        {static_cast<std::uint8_t>(NOP9), NOP9},
        {static_cast<std::uint8_t>(NOP10), NOP10},
        {static_cast<std::uint8_t>(PUBKEYHASH), PUBKEYHASH},
        {static_cast<std::uint8_t>(PUBKEY), PUBKEY},
        {static_cast<std::uint8_t>(INVALIDOPCODE), INVALIDOPCODE},
    });

    return map.at(std::to_integer<std::uint8_t>(in));
}

auto Script::evaluate_data(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 <= script.size());

    for (auto i = 1_uz; i < script.size(); ++i) {
        if (false == is_data_push(script.at(i))) { return Pattern::Custom; }
    }

    return Pattern::NullData;
}

auto Script::evaluate_multisig(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(4 <= script.size());

    auto it = script.crbegin();
    std::advance(it, 1);
    const auto n = to_number(it->opcode_);

    if (0u == n) { return Pattern::Malformed; }
    if ((3u + n) > script.size()) { return Pattern::Malformed; }

    for (auto i = std::uint8_t{0}; i < n; ++i) {
        std::advance(it, 1);

        if (false == is_public_key(*it)) { return Pattern::Malformed; }
    }

    std::advance(it, 1);
    const auto m = to_number(it->opcode_);

    if ((0u == m) || (m > n)) { return Pattern::Malformed; }

    if ((3u + n) < script.size()) { return Pattern::Custom; }

    return Pattern::PayToMultisig;
}

auto Script::evaluate_pubkey(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 == script.size());

    if (is_public_key(script.at(0))) { return Pattern::PayToPubkey; }

    return Pattern::Custom;
}

auto Script::evaluate_pubkey_hash(const ScriptElements& script) noexcept
    -> Pattern
{
    OT_ASSERT(5 == script.size());

    auto it = script.cbegin();

    if (OP::DUP != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (OP::HASH160 != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Pattern::Custom; }

    std::advance(it, 1);

    if (OP::EQUALVERIFY != it->opcode_) { return Pattern::Custom; }

    return Pattern::PayToPubkeyHash;
}

auto Script::evaluate_script_hash(const ScriptElements& script) noexcept
    -> Pattern
{
    OT_ASSERT(3 == script.size());

    auto it = script.cbegin();

    if (OP::HASH160 != it->opcode_) { return Pattern::Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Pattern::Custom; }
    if (OP::PUSHDATA_20 != it->opcode_) { return Pattern::Custom; }

    return Pattern::PayToScriptHash;
}

auto Script::evaluate_segwit(const ScriptElements& script) noexcept -> Pattern
{
    OT_ASSERT(2 == script.size());

    const auto& opcode = script.at(0).opcode_;
    const auto& program = script.at(1).data_.value();

    switch (to_number(opcode)) {
        case 0: {
            switch (program.size()) {
                case 20: {
                    return Pattern::PayToWitnessPubkeyHash;
                }
                case 32: {
                    return Pattern::PayToWitnessScriptHash;
                }
                default: {
                }
            }
        } break;
        case 1: {
            switch (program.size()) {
                case 32: {
                    return Pattern::PayToTaproot;
                }
                default: {
                }
            }
        } break;
        default: {
        }
    }

    return Pattern::Custom;
}

auto Script::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    if (elements_.empty()) {
        LogTrace()(OT_PRETTY_CLASS())("skipping empty script").Flush();

        return;
    }

    switch (style) {
        case cfilter::Type::ES: {
            LogTrace()(OT_PRETTY_CLASS())("processing data pushes").Flush();

            for (const auto& element : *this) {
                if (is_data_push(element)) {
                    const auto& data = element.data_.value();
                    const auto* it{data.data()};

                    switch (data.size()) {
                        case 65: {
                            std::advance(it, 1);
                            [[fallthrough]];
                        }
                        case 64: {
                            out.emplace_back(it, it + 32);
                            std::advance(it, 32);
                            out.emplace_back(it, it + 32);
                            [[fallthrough]];
                        }
                        case 33:
                        case 32:
                        case 20: {
                            out.emplace_back(data.cbegin(), data.cend());
                        } break;
                        default: {
                        }
                    }
                }
            }

            if (const auto subscript = RedeemScript(); subscript) {
                subscript->Internal().ExtractElements(style, out);
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant:
        case cfilter::Type::Unknown:
        default: {
            if (OP::RETURN == elements_.at(0).opcode_) {
                LogTrace()(OT_PRETTY_CLASS())("skipping null data script")
                    .Flush();

                return;
            }

            LogTrace()(OT_PRETTY_CLASS())("processing serialized script")
                .Flush();
            auto& script = out.emplace_back();
            Serialize(writer(script));
        }
    }
}

auto Script::first_opcode(const ScriptElements& script) noexcept -> OP
{
    return script.cbegin()->opcode_;
}

auto Script::get_data(const std::size_t position) const noexcept(false)
    -> ReadView
{
    const auto& data = elements_.at(position).data_;

    if (false == data.has_value()) {
        throw std::out_of_range("No data at specified script position");
    }

    return reader(data.value());
}

auto Script::get_opcode(const std::size_t position) const noexcept(false) -> OP
{
    return elements_.at(position).opcode_;
}

auto Script::get_type(
    const Position role,
    const ScriptElements& script) noexcept -> Pattern
{
    if (0 == script.size()) { return Pattern::Empty; }

    switch (role) {
        case Position::Coinbase: {

            return Pattern::Coinbase;
        }
        case Position::Input: {

            return Pattern::Input;
        }
        case Position::Redeem:
        case Position::Output: {
            if (potential_pubkey_hash(script)) {
                return evaluate_pubkey_hash(script);
            } else if (potential_script_hash(script)) {
                return evaluate_script_hash(script);
            } else if (potential_data(script)) {
                return evaluate_data(script);
            } else if (potential_segwit(script)) {
                return evaluate_segwit(script);
            } else if (potential_pubkey(script)) {
                return evaluate_pubkey(script);
            } else if (potential_multisig(script)) {
                return evaluate_multisig(script);
            } else {
                return Pattern::Custom;
            }
        }
        default: {
            OT_FAIL;
        }
    }
}

auto Script::IndexElements(const api::Session& api, ElementHashes& out)
    const noexcept -> void
{
    const auto hashes = LikelyPubkeyHashes(api.Crypto());
    std::transform(
        std::begin(hashes),
        std::end(hashes),
        std::inserter(out, out.end()),
        [&](const auto& hash) -> auto{
            return api.Crypto().Blockchain().Internal().IndexItem(hash.Bytes());
        });
}

auto Script::IsNotification(
    const std::uint8_t version,
    const PaymentCode& recipient) const noexcept -> bool
{
    if (Pattern::PayToMultisig != Type()) { return false; }

    if (M().value_or(0) != 1u) { return false; }
    if (N().value_or(0) != 3u) { return false; }

    const auto key = MultisigPubkey(1);

    if (false == key.has_value()) { return false; }

    const auto bytes = key.value();

    if ((33u != bytes.size()) && (65u != bytes.size())) { return false; }

    const auto expect = [&] {
        auto out = Space{};
        recipient.Locator(writer(out), version);

        return out;
    }();

    if (32u != expect.size()) { return false; }

    return 0 == std::memcmp(expect.data(), std::next(bytes.data()), 32);
}

auto Script::is_data_push(const value_type& element) noexcept -> bool
{
    return validate(element, true);
}

auto Script::is_direct_push(const OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    const auto value = static_cast<std::uint8_t>(opcode);

    if ((0 < value) && (76 > value)) { return value; }

    return std::nullopt;
}

auto Script::is_push(const OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    constexpr auto low = 75_uz;
    constexpr auto high = 79_uz;
    constexpr auto shift = low + 1_uz;
    constexpr auto one = 1_uz;
    const auto value = std::size_t{static_cast<std::uint8_t>(opcode)};

    if ((low < value) && (high > value)) { return one << (value - shift); }

    return std::nullopt;
}

auto Script::is_hash160(const value_type& element) noexcept -> bool
{
    if (false == is_data_push(element)) { return false; }

    return 20 == element.data_->size();
}

auto Script::is_public_key(const value_type& element) noexcept -> bool
{
    if (false == is_data_push(element)) { return false; }

    const auto size = element.data_->size();

    return (33 == size) || (65 == size);
}

auto Script::last_opcode(const ScriptElements& script) noexcept -> OP
{
    return script.crbegin()->opcode_;
}

auto Script::LikelyPubkeyHashes(const api::Crypto& crypto) const noexcept
    -> UnallocatedVector<ByteArray>
{
    auto output = UnallocatedVector<ByteArray>{};

    switch (type_) {
        case Pattern::PayToPubkeyHash: {
            const auto hash = PubkeyHash();

            OT_ASSERT(hash.has_value());

            output.emplace_back(hash.value());
        } break;
        case Pattern::PayToMultisig: {
            for (auto i = std::uint8_t{0}; i < N().value(); ++i) {
                auto hash = ByteArray{};
                const auto key = MultisigPubkey(i);

                OT_ASSERT(key.has_value());

                blockchain::PubkeyHash(
                    crypto, chain_, key.value(), hash.WriteInto());
                output.emplace_back(std::move(hash));
            }
        } break;
        case Pattern::PayToPubkey: {
            auto hash = ByteArray{};
            const auto key = Pubkey();

            OT_ASSERT(key.has_value());

            blockchain::PubkeyHash(
                crypto, chain_, key.value(), hash.WriteInto());
            output.emplace_back(std::move(hash));
        } break;
        case Pattern::Coinbase:
        case Pattern::PayToScriptHash:
        case Pattern::Empty:
        case Pattern::Malformed: {
        } break;
        case Pattern::Custom:
        case Pattern::NullData:
        case Pattern::Input:
        case Pattern::PayToWitnessPubkeyHash:  // TODO
        case Pattern::PayToWitnessScriptHash:
        case Pattern::PayToTaproot:
        case Pattern::None:
        default: {
            for (const auto& element : elements_) {
                if (is_hash160(element)) {
                    OT_ASSERT(element.data_.has_value());

                    output.emplace_back(reader(element.data_.value()));
                } else if (is_public_key(element)) {
                    OT_ASSERT(element.data_.has_value());

                    auto hash = ByteArray{};
                    blockchain::PubkeyHash(
                        crypto,
                        chain_,
                        reader(element.data_.value()),
                        hash.WriteInto());
                    output.emplace_back(std::move(hash));
                }
            }
        }
    }

    return output;
}

auto Script::M() const noexcept -> std::optional<std::uint8_t>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(0));
}

auto Script::MultisigPubkey(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    const auto index = position + 1_uz;

    if (index > N()) { return {}; }

    return get_data(index);
}

auto Script::N() const noexcept -> std::optional<std::uint8_t>
{
    if (Pattern::PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(elements_.size() - 2));
}

auto Script::potential_data(const ScriptElements& script) noexcept -> bool
{
    return (OP::RETURN == first_opcode(script)) && (2 <= script.size());
}

auto Script::potential_multisig(const ScriptElements& script) noexcept -> bool
{
    return (OP::CHECKMULTISIG == last_opcode(script)) && (4 <= script.size());
}

auto Script::potential_pubkey(const ScriptElements& script) noexcept -> bool
{
    return (OP::CHECKSIG == last_opcode(script)) && (2 == script.size());
}

auto Script::potential_pubkey_hash(const ScriptElements& script) noexcept
    -> bool
{
    return (OP::CHECKSIG == last_opcode(script)) && (5 == script.size());
}

auto Script::potential_script_hash(const ScriptElements& script) noexcept
    -> bool
{
    return (OP::HASH160 == first_opcode(script)) &&
           (OP::EQUAL == last_opcode(script)) && (3 == script.size());
}

auto Script::potential_segwit(const ScriptElements& script) noexcept -> bool
{
    using enum OP;

    if (2 != script.size()) { return false; }

    switch (first_opcode(script)) {
        case ZERO:
        case ONE:
        case TWO:
        case THREE:
        case FOUR:
        case FIVE:
        case SIX:
        case SEVEN:
        case EIGHT:
        case NINE:
        case TEN:
        case ELEVEN:
        case TWELVE:
        case THIRTEEN:
        case FOURTEEN:
        case FIFTEEN:
        case SIXTEEN: {
        } break;
        case PUSHDATA_1:
        case PUSHDATA_2:
        case PUSHDATA_3:
        case PUSHDATA_4:
        case PUSHDATA_5:
        case PUSHDATA_6:
        case PUSHDATA_7:
        case PUSHDATA_8:
        case PUSHDATA_9:
        case PUSHDATA_10:
        case PUSHDATA_11:
        case PUSHDATA_12:
        case PUSHDATA_13:
        case PUSHDATA_14:
        case PUSHDATA_15:
        case PUSHDATA_16:
        case PUSHDATA_17:
        case PUSHDATA_18:
        case PUSHDATA_19:
        case PUSHDATA_20:
        case PUSHDATA_21:
        case PUSHDATA_22:
        case PUSHDATA_23:
        case PUSHDATA_24:
        case PUSHDATA_25:
        case PUSHDATA_26:
        case PUSHDATA_27:
        case PUSHDATA_28:
        case PUSHDATA_29:
        case PUSHDATA_30:
        case PUSHDATA_31:
        case PUSHDATA_32:
        case PUSHDATA_33:
        case PUSHDATA_34:
        case PUSHDATA_35:
        case PUSHDATA_36:
        case PUSHDATA_37:
        case PUSHDATA_38:
        case PUSHDATA_39:
        case PUSHDATA_40:
        case PUSHDATA_41:
        case PUSHDATA_42:
        case PUSHDATA_43:
        case PUSHDATA_44:
        case PUSHDATA_45:
        case PUSHDATA_46:
        case PUSHDATA_47:
        case PUSHDATA_48:
        case PUSHDATA_49:
        case PUSHDATA_50:
        case PUSHDATA_51:
        case PUSHDATA_52:
        case PUSHDATA_53:
        case PUSHDATA_54:
        case PUSHDATA_55:
        case PUSHDATA_56:
        case PUSHDATA_57:
        case PUSHDATA_58:
        case PUSHDATA_59:
        case PUSHDATA_60:
        case PUSHDATA_61:
        case PUSHDATA_62:
        case PUSHDATA_63:
        case PUSHDATA_64:
        case PUSHDATA_65:
        case PUSHDATA_66:
        case PUSHDATA_67:
        case PUSHDATA_68:
        case PUSHDATA_69:
        case PUSHDATA_70:
        case PUSHDATA_71:
        case PUSHDATA_72:
        case PUSHDATA_73:
        case PUSHDATA_74:
        case PUSHDATA_75:
        case PUSHDATA1:
        case PUSHDATA2:
        case PUSHDATA4:
        case ONE_NEGATE:
        case RESERVED:
        case NOP:
        case VER:
        case IF:
        case NOTIF:
        case VERIF:
        case VERNOTIF:
        case ELSE:
        case ENDIF:
        case VERIFY:
        case RETURN:
        case TOALTSTACK:
        case FROMALTSTACK:
        case TWO_DROP:
        case TWO_DUP:
        case THREE_DUP:
        case TWO_OVER:
        case TWO_ROT:
        case TWO_SWAP:
        case IFDUP:
        case DEPTH:
        case DROP:
        case DUP:
        case NIP:
        case OVER:
        case PICK:
        case ROLL:
        case ROT:
        case SWAP:
        case TUCK:
        case CAT:
        case SUBSTR:
        case LEFT:
        case RIGHT:
        case SIZE:
        case INVERT:
        case AND:
        case OR:
        case XOR:
        case EQUAL:
        case EQUALVERIFY:
        case RESERVED1:
        case RESERVED2:
        case ONE_ADD:
        case ONE_SUB:
        case TWO_MUL:
        case TWO_DIV:
        case NEGATE:
        case ABS:
        case NOT:
        case ZERO_NOTEQUAL:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case MOD:
        case LSHIFT:
        case RSHIFT:
        case BOOLAND:
        case BOOLOR:
        case NUMEQUAL:
        case NUMEQUALVERIFY:
        case NUMNOTEQUAL:
        case LESSTHAN:
        case GREATERTHAN:
        case LESSTHANOREQUAL:
        case GREATERTHANOREQUAL:
        case MIN:
        case MAX:
        case WITHIN:
        case RIPEMD160:
        case SHA1:
        case SHA256:
        case HASH160:
        case HASH256:
        case CODESEPARATOR:
        case CHECKSIG:
        case CHECKSIGVERIFY:
        case CHECKMULTISIG:
        case CHECKMULTISIGVERIFY:
        case NOP1:
        case NOP2:
        case NOP3:
        case NOP4:
        case NOP5:
        case NOP6:
        case NOP7:
        case NOP8:
        case NOP9:
        case NOP10:
        case PUBKEYHASH:
        case PUBKEY:
        case INVALIDOPCODE:
        default: {

            return false;
        }
    }

    const auto& element = script.at(1);

    if (false == element.data_.has_value()) { return false; }

    const auto& program = element.data_.value();

    return (1u < program.size()) && (41u > program.size());
}

auto Script::Print() const noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};

    for (const auto& [opcode, invalid, push, data] : elements_) {
        output << "      op: "
               << std::to_string(static_cast<std::uint8_t>(opcode));

        if (invalid) {
            output << " invalid: "
                   << std::to_string(
                          std::to_integer<std::uint8_t>(invalid.value()));
        }

        if (push) {
            auto bytes = std::uint64_t{};
            std::memcpy(
                &bytes,
                push.value().data(),
                std::min(push.value().size(), sizeof(bytes)));
            output << " push bytes: " << bytes;
        }

        if (data) {
            auto item = ByteArray{};
            item.Assign(reader(data.value()));
            output << " (" << item.size() << ") bytes : " << item.asHex();
        }

        output << '\n';
    }

    return output.str();
}

auto Script::Pubkey() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToPubkey: {
            return get_data(0);
        }
        case Pattern::PayToTaproot: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::PubkeyHash() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToPubkeyHash: {
            return get_data(2);
        }
        case Pattern::PayToWitnessPubkeyHash: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::RedeemScript() const noexcept -> std::unique_ptr<block::Script>
{
    if (Position::Input != role_) { return {}; }
    if (0 == elements_.size()) { return {}; }

    const auto& element = *elements_.crbegin();

    if (false == is_data_push(element)) { return {}; }

    return factory::BitcoinScript(
        chain_, reader(element.data_.value()), Position::Redeem, true, true);
}

auto Script::ScriptHash() const noexcept -> std::optional<ReadView>
{
    switch (type_) {
        case Pattern::PayToScriptHash:
        case Pattern::PayToWitnessScriptHash: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::Serialize(Writer&& destination) const noexcept -> bool
{
    try {
        const auto size = CalculateSize();

        if (0_uz == size) { return true; }

        auto buf = reserve(std::move(destination), size, "script");

        for (const auto& element : elements_) {
            const auto& [opcode, invalid, bytes, data] = element;

            if (invalid.has_value()) {
                const auto& code = invalid.value();
                serialize_object(code, buf, "opcode");
            } else {
                serialize_object(opcode, buf, "opcode");
            }

            if (bytes.has_value()) { copy(reader(*bytes), buf, "argument"); }

            if (data.has_value()) { copy(reader(*data), buf, "data"); }
        }

        check_finished(buf);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Script::SigningSubscript(const blockchain::Type chain) const noexcept
    -> std::unique_ptr<internal::Script>
{
    switch (type_) {
        case Pattern::PayToWitnessPubkeyHash: {
            auto elements = [&] {
                auto out = ScriptElements{};
                out.emplace_back(internal::Opcode(OP::DUP));
                out.emplace_back(internal::Opcode(OP::HASH160));
                out.emplace_back(internal::PushData(PubkeyHash().value()));
                out.emplace_back(internal::Opcode(OP::EQUALVERIFY));
                out.emplace_back(internal::Opcode(OP::CHECKSIG));

                return out;
            }();

            return std::make_unique<Script>(
                chain_, Position::Output, std::move(elements));
        }
        default: {
            // TODO handle OP_CODESEPERATOR shit

            return clone();
        }
    }
}

auto Script::to_number(const OP opcode) noexcept -> std::uint8_t
{
    if ((OP::ONE <= opcode) && (OP::SIXTEEN >= opcode)) {

        return static_cast<std::uint8_t>(opcode) -
               static_cast<std::uint8_t>(OP::ONE) + 1u;
    }

    return 0;
}

auto Script::validate(const ScriptElements& elements) noexcept -> bool
{
    for (const auto& element : elements) {
        if (false == validate(element, false)) { return false; }
    }

    return true;
}

auto Script::validate(
    const ScriptElement& element,
    const bool checkForData) noexcept -> bool
{
    const auto& [opcode, invalid, bytes, data] = element;

    if (invalid.has_value()) { return !checkForData; }

    if (auto direct = is_direct_push(opcode); direct.has_value()) {
        if (bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }
        if (direct != data.value().size()) { return false; }

        return true;
    } else if (auto push = is_push(opcode); push.has_value()) {
        if (false == bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }

        auto size = 0_uz;
        const auto& pushBytes = push.value();

        if (pushBytes != bytes->size()) { return false; }

        switch (pushBytes) {
            case 1:
            case 2:
            case 4: {
                auto buf = be::little_uint32_buf_t{};
                std::memcpy(
                    static_cast<void*>(&buf), bytes.value().data(), pushBytes);
                size = buf.value();
            } break;
            default: {
                OT_FAIL;
            }
        }

        if (size != data.value().size()) { return false; }

        return true;
    } else {

        return !checkForData;
    }
}

auto Script::Value(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    if (Pattern::NullData != type_) { return {}; }

    const auto index = position + 1u;

    if (index > elements_.size()) { return {}; }

    return get_data(index);
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
