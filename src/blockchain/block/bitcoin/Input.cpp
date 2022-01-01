// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Input.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <memory_resource>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/contact/Contact.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainInputWitness.pb.h"
#include "serialization/protobuf/BlockchainPreviousOutput.pb.h"
#include "serialization/protobuf/BlockchainTransactionInput.pb.h"
#include "serialization/protobuf/BlockchainWalletKey.pb.h"

namespace be = boost::endian;

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Input;
using Position = blockchain::block::bitcoin::Script::Position;

auto BitcoinTransactionInput(
    const api::Session& api,
    const blockchain::Type chain,
    const UTXO& spends,
    const std::optional<std::uint32_t> sequence) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    namespace b = opentxs::blockchain;
    namespace bb = b::block::bitcoin;
    namespace bi = bb::internal;

    auto& pPrevOut = spends.second;

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
    auto witness = std::pmr::vector<Space>{};

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

    auto keys = boost::container::flat_set<blockchain::crypto::Key>{};
    std::for_each(
        std::begin(outputKeys), std::end(outputKeys), [&](const auto& key) {
            keys.emplace(key);
        });

    return std::make_unique<ReturnType>(
        api,
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
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase,
    std::pmr::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    try {
        auto buf = be::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {
            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());

        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
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
                api,
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
    const api::Session& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput in,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>
{
    const auto& outpoint = in.previous();
    auto witness = std::pmr::vector<Space>{};

    for (const auto& bytes : in.witness().item()) {
        const auto it = reinterpret_cast<const std::byte*>(bytes.data());
        witness.emplace_back(it, it + bytes.size());
    }

    auto spends =
        std::unique_ptr<blockchain::block::bitcoin::internal::Output>{};

    if (in.has_spends()) {
        spends = factory::BitcoinTransactionOutput(api, chain, in.spends());
    }

    try {
        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
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
            auto keys = boost::container::flat_set<blockchain::crypto::Key>{};
            auto pkh = boost::container::flat_set<blockchain::PatternID>{};

            for (const auto& key : in.key()) {
                keys.emplace(
                    key.subaccount(),
                    static_cast<blockchain::crypto::Subchain>(
                        static_cast<std::uint8_t>(key.subchain())),
                    key.index());
            }

            for (const auto& pattern : in.pubkey_hash()) {
                pkh.emplace(pattern);
            }

            return std::make_unique<ReturnType>(
                api,
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
                std::move(pkh),
                (in.has_script_hash()
                     ? std::make_optional<blockchain::PatternID>(
                           in.script_hash())
                     : std::nullopt),
                in.indexed(),
                std::move(spends));
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Input::default_version_{1};
const VersionNumber Input::outpoint_version_{1};
const VersionNumber Input::key_version_{1};

Input::Input(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::pmr::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    Space&& coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size,
    boost::container::flat_set<crypto::Key>&& keys,
    boost::container::flat_set<PatternID>&& pubkeyHashes,
    std::optional<PatternID>&& scriptHash,
    const bool indexed,
    std::unique_ptr<const internal::Output> output) noexcept(false)
    : api_(api)
    , chain_(chain)
    , serialize_version_(version)
    , previous_(std::move(previous))
    , witness_(std::move(witness))
    , script_(std::move(script))
    , coinbase_(std::move(coinbase))
    , sequence_(sequence)
    , pubkey_hashes_(std::move(pubkeyHashes))
    , script_hash_(std::move(scriptHash))
    , cache_(api, std::move(output), std::move(size), std::move(keys))
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid input script");
    }

    if ((0 < coinbase_.size()) && (0 < script_->size())) {
        throw std::runtime_error("Input has both script and coinbase");
    }

    if (false == indexed) { index_elements(); }
}

Input::Input(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::pmr::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          size,
          {},
          {},
          {},
          false,
          nullptr)
{
}

Input::Input(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::pmr::vector<Space>&& witness,
    std::unique_ptr<const internal::Script> script,
    const VersionNumber version,
    std::unique_ptr<const internal::Output> output,
    boost::container::flat_set<crypto::Key>&& keys) noexcept(false)
    : Input(
          api,
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          {},
          std::move(keys),
          {},
          {},
          false,
          std::move(output))
{
}

Input::Input(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::pmr::vector<Space>&& witness,
    const ReadView coinbase,
    const VersionNumber version,
    std::unique_ptr<const internal::Output> output,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
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
          {},
          {},
          false,
          std::move(output))
{
}

Input::Input(const Input& rhs) noexcept
    : Input(rhs, rhs.script_->clone())
{
}

Input::Input(
    const Input& rhs,
    std::unique_ptr<const internal::Script> script) noexcept
    : api_(rhs.api_)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , previous_(rhs.previous_)
    , witness_(rhs.witness_.begin(), rhs.witness_.end())
    , script_(std::move(script))
    , coinbase_(rhs.coinbase_.begin(), rhs.coinbase_.end())
    , sequence_(rhs.sequence_)
    , pubkey_hashes_(rhs.pubkey_hashes_)
    , script_hash_(rhs.script_hash_)
    , cache_(rhs.cache_)
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

    auto& script =
        const_cast<std::unique_ptr<const internal::Script>&>(script_);
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
            const_cast<std::unique_ptr<const internal::Script>&>(script_);
        script = factory::BitcoinScript(
            chain_, std::move(elements), Script::Position::Input);
        cache_.reset_size();

        return bool(script);
    } else {
        // TODO this only works for P2WPKH
        auto& witness = const_cast<std::pmr::vector<Space>&>(witness_);
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
    std::pmr::vector<OTNymID>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = api_.Crypto().Blockchain().Owner(key);

        if (false == owner.empty()) { output.emplace_back(owner); }
    });
}

auto Input::AssociatedRemoteContacts(
    std::pmr::vector<OTIdentifier>& output) const noexcept -> void
{
    const auto hashes = script_->LikelyPubkeyHashes(api_);
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = api_.Crypto().Blockchain().LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });

    auto payer = cache_.payer();

    if (false == payer->empty()) { output.emplace_back(std::move(payer)); }
}

auto Input::AssociatePreviousOutput(const internal::Output& in) noexcept -> bool
{
    return cache_.associate(in);
}

auto Input::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    return cache_.size(normalized, [&] {
        const auto data = (0 < coinbase_.size()) ? coinbase_.size()
                                                 : script_->CalculateSize();
        const auto cs = blockchain::bitcoin::CompactSize(data);

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

auto Input::decode_coinbase() const noexcept -> std::string
{
    const auto size = coinbase_.size();

    if (0u == size) { return {}; }

    auto out = std::stringstream{};
    const auto hex = [&] {
        const auto data = api_.Factory().Data(reader(coinbase_));
        out << "      hex: " << data->asHex();

        return out.str();
    };

    const auto first = std::to_integer<std::uint8_t>(coinbase_.front());
    auto* data = reinterpret_cast<const char*>(coinbase_.data());

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

auto Input::ExtractElements(const filter::Type style) const noexcept
    -> std::pmr::vector<Space>
{
    auto output = std::pmr::vector<Space>{};

    if (Script::Position::Coinbase == script_->Role()) { return output; }

    switch (style) {
        case filter::Type::ES: {

            LogTrace()(OT_PRETTY_CLASS())("processing input script").Flush();
            output = script_->ExtractElements(style);

            for (const auto& data : witness_) {
                switch (data.size()) {
                    case 33:
                    case 32:
                    case 20: {
                        output.emplace_back(data.cbegin(), data.cend());
                    } break;
                    default: {
                    }
                }
            }

            if (const auto type{classify()}; type != Redeem::None) {
                OT_ASSERT(1 < witness_.size());

                const auto& bytes = *witness_.crbegin();
                const auto pSub = factory::BitcoinScript(
                    chain_,
                    reader(bytes),
                    Script::Position::Redeem,
                    true,
                    true);

                if (pSub) {
                    const auto& sub = *pSub;
                    auto temp = sub.ExtractElements(style);
                    output.insert(
                        output.end(),
                        std::make_move_iterator(temp.begin()),
                        std::make_move_iterator(temp.end()));
                } else if (Redeem::MaybeP2WSH != type) {
                    LogError()(OT_PRETTY_CLASS())("Invalid redeem script")
                        .Flush();
                }
            }

            [[fallthrough]];
        }
        case filter::Type::Basic_BCHVariant: {
            LogTrace()(OT_PRETTY_CLASS())("processing consumed outpoint")
                .Flush();
            auto it = reinterpret_cast<const std::byte*>(&previous_);
            output.emplace_back(it, it + sizeof(previous_));
        } break;
        case filter::Type::Basic_BIP158:
        default: {
            LogTrace()(OT_PRETTY_CLASS())("skipping input").Flush();
        }
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Input::FindMatches(
    const ReadView txid,
    const filter::Type type,
    const Patterns& txos,
    const ParsedPatterns&) const noexcept -> Matches
{
    auto matches = Matches{};
    auto& [inputs, outputs] = matches;

    for (const auto& [element, outpoint] : txos) {
        if (reader(outpoint) != previous_.Bytes()) { continue; }

        inputs.emplace_back(
            api_.Factory().Data(txid), previous_.Bytes(), element);
        const auto& [index, subchainID] = element;
        const auto& [subchain, account] = subchainID;
        cache_.add({account->str(), subchain, index});
    }

    LogTrace()(OT_PRETTY_CLASS())("Verified ")(inputs.size())(
        " outpoint matches")
        .Flush();

    return matches;
}

auto Input::GetPatterns() const noexcept -> std::pmr::vector<PatternID>
{
    return {std::begin(pubkey_hashes_), std::end(pubkey_hashes_)};
}

auto Input::index_elements() noexcept -> void
{
    auto& hashes =
        const_cast<boost::container::flat_set<PatternID>&>(pubkey_hashes_);
    const auto patterns = script_->ExtractPatterns(api_);
    LogTrace()(OT_PRETTY_CLASS())(patterns.size())(" pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto {
            hashes.emplace(id);
            LogTrace()("    * ")(id).Flush();
        });
    const auto script = script_->RedeemScript();

    if (script) {
        auto scriptHash = Space{};
        script->CalculateHash160(api_, writer(scriptHash));
        const_cast<std::optional<PatternID>&>(script_hash_) =
            api_.Crypto().Blockchain().IndexItem(reader(scriptHash));
    }
}

auto Input::MergeMetadata(const internal::Input& rhs) noexcept -> bool
{
    return cache_.merge(rhs);
}

auto Input::Print() const noexcept -> std::string
{
    auto out = std::stringstream{};
    out << "    outpoint: " << previous_.str() << '\n';
    auto count{0};
    const auto total = witness_.size();

    for (const auto& witness : witness_) {
        const auto bytes = api_.Factory().Data(reader(witness));
        out << "    witness " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << "      " << bytes->asHex() << '\n';
    }

    if (Script::Position::Coinbase == script_->Role()) {
        out << "    coinbase: " << '\n';
        out << decode_coinbase() << '\n';
        ;
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
            const_cast<std::unique_ptr<const internal::Script>&>(script_);
        script.reset(subscript.release());
        cache_.reset_size();

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Input::serialize(const AllocateOutput destination, const bool normalized)
    const noexcept -> std::optional<std::size_t>
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize(normalized);
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &previous_, sizeof(previous_));
    std::advance(it, sizeof(previous_));
    const auto isCoinbase{0 < coinbase_.size()};
    const auto cs = normalized ? blockchain::bitcoin::CompactSize(0)
                               : blockchain::bitcoin::CompactSize(
                                     isCoinbase ? coinbase_.size()
                                                : script_->CalculateSize());
    const auto csData = cs.Encode();
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (false == normalized) {
        if (isCoinbase) {
            std::memcpy(it, coinbase_.data(), coinbase_.size());
        } else {
            if (false == script_->Serialize(preallocated(cs.Value(), it))) {
                LogError()(OT_PRETTY_CLASS())("Failed to serialize script")
                    .Flush();

                return std::nullopt;
            }
        }

        std::advance(it, cs.Value());
    }

    auto buf = be::little_uint32_buf_t{sequence_};
    std::memcpy(static_cast<void*>(it), &buf, sizeof(buf));

    return size;
}

auto Input::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Input::Serialize(const std::uint32_t index, SerializeType& out)
    const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {

        return false;
    }

    out.set_sequence(sequence_);
    auto& outpoint = *out.mutable_previous();
    outpoint.set_version(outpoint_version_);
    outpoint.set_txid(std::string{previous_.Txid()});
    outpoint.set_index(previous_.Index());
    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(BlockchainToUnit(chain_))));
        serializedKey.set_nym(api_.Crypto().Blockchain().Owner(key).str());
        serializedKey.set_subaccount(accountID);
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    });

    for (const auto& id : pubkey_hashes_) { out.add_pubkey_hash(id); }

    if (script_hash_.has_value()) { out.set_script_hash(script_hash_.value()); }

    out.set_indexed(true);

    try {
        auto& spends = *out.mutable_spends();
        cache_.spends().Serialize(spends);
    } catch (...) {
    }

    return true;
}

auto Input::SerializeNormalized(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, true);
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

    return std::move(output);
}

auto Input::SignatureVersion(std::unique_ptr<internal::Script> subscript)
    const noexcept -> std::unique_ptr<internal::Input>
{
    return std::make_unique<Input>(*this, std::move(subscript));
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
