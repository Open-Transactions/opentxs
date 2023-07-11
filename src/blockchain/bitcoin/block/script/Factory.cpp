// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/script/Imp.hpp"
#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
namespace be = boost::endian;

auto BitcoinScript(
    const blockchain::Type chain,
    ReadView bytes,
    const blockchain::bitcoin::block::script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    using enum blockchain::bitcoin::block::script::Position;
    using Data = blockchain::bitcoin::block::script::Element::Data;
    using ReturnType = blockchain::bitcoin::block::implementation::Script;
    using BlankType = blockchain::bitcoin::block::ScriptPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if ((false == valid(bytes) || (Coinbase == role))) {
            pmr.construct(
                out,
                chain,
                role,
                Vector<blockchain::bitcoin::block::script::Element>{pmr},
                0);

            return out;
        }

        auto elements = blockchain::bitcoin::block::ScriptElements{pmr};
        elements.clear();
        elements.reserve(bytes.size());
        const auto* it = reinterpret_cast<const std::byte*>(bytes.data());
        auto read = 0_uz;
        const auto target = bytes.size();

        while (read < target) {
            auto& element = elements.emplace_back();
            auto& [opcode, invalid, size, data] = element;

            try {
                opcode = ReturnType::decode(*it);
            } catch (...) {
                using enum blockchain::bitcoin::block::script::OP;

                if (allowInvalidOpcodes) {
                    opcode = INVALIDOPCODE;
                    invalid = std::to_integer<std::uint8_t>(*it);
                } else {
                    throw std::runtime_error{"unknown opcode"};
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

                    throw std::runtime_error{"incomplete direct data push"};
                }

                data = Data{{reinterpret_cast<const char*>(it), effectiveSize}};
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

                        throw std::runtime_error{"incomplete data push"};
                    }

                    if (0u < effectiveSize) {
                        size = Data{
                            {reinterpret_cast<const char*>(it), effectiveSize}};
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

                        throw std::runtime_error{"data push bytes missing"};
                    }

                    data = Data{
                        {reinterpret_cast<const char*>(it), effectiveSize}};
                    read += effectiveSize;
                    std::advance(it, effectiveSize);
                }

                continue;
            }
        }

        elements.shrink_to_fit();
        pmr.construct(out, chain, role, std::move(elements), bytes.size());

        return out;
    } catch (const std::exception& e) {
        const auto& logger = mute ? LogTrace() : LogVerbose();
        logger("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto BitcoinScript(
    const blockchain::Type chain,
    Vector<blockchain::bitcoin::block::script::Element> elements,
    const blockchain::bitcoin::block::script::Position role,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    using enum blockchain::bitcoin::block::script::Position;
    using ReturnType = blockchain::bitcoin::block::implementation::Script;
    using BlankType = blockchain::bitcoin::block::ScriptPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        if (false == ReturnType::validate(elements)) {

            throw std::runtime_error{"invalid elements"};
        }

        if (elements.empty() && (Output == role)) {

            throw std::runtime_error{"empty input"};
        }

        out = pmr.allocate(1_uz);
        pmr.construct(out, chain, role, std::move(elements), std::nullopt);

        return out;
    } catch (const std::exception& e) {
        LogVerbose()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(1_uz + data.size());
    elements.clear();
    elements.emplace_back(bb::internal::Opcode(RETURN));

    for (const auto& element : data) {
        elements.emplace_back(bb::internal::PushData(element));
    }

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

    if ((0u == M) || (16u < M)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid M").Flush();

        return {};
    }

    if ((0u == N) || (16u < N)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid N").Flush();

        return {};
    }

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(3_uz + keys.size());
    elements.clear();
    elements.emplace_back(
        bb::internal::Opcode(static_cast<bb::script::OP>(M + 80)));

    for (const auto& pKey : keys) {
        if (nullptr == pKey) {
            LogError()("opentxs::factory::")(__func__)(": Invalid key").Flush();

            return {};
        }

        const auto& key = *pKey;
        elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    }

    elements.emplace_back(
        bb::internal::Opcode(static_cast<bb::script::OP>(N + 80)));
    elements.emplace_back(bb::internal::Opcode(CHECKMULTISIG));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2PK(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(2_uz);
    elements.clear();
    elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    elements.emplace_back(bb::internal::Opcode(CHECKSIG));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(5_uz);
    elements.clear();
    elements.emplace_back(bb::internal::Opcode(DUP));
    elements.emplace_back(bb::internal::Opcode(HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(EQUALVERIFY));
    elements.emplace_back(bb::internal::Opcode(CHECKSIG));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

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

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(3_uz);
    elements.clear();
    elements.emplace_back(bb::internal::Opcode(HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(EQUAL));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(2_uz);
    elements.clear();
    elements.emplace_back(bb::internal::Opcode(ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}

auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::bitcoin::block;
    using enum blockchain::bitcoin::block::script::Position;
    using enum blockchain::bitcoin::block::script::OP;

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

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(2_uz);
    elements.clear();
    elements.emplace_back(bb::internal::Opcode(ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));

    return factory::BitcoinScript(chain, elements, Output, alloc);
}
}  // namespace opentxs::factory
