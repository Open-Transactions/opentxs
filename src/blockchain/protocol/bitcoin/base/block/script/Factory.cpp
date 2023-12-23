// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/script/Imp.hpp"
#include "blockchain/protocol/bitcoin/base/block/script/ScriptPrivate.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Element.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/OP.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Position.hpp"  // IWYU pragma: keep
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
    const blockchain::protocol::bitcoin::base::block::script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using Data =
        blockchain::protocol::bitcoin::base::block::script::Element::Data;
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Script;
    using BlankType = blockchain::protocol::bitcoin::base::block::ScriptPrivate;

    try {
        if ((false == valid(bytes) || (Coinbase == role))) {

            return pmr::construct<ReturnType>(
                alloc.result_,
                chain,
                role,
                Vector<blockchain::protocol::bitcoin::base::block::script::
                           Element>{alloc.result_},
                0);
        }

        auto elements =
            blockchain::protocol::bitcoin::base::block::ScriptElements{
                alloc.result_};
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
                using enum blockchain::protocol::bitcoin::base::block::script::
                    OP;

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

                    assert_true(0 < sizeBytes);
                    assert_true(5 > sizeBytes);

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

        return pmr::construct<ReturnType>(
            alloc.result_, chain, role, std::move(elements), bytes.size());
    } catch (const std::exception& e) {
        const auto& logger = mute ? LogTrace() : LogVerbose();
        logger()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinScript(
    const blockchain::Type chain,
    Vector<blockchain::protocol::bitcoin::base::block::script::Element>
        elements,
    const blockchain::protocol::bitcoin::base::block::script::Position role,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Script;
    using BlankType = blockchain::protocol::bitcoin::base::block::ScriptPrivate;

    try {
        if (false == ReturnType::validate(elements)) {

            throw std::runtime_error{"invalid elements"};
        }

        if (elements.empty() && (Output == role)) {

            throw std::runtime_error{"empty input"};
        }

        return pmr::construct<ReturnType>(
            alloc.result_, chain, role, std::move(elements), std::nullopt);
    } catch (const std::exception& e) {
        LogVerbose()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

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
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

    if ((0u == M) || (16u < M)) {
        LogError()()("Invalid M").Flush();

        return {};
    }

    if ((0u == N) || (16u < N)) {
        LogError()()("Invalid N").Flush();

        return {};
    }

    auto elements = bb::ScriptElements{alloc.result_};
    elements.reserve(3_uz + keys.size());
    elements.clear();
    elements.emplace_back(
        bb::internal::Opcode(static_cast<bb::script::OP>(M + 80)));

    for (const auto& pKey : keys) {
        if (nullptr == pKey) {
            LogError()()("Invalid key").Flush();

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
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

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
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()()("Failed to calculate pubkey hash").Flush();

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
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()()("Failed to serialize script").Flush();

        return {};
    }

    if (false == b::ScriptHash(crypto, chain, reader(bytes), writer(hash))) {
        LogError()()("Failed to calculate script hash").Flush();

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
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

    auto hash = Space{};

    if (false == b::PubkeyHash(crypto, chain, key.PublicKey(), writer(hash))) {
        LogError()()("Failed to calculate pubkey hash").Flush();

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
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    namespace b = opentxs::blockchain;
    namespace bb = blockchain::protocol::bitcoin::base::block;
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using enum blockchain::protocol::bitcoin::base::block::script::OP;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()()("Failed to serialize script").Flush();

        return {};
    }

    if (false ==
        b::ScriptHashSegwit(crypto, chain, reader(bytes), writer(hash))) {
        LogError()()("Failed to calculate script hash").Flush();

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
