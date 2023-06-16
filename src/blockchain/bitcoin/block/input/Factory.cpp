// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include <BlockchainInputWitness.pb.h>
#include <BlockchainPreviousOutput.pb.h>
#include <BlockchainTransactionInput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "blockchain/bitcoin/block/input/Imp.hpp"
#include "blockchain/bitcoin/block/input/InputPrivate.hpp"
#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const blockchain::node::UTXO& spends,
    const std::optional<std::uint32_t> sequence,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    using enum blockchain::bitcoin::block::script::Position;
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using BlankType = blockchain::bitcoin::block::InputPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        namespace b = opentxs::blockchain;
        namespace bb = b::bitcoin::block;
        namespace bi = bb::internal;

        const auto& prevOut = spends.second;

        if (false == prevOut.IsValid()) {

            throw std::runtime_error{"invalid previous output"};
        }

        const auto outputKeys = prevOut.Keys(alloc);

        if (outputKeys.empty()) {

            throw std::runtime_error{"no keys associated with previous output"};
        }

        // TODO if this is input spends a segwit script then make a dummy
        // witness
        auto elements = bb::ScriptElements{alloc};
        auto witness = Vector<bb::WitnessItem>{alloc};
        elements.clear();
        witness.clear();
        using Pattern = bb::script::Pattern;

        switch (prevOut.Script().Type()) {
            case Pattern::PayToWitnessPubkeyHash: {
                witness.emplace_back(
                    reader(bi::Script::blank_signature(chain)));
                witness.emplace_back(reader(bi::Script::blank_pubkey(chain)));
            } break;
            case Pattern::PayToPubkeyHash: {
                elements.emplace_back(bb::internal::PushData(
                    reader(bi::Script::blank_signature(chain))));
                elements.emplace_back(bb::internal::PushData(
                    reader(bi::Script::blank_pubkey(chain))));
            } break;
            case Pattern::PayToPubkey: {
                elements.emplace_back(bb::internal::PushData(
                    reader(bi::Script::blank_signature(chain))));
            } break;
            case Pattern::PayToMultisig: {
                // TODO this is probably wrong. Come up with a better algorithm
                // once multisig is supported
                const auto n = outputKeys.size();

                for (auto i = std::uint8_t{0}; i < n; ++i) {
                    elements.emplace_back(bb::internal::PushData(
                        reader(bi::Script::blank_signature(chain))));
                }
            } break;
            default: {

                throw std::runtime_error{"unhandled script type"};
            }
        }

        auto keys = Set<blockchain::crypto::Key>{alloc};
        keys.clear();
        std::for_each(
            std::begin(outputKeys), std::end(outputKeys), [&](const auto& key) {
                keys.emplace(key);
            });
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            chain,
            sequence.value_or(0xffffffff),
            blockchain::block::Outpoint{spends.first},
            witness,
            BitcoinScript(chain, elements, Input, {}),
            ReturnType::default_version_,
            prevOut,
            std::move(keys));

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const ReadView outpoint,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase,
    Vector<blockchain::bitcoin::block::WitnessItem> witness,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    using enum blockchain::bitcoin::block::script::Position;
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using BlankType = blockchain::bitcoin::block::InputPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        auto buf = boost::endian::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {

            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());
        out = pmr.allocate(1_uz);

        if (coinbase) {
            pmr.construct(
                out,
                chain,
                buf.value(),
                blockchain::block::Outpoint{outpoint},
                std::move(witness),
                script,
                ReturnType::default_version_,
                blockchain::bitcoin::block::OutputPrivate::Blank(alloc),
                outpoint.size() + cs.Total() + sequence.size());
        } else {
            pmr.construct(
                out,
                chain,
                buf.value(),
                blockchain::block::Outpoint{outpoint},
                std::move(witness),
                factory::BitcoinScript(
                    chain, script, Input, true, false, alloc),
                ReturnType::default_version_,
                outpoint.size() + cs.Total() + sequence.size());
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto BitcoinTransactionInput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput& in,
    const bool coinbase,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    using enum blockchain::bitcoin::block::script::Position;
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using BlankType = blockchain::bitcoin::block::InputPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        const auto& outpoint = in.previous();
        auto witness = Vector<blockchain::bitcoin::block::WitnessItem>{alloc};
        witness.reserve(in.witness().item().size());
        witness.clear();

        for (const auto& bytes : in.witness().item()) {
            witness.emplace_back(bytes);
        }

        auto spends = blockchain::bitcoin::block::Output{alloc};

        if (in.has_spends()) {
            spends = factory::BitcoinTransactionOutput(
                crypto, factory, chain, in.spends(), alloc);
        }

        if (coinbase) {
            out = pmr.allocate(1_uz);
            pmr.construct(
                out,
                chain,
                in.sequence(),
                blockchain::block::Outpoint{
                    ReadView{outpoint.txid()},
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                in.script(),
                in.version(),
                std::move(spends),
                std::nullopt);
        } else {
            auto keys = Set<blockchain::crypto::Key>{alloc};
            auto pkh = ReturnType::PubkeyHashes{alloc};
            keys.clear();
            pkh.clear();

            for (const auto& key : in.key()) {
                keys.emplace(
                    factory.AccountIDFromBase58(key.subaccount()),
                    static_cast<blockchain::crypto::Subchain>(
                        static_cast<std::uint8_t>(key.subchain())),
                    key.index());
            }

            for (const auto& pattern : in.pubkey_hash()) {
                pkh.emplace(pattern);
            }

            out = pmr.allocate(1_uz);
            pmr.construct(
                out,
                chain,
                in.sequence(),
                blockchain::block::Outpoint{
                    ReadView{outpoint.txid()},
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                factory::BitcoinScript(
                    chain,
                    in.script(),
                    coinbase ? Coinbase : Input,
                    true,
                    false,
                    alloc),
                ByteArray{alloc},
                in.version(),
                std::nullopt,
                std::move(keys),
                std::move(spends));
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory
