// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::OP
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain

#include "blockchain/bitcoin/block/Input.hpp"  // IWYU pragma: associated

#include <BlockchainInputWitness.pb.h>
#include <BlockchainPreviousOutput.pb.h>
#include <BlockchainTransactionInput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const UTXO& spends,
    const std::optional<std::uint32_t> sequence) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using Position = blockchain::bitcoin::block::Script::Position;
    namespace b = opentxs::blockchain;
    namespace bb = b::bitcoin::block;
    namespace bi = bb::internal;

    const auto& pPrevOut = spends.second;

    if (false == bool(pPrevOut)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid previous output")
            .Flush();

        return {};
    }

    const auto& prevOut = *pPrevOut;
    const auto& outputKeys = prevOut.Keys();

    if (0 == outputKeys.size()) {
        LogError()("opentxs::factory::")(__func__)(
            ": No keys associated with previous output")
            .Flush();

        return {};
    }

    // TODO if this is input spends a segwit script then make a dummy witness
    auto elements = bb::ScriptElements{};
    auto witness = UnallocatedVector<Space>{};

    switch (prevOut.Script().Type()) {
        case bb::Script::Pattern::PayToWitnessPubkeyHash: {
            witness.emplace_back(bi::Script::blank_signature(chain));
            witness.emplace_back(bi::Script::blank_pubkey(chain));
        } break;
        case bb::Script::Pattern::PayToPubkeyHash: {
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_signature(chain))));
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_pubkey(chain))));
        } break;
        case bb::Script::Pattern::PayToPubkey: {
            elements.emplace_back(bb::internal::PushData(
                reader(bi::Script::blank_signature(chain))));
        } break;
        case bb::Script::Pattern::PayToMultisig: {
            // TODO this is probably wrong. Come up with a better algorithm once
            // multisig is supported
            const auto n = outputKeys.size();

            for (auto i = std::uint8_t{0}; i < n; ++i) {
                elements.emplace_back(bb::internal::PushData(
                    reader(bi::Script::blank_signature(chain))));
            }
        } break;
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": Unhandled script type")
                .Flush();

            return {};
        }
    }

    auto keys = Set<blockchain::crypto::Key>{};
    std::for_each(
        std::begin(outputKeys), std::end(outputKeys), [&](const auto& key) {
            keys.emplace(key);
        });

    return std::make_unique<ReturnType>(
        chain,
        sequence.value_or(0xffffffff),
        blockchain::block::Outpoint{spends.first},
        std::move(witness),
        BitcoinScript(chain, std::move(elements), Position::Input),
        ReturnType::default_version_,
        prevOut.Internal().clone(),
        std::move(keys));
}

auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase,
    UnallocatedVector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using Position = blockchain::bitcoin::block::Script::Position;

    try {
        auto buf = be::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {
            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());

        if (coinbase) {
            return std::make_unique<ReturnType>(
                chain,
                buf.value(),
                blockchain::block::Outpoint{outpoint},
                std::move(witness),
                script,
                ReturnType::default_version_,
                nullptr,
                outpoint.size() + cs.Total() + sequence.size());
        } else {
            return std::make_unique<ReturnType>(
                chain,
                buf.value(),
                blockchain::block::Outpoint{outpoint},
                std::move(witness),
                factory::BitcoinScript(chain, script, Position::Input),
                ReturnType::default_version_,
                outpoint.size() + cs.Total() + sequence.size());
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionInput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput& in,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Input;
    using Position = blockchain::bitcoin::block::Script::Position;
    const auto& outpoint = in.previous();
    auto witness = UnallocatedVector<Space>{};

    for (const auto& bytes : in.witness().item()) {
        const auto* const it = reinterpret_cast<const std::byte*>(bytes.data());
        witness.emplace_back(it, it + bytes.size());
    }

    auto spends =
        std::unique_ptr<blockchain::bitcoin::block::internal::Output>{};

    if (in.has_spends()) {
        spends = factory::BitcoinTransactionOutput(
            crypto, factory, chain, in.spends());
    }

    try {
        if (coinbase) {
            return std::make_unique<ReturnType>(
                chain,
                in.sequence(),
                blockchain::block::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                in.script(),
                in.version(),
                std::move(spends));
        } else {
            auto keys = Set<blockchain::crypto::Key>{};
            auto pkh = ReturnType::PubkeyHashes{};

            for (const auto& key : in.key()) {
                keys.emplace(
                    factory.IdentifierFromBase58(key.subaccount()),
                    static_cast<blockchain::crypto::Subchain>(
                        static_cast<std::uint8_t>(key.subchain())),
                    key.index());
            }

            for (const auto& pattern : in.pubkey_hash()) {
                pkh.emplace(pattern);
            }

            return std::make_unique<ReturnType>(
                chain,
                in.sequence(),
                blockchain::block::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                factory::BitcoinScript(
                    chain,
                    in.script(),
                    coinbase ? Position::Coinbase : Position::Input),
                Space{},
                in.version(),
                std::nullopt,
                std::move(keys),
                // TODO std::move(pkh),
                // (in.has_script_hash()
                //      ? std::make_optional<blockchain::PatternID>(
                //            in.script_hash())
                //      : std::nullopt),
                // in.indexed(),
                std::move(spends));
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Input::default_version_{1};
const VersionNumber Input::outpoint_version_{1};
const VersionNumber Input::key_version_{1};

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    UnallocatedVector<Space>&& witness,
    std::unique_ptr<const block::Script> script,
    Space&& coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size,
    Set<crypto::Key>&& keys,
    std::unique_ptr<internal::Output> output) noexcept(false)
    : chain_(chain)
    , serialize_version_(version)
    , previous_(std::move(previous))
    , witness_(std::move(witness))
    , script_(std::move(script))
    , coinbase_(std::move(coinbase))
    , sequence_(sequence)
    , cache_(std::move(output), std::move(size), std::move(keys))
    , guarded_()
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid input script");
    }

    if ((0 < coinbase_.size()) && (0 < script_->size())) {
        throw std::runtime_error("Input has both script and coinbase");
    }
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    UnallocatedVector<Space>&& witness,
    std::unique_ptr<const block::Script> script,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          size,
          {},
          nullptr)
{
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    UnallocatedVector<Space>&& witness,
    std::unique_ptr<const block::Script> script,
    const VersionNumber version,
    std::unique_ptr<internal::Output> output,
    Set<crypto::Key>&& keys) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          {},
          std::move(keys),
          std::move(output))
{
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    UnallocatedVector<Space>&& witness,
    const ReadView coinbase,
    const VersionNumber version,
    std::unique_ptr<internal::Output> output,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          factory::BitcoinScript(
              chain,
              ScriptElements{},
              Script::Position::Coinbase),
          Space{
              reinterpret_cast<const std::byte*>(coinbase.data()),
              reinterpret_cast<const std::byte*>(coinbase.data()) +
                  coinbase.size()},
          version,
          size,
          {},
          std::move(output))
{
}

Input::Input(const Input& rhs) noexcept
    : Input(rhs, rhs.script_->Internal().clone())
{
}

Input::Input(
    const Input& rhs,
    std::unique_ptr<const block::Script> script) noexcept
    : chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , previous_(rhs.previous_)
    , witness_(rhs.witness_.begin(), rhs.witness_.end())
    , script_(std::move(script))
    , coinbase_(rhs.coinbase_.begin(), rhs.coinbase_.end())
    , sequence_(rhs.sequence_)
    , cache_(rhs.cache_)
    , guarded_()
{
}

auto Input::AddMultisigSignatures(const Signatures& signatures) noexcept -> bool
{
    auto elements = ScriptElements{};
    elements.emplace_back(internal::Opcode(OP::ZERO));

    for (const auto& [sig, key] : signatures) {
        if (false == valid(sig)) { return false; }

        elements.emplace_back(internal::PushData(sig));
    }

    auto& script = const_cast<std::unique_ptr<const block::Script>&>(script_);
    script = factory::BitcoinScript(
        chain_, std::move(elements), Script::Position::Input);
    cache_.reset_size();

    return bool(script);
}

auto Input::AddSignatures(const Signatures& signatures) noexcept -> bool
{
    if (0 == witness_.size()) {
        auto elements = ScriptElements{};

        for (const auto& [sig, key] : signatures) {
            if (false == valid(sig)) { return false; }

            elements.emplace_back(internal::PushData(sig));

            if (valid(key)) { elements.emplace_back(internal::PushData(key)); }
        }

        auto& script =
            const_cast<std::unique_ptr<const block::Script>&>(script_);
        script = factory::BitcoinScript(
            chain_, std::move(elements), Script::Position::Input);
        cache_.reset_size();

        return bool(script);
    } else {
        // TODO this only works for P2WPKH
        auto& witness = const_cast<UnallocatedVector<Space>&>(witness_);
        witness.clear();

        for (const auto& [sig, key] : signatures) {
            if (false == valid(sig)) { return false; }
            if (false == valid(key)) { return false; }

            witness.emplace_back(space(sig));
            witness.emplace_back(space(key));
        }

        cache_.reset_size();

        return true;
    }
}

auto Input::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    UnallocatedVector<identifier::Nym>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = crypto.Owner(key);

        if (false == owner.empty()) { output.emplace_back(owner); }
    });
}

auto Input::AssociatedRemoteContacts(
    const api::session::Client& api,
    UnallocatedVector<identifier::Generic>& output) const noexcept -> void
{
    const auto hashes = script_->Internal().LikelyPubkeyHashes(api.Crypto());
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = api.Crypto().Blockchain().LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });

    auto payer = cache_.payer();

    if (false == payer.empty()) { output.emplace_back(std::move(payer)); }
}

auto Input::AssociatePreviousOutput(const internal::Output& in) noexcept -> bool
{
    return cache_.associate(in);
}

auto Input::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    return cache_.size(normalized, [&] {
        const auto cs = blockchain::bitcoin::CompactSize(payload_bytes());

        return sizeof(previous_) + (normalized ? 1 : cs.Total()) +
               sizeof(sequence_);
    });
}

auto Input::classify() const noexcept -> Redeem
{
    if (0u == witness_.size()) { return Redeem::None; }

    switch (script_->size()) {
        case 0: {

            return Redeem::MaybeP2WSH;
        }
        case 2: {
            const auto& program = script_->at(0);
            const auto& payload = script_->at(1);

            if (OP::ZERO != program.opcode_) { return Redeem::None; }

            if (false == payload.data_.has_value()) { return Redeem::None; }

            const auto& hash = payload.data_.value();

            switch (hash.size()) {
                case 20: {

                    return Redeem::P2SH_P2WPKH;
                }
                case 32: {

                    return Redeem::P2SH_P2WSH;
                }
                default: {

                    return Redeem::None;
                }
            }
        }
        default: {

            return Redeem::None;
        }
    }
}

auto Input::decode_coinbase() const noexcept -> UnallocatedCString
{
    const auto size = coinbase_.size();

    if (0u == size) { return {}; }

    auto out = std::stringstream{};
    const auto hex = [&] {
        out << "      hex: " << to_hex(coinbase_.data(), coinbase_.size());

        return out.str();
    };

    const auto first = std::to_integer<std::uint8_t>(coinbase_.front());
    const auto* data = reinterpret_cast<const char*>(coinbase_.data());

    switch (first) {
        case 1u: {
            if (1u >= size) { return hex(); }

            auto buf = be::little_int8_buf_t{};
            std::advance(data, 1);
            std::memcpy(&buf, data, sizeof(buf));
            std::advance(data, sizeof(buf));
            out << "      height: " << std::to_string(buf.value()) << ' ';

            if (2u < size) { out << ReadView{data, size - 2u}; }
        } break;
        case 2u: {
            if (2u >= size) { return hex(); }

            auto buf = be::little_int16_buf_t{};
            std::advance(data, 1);
            std::memcpy(&buf, data, sizeof(buf));
            std::advance(data, sizeof(buf));
            out << "      height: " << std::to_string(buf.value()) << ' ';

            if (3u < size) { out << ReadView{data, size - 3u}; }
        } break;
        case 3u: {
            if (3u >= size) { return hex(); }

            auto buf = be::little_int32_buf_t{};
            std::advance(data, 1);
            std::memcpy(&buf, data, sizeof(buf));
            std::advance(data, sizeof(buf));
            out << "      height: " << std::to_string(buf.value()) << ' ';

            if (4u < size) { out << ReadView{data, size - 4u}; }
        } break;
        case 4u: {
            if (4u >= size) { return hex(); }

            auto buf = be::little_int32_buf_t{};
            std::advance(data, 1);
            std::memcpy(&buf, data, sizeof(buf));
            std::advance(data, sizeof(buf));
            out << "      height: " << std::to_string(buf.value()) << ' ';

            if (5u < size) { out << ReadView{data, size - 5u}; }
        } break;
        default: {
            out << "      text: " << reader(coinbase_);
        }
    }

    return out.str();
}

auto Input::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    if (Script::Position::Coinbase == script_->Role()) { return; }

    switch (style) {
        case cfilter::Type::ES: {
            LogTrace()(OT_PRETTY_CLASS())("processing input script").Flush();
            script_->Internal().ExtractElements(style, out);

            if (false == witness_.empty()) {
                for (const auto& data : witness_) {
                    switch (data.size()) {
                        case 33:
                        case 32:
                        case 20: {
                            out.emplace_back(data.cbegin(), data.cend());
                        } break;
                        default: {
                        }
                    }
                }

                if (const auto type{classify()}; type != Redeem::None) {
                    const auto& bytes = *witness_.crbegin();
                    const auto pSub = factory::BitcoinScript(
                        chain_,
                        reader(bytes),
                        Script::Position::Redeem,
                        true,
                        true);

                    if (pSub) {
                        const auto& sub = *pSub;
                        sub.Internal().ExtractElements(style, out);
                    } else if (Redeem::MaybeP2WSH != type) {
                        LogError()(OT_PRETTY_CLASS())("Invalid redeem script")
                            .Flush();
                    }
                }
            }

            [[fallthrough]];
        }
        case cfilter::Type::Basic_BCHVariant: {
            LogTrace()(OT_PRETTY_CLASS())("processing consumed outpoint")
                .Flush();
            const auto* it = reinterpret_cast<const std::byte*>(&previous_);
            out.emplace_back(it, it + sizeof(previous_));
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Unknown:
        default: {
            LogTrace()(OT_PRETTY_CLASS())("skipping input").Flush();
        }
    }
}

auto Input::ExtractElements(const cfilter::Type style, alloc::Default alloc)
    const noexcept -> Elements
{
    auto out = Elements{alloc};
    ExtractElements(style, out);
    std::sort(out.begin(), out.end());

    return out;
}

auto Input::FindMatches(
    const api::Session& api,
    const Txid& txid,
    const cfilter::Type type,
    const Patterns& txos,
    const ParsedPatterns& patterns,
    const std::size_t position,
    const Log& log,
    Matches& out,
    alloc::Default monotonic) const noexcept -> void
{
    auto& [inputs, outputs] = out;

    for (const auto& [element, outpoint] : txos) {
        if (reader(outpoint) != previous_.Bytes()) { continue; }

        inputs.emplace_back(txid, previous_.Bytes(), element);
        const auto& [index, subchainID] = element;
        const auto& [subchain, account] = subchainID;
        cache_.add({account, subchain, index});
        log(OT_PRETTY_CLASS())("input ")(position)(" of transaction ")
            .asHex(txid)(" spends ")(Outpoint{reader(outpoint)})
            .Flush();
    }

    const auto keyMatches = blockchain::block::internal::SetIntersection(
        txid.Bytes(),
        patterns,
        ExtractElements(type, monotonic),
        monotonic,
        monotonic);

    for (const auto& [t, element] : keyMatches.second) {
        inputs.emplace_back(txid, previous_.Bytes(), element);
    }
}

auto Input::GetBytes(std::size_t& base, std::size_t& witness) const noexcept
    -> void
{
    using CS = blockchain::bitcoin::CompactSize;

    {
        const auto cs = CS{payload_bytes()};
        base += sizeof(previous_);
        base += cs.Total();
        base += sizeof(sequence_);
    }
    {
        const auto count = CS{witness_.size()};
        witness += std::accumulate(
            witness_.begin(),
            witness_.end(),
            count.Size(),
            [](const auto lhs, const auto& rhs) {
                const auto cs = CS{rhs.size()};

                return lhs + cs.Total();
            });
    }
}

auto Input::get_pubkeys(const api::Session& api, alloc::Default monotonic)
    const noexcept -> const PubkeyHashes&
{
    const auto instance = api.Instance();
    auto handle = guarded_.lock();
    auto& map = *handle;

    if (auto i = map.find(instance); map.end() != i) {

        return i->second.first;
    } else {
        auto& [pubkeys, _] = map[instance];
        index_elements(api, pubkeys, monotonic);

        return pubkeys;
    }
}

auto Input::get_script_hash(const api::Session& api) const noexcept
    -> const std::optional<ElementHash>&
{
    const auto instance = api.Instance();
    auto handle = guarded_.lock();
    auto& map = *handle;

    if (auto i = map.find(instance); map.end() != i) {

        return i->second.second;
    } else {
        auto& [_, sh] = map[instance];

        if (const auto script = script_->RedeemScript(); script) {
            auto scriptHash = Space{};
            script->CalculateHash160(api.Crypto(), writer(scriptHash));
            sh.emplace(api.Crypto().Blockchain().Internal().IndexItem(
                reader(scriptHash)));
        }

        return sh;
    }
}

auto Input::IndexElements(const api::Session& api, ElementHashes& out)
    const noexcept -> void
{
    // TODO monotonic allocator
    const auto& keys = get_pubkeys(api, {});
    std::copy(keys.begin(), keys.end(), std::inserter(out, out.end()));
}

auto Input::index_elements(
    const api::Session& api,
    PubkeyHashes& hashes,
    alloc::Default monotonic) const noexcept -> void
{
    const auto patterns = [&] {
        auto out = ElementHashes{monotonic};
        out.clear();
        script_->Internal().IndexElements(api, out);

        return out;
    }();
    LogTrace()(OT_PRETTY_CLASS())(patterns.size())(" pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto{
            hashes.emplace(id);
            LogTrace()("    * ")(id).Flush();
        });
}

auto Input::MergeMetadata(
    const internal::Input& rhs,
    const std::size_t index,
    const Log& log) noexcept -> bool
{
    return cache_.merge(rhs, index, log);
}

auto Input::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const std::size_t index,
    const Log& log) const noexcept -> opentxs::Amount
{
    return cache_.net_balance_change(crypto, nym, index, log);
}

auto Input::payload_bytes() const noexcept -> std::size_t
{
    if (0u < coinbase_.size()) {

        return coinbase_.size();
    } else {

        return script_->CalculateSize();
    }
}

auto Input::Print() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << "    outpoint: " << previous_.str() << '\n';
    auto count{0};
    const auto total = witness_.size();

    for (const auto& witness : witness_) {
        out << "    witness " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << "      " << to_hex(witness.data(), witness.size()) << '\n';
    }

    if (Script::Position::Coinbase == script_->Role()) {
        out << "    coinbase: " << '\n';
        out << decode_coinbase() << '\n';
    } else {
        out << "    script: " << '\n';
        out << script_->Print();
    }

    out << "    sequence: " << std::to_string(sequence_) << '\n';

    return out.str();
}

auto Input::ReplaceScript() noexcept -> bool
{
    try {
        const auto& output = cache_.spends();
        auto subscript = output.SigningSubscript();

        if (false == bool(subscript)) {
            throw std::runtime_error("Failed to obtain signing subscript");
        }

        auto& script =
            const_cast<std::unique_ptr<const block::Script>&>(script_);
        script.reset(subscript.release());
        cache_.reset_size();

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Input::serialize(Writer&& destination, const bool normalized)
    const noexcept -> std::optional<std::size_t>
{
    try {
        const auto size = CalculateSize(normalized);
        auto buf = reserve(std::move(destination), size, "input");
        serialize_object(previous_, buf, "previous output");
        const auto isCoinbase{0 < coinbase_.size()};
        const auto bytes = normalized   ? 0_uz
                           : isCoinbase ? coinbase_.size()
                                        : script_->CalculateSize();
        serialize_compact_size(bytes, buf, "script size");

        if (false == normalized) {
            if (isCoinbase) {
                copy(reader(coinbase_), buf, "coinbase");
            } else {
                if (false == script_->Serialize(buf.Write(bytes))) {

                    throw std::runtime_error{"failed to serialize script"};
                }
            }
        }

        const auto sequence = be::little_uint32_buf_t{sequence_};
        serialize_object(sequence, buf, "sequence");
        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Input::Serialize(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(std::move(destination), false);
}

auto Input::Serialize(
    const api::Session& api,
    const std::uint32_t index,
    SerializeType& out) const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {

        return false;
    }

    out.set_sequence(sequence_);
    auto& outpoint = *out.mutable_previous();
    outpoint.set_version(outpoint_version_);
    outpoint.set_txid(UnallocatedCString{previous_.Txid()});
    outpoint.set_index(previous_.Index());
    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(BlockchainToUnit(chain_))));
        serializedKey.set_nym(
            api.Crypto().Blockchain().Owner(key).asBase58(api.Crypto()));
        serializedKey.set_subaccount(accountID.asBase58(api.Crypto()));
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    });

    // TODO monotonic allocator
    for (const auto& id : get_pubkeys(api, {})) { out.add_pubkey_hash(id); }

    if (const auto& sh = get_script_hash(api); sh.has_value()) {
        out.set_script_hash(sh.value());
    }

    out.set_indexed(true);

    try {
        auto& spends = *out.mutable_spends();
        cache_.spends().Serialize(api, spends);
    } catch (...) {
        out.clear_spends();
    }

    return true;
}

auto Input::SerializeNormalized(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(std::move(destination), true);
}

auto Input::SignatureVersion() const noexcept
    -> std::unique_ptr<internal::Input>
{
    auto output = std::make_unique<Input>(
        *this,
        factory::BitcoinScript(
            chain_, ScriptElements{}, Script::Position::Input));

    OT_ASSERT(output);

    output->cache_.reset_size();

    return output;
}

auto Input::SignatureVersion(std::unique_ptr<internal::Script> subscript)
    const noexcept -> std::unique_ptr<internal::Input>
{
    return std::make_unique<Input>(*this, std::move(subscript));
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
