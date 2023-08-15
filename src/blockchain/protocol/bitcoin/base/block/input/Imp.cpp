// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/input/Imp.hpp"  // IWYU pragma: associated

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
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/output/OutputPrivate.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Opcodes.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
const VersionNumber Input::default_version_{1};
const VersionNumber Input::outpoint_version_{1};
const VersionNumber Input::key_version_{1};
const VersionNumber Input::witness_version_{1};

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    Vector<WitnessItem> witness,
    block::Script script,
    ByteArray coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size,
    Set<crypto::Key>&& keys,
    block::Output output,
    allocator_type alloc) noexcept(false)
    : InputPrivate(alloc)
    , chain_(chain)
    , serialize_version_(version)
    , previous_(std::move(previous))
    , witness_(std::move(witness), alloc)
    , script_(std::move(script), alloc)
    , coinbase_(std::move(coinbase), alloc)
    , sequence_(sequence)
    , cache_(std::move(output), std::move(size), std::move(keys), alloc)
{
    if (false == script_.IsValid()) {
        throw std::runtime_error("Invalid input script");
    }

    if ((0 < coinbase_.size()) && (0 < script_.get().size())) {
        throw std::runtime_error("Input has both script and coinbase");
    }
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    Vector<WitnessItem> witness,
    block::Script script,
    const VersionNumber version,
    std::optional<std::size_t> size,
    allocator_type alloc) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          ByteArray{alloc},
          version,
          size,
          Set<crypto::Key>{alloc},
          block::Output{OutputPrivate::Blank(alloc)},
          alloc)
{
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    Vector<WitnessItem> witness,
    block::Script script,
    const VersionNumber version,
    block::Output output,
    Set<crypto::Key>&& keys,
    allocator_type alloc) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          ByteArray{alloc},
          version,
          std::nullopt,
          std::move(keys),
          std::move(output),
          alloc)
{
}

Input::Input(
    const blockchain::Type chain,
    const std::uint32_t sequence,
    Outpoint&& previous,
    Vector<WitnessItem> witness,
    const ReadView coinbase,
    const VersionNumber version,
    block::Output output,
    std::optional<std::size_t> size,
    allocator_type alloc) noexcept(false)
    : Input(
          chain,
          sequence,
          std::move(previous),
          std::move(witness),
          factory::BitcoinScript(
              chain,
              Vector<script::Element>{alloc},
              script::Position::Coinbase,
              alloc),
          ByteArray{coinbase, alloc},
          version,
          size,
          Set<crypto::Key>{alloc},
          std::move(output),
          alloc)
{
}

Input::Input(const Input& rhs, allocator_type alloc) noexcept
    : Input(rhs, rhs.script_, alloc)
{
}

Input::Input(
    const Input& rhs,
    block::Script script,
    allocator_type alloc) noexcept
    : InputPrivate(rhs, alloc)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , previous_(rhs.previous_)
    , witness_(rhs.witness_, alloc)
    , script_(std::move(script), alloc)
    , coinbase_(rhs.coinbase_, alloc)
    , sequence_(rhs.sequence_)
    , cache_(*rhs.cache_.lock(), alloc)
{
}

auto Input::AddMultisigSignatures(const Signatures& signatures) noexcept -> bool
{
    using enum script::OP;
    // TODO allocator
    auto elements = Vector<script::Element>{};
    elements.reserve(1_uz + signatures.size());
    elements.clear();
    elements.emplace_back(internal::Opcode(ZERO));

    for (const auto& [sig, key] : signatures) {
        if (false == valid(sig)) { return false; }

        elements.emplace_back(internal::PushData(sig));
    }

    using enum script::Position;
    // TODO allocator
    script_ = factory::BitcoinScript(chain_, elements, Input, {});
    cache_.lock()->reset_size();

    return script_.IsValid();
}

auto Input::AddSignatures(const Signatures& signatures) noexcept -> bool
{
    if (0 == witness_.size()) {
        auto elements = Vector<script::Element>{};
        elements.reserve(signatures.size());
        elements.clear();

        for (const auto& [sig, key] : signatures) {
            if (false == valid(sig)) { return false; }

            elements.emplace_back(internal::PushData(sig));

            if (valid(key)) { elements.emplace_back(internal::PushData(key)); }
        }

        using enum script::Position;
        script_ = factory::BitcoinScript(chain_, elements, Input, {});
        cache_.lock()->reset_size();

        return script_.IsValid();
    } else {
        // TODO this only works for P2WPKH
        witness_.clear();

        for (const auto& [sig, key] : signatures) {
            if (false == valid(sig)) { return false; }
            if (false == valid(key)) { return false; }

            witness_.emplace_back(reader(space(sig)));
            witness_.emplace_back(reader(space(key)));
        }

        cache_.lock()->reset_size();

        return true;
    }
}

auto Input::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    Set<identifier::Nym>& output) const noexcept -> void
{
    cache_.lock()->for_each_key([&](const auto& key) {
        const auto& owner = crypto.Owner(key);

        if (false == owner.empty()) { output.emplace(owner); }
    });
}

auto Input::AssociatedRemoteContacts(
    const api::session::Client& api,
    Set<identifier::Generic>& output) const noexcept -> void
{
    const auto hashes = script_.Internal().LikelyPubkeyHashes(api.Crypto());
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = api.Crypto().Blockchain().LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::inserter(output, output.end()));
    });

    auto payer = cache_.lock()->payer();

    if (false == payer.empty()) { output.emplace(std::move(payer)); }
}

auto Input::AssociatePreviousOutput(const block::Output& in) noexcept -> bool
{
    return cache_.lock()->associate(in);
}

auto Input::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    return cache_.lock()->size(normalized, [&] {
        const auto cs =
            blockchain::protocol::bitcoin::base::CompactSize(payload_bytes());

        return sizeof(previous_) + (normalized ? 1 : cs.Total()) +
               sizeof(sequence_);
    });
}

auto Input::classify() const noexcept -> Redeem
{
    if (0u == witness_.size()) { return Redeem::None; }

    const auto elements = script_.get();

    switch (elements.size()) {
        case 0: {

            return Redeem::MaybeP2WSH;
        }
        case 2: {
            const auto& program = elements[0];
            const auto& payload = elements[1];
            using enum script::OP;

            if (ZERO != program.opcode_) { return Redeem::None; }

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

auto Input::Coinbase() const noexcept -> ReadView { return coinbase_.Bytes(); }

auto Input::decode_coinbase() const noexcept -> UnallocatedCString
{
    const auto size = coinbase_.size();

    if (0u == size) { return {}; }

    auto out = std::stringstream{};
    const auto hex = [&] {
        out << "      hex: " << coinbase_.asHex();

        return out.str();
    };

    const auto first = std::to_integer<std::uint8_t>(
        *static_cast<const std::byte*>(coinbase_.data()));
    const auto* data = static_cast<const char*>(coinbase_.data());

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
            out << "      text: " << coinbase_.Bytes();
        }
    }

    return out.str();
}

auto Input::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    using enum script::Position;

    if (Coinbase == script_.Role()) { return; }

    switch (style) {
        case cfilter::Type::ES: {
            LogTrace()(OT_PRETTY_CLASS())("processing input script").Flush();
            script_.Internal().ExtractElements(style, out);

            if (false == witness_.empty()) {
                for (const auto& data : witness_) {
                    switch (data.size()) {
                        case 33:
                        case 32:
                        case 20: {
                            auto d = data.get();
                            out.emplace_back(d.begin(), d.end());
                        } break;
                        default: {
                        }
                    }
                }

                if (const auto type{classify()}; type != Redeem::None) {
                    const auto& bytes = *witness_.crbegin();
                    // TODO monotonic allocator
                    const auto sub = factory::BitcoinScript(
                        chain_, bytes.Bytes(), Redeem, true, true, {});

                    if (sub.IsValid()) {
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
        case cfilter::Type::UnknownCfilter:
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
    const TransactionHash& txid,
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
        cache_.lock()->add({account, subchain, index});
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
    using CS = blockchain::protocol::bitcoin::base::CompactSize;

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
    return cache_.lock()->Hashes([&] {
        auto out = PubkeyHashes{get_allocator()};
        out.clear();
        index_elements(api, out, monotonic);

        return out;
    });
}

auto Input::get_script_hash(const api::Session& api) const noexcept
    -> const std::optional<ElementHash>&
{
    return cache_.lock()->ScriptHash(
        [this, &api]() -> std::optional<ElementHash> {
            if (const auto script = script_.RedeemScript({});
                script.IsValid()) {
                auto scriptHash = Space{};
                script.CalculateHash160(api.Crypto(), writer(scriptHash));

                return api.Crypto().Blockchain().Internal().IndexItem(
                    reader(scriptHash));
            } else {

                return std::nullopt;
            }
        });
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
    auto patterns = [&] {
        auto out = ElementHashes{monotonic};
        out.clear();
        script_.Internal().IndexElements(api, out);

        return out;
    }();
    std::move(
        std::begin(patterns),
        std::end(patterns),
        std::inserter(hashes, hashes.end()));
}

auto Input::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    auto out = Set<crypto::Key>{alloc};
    out.clear();
    cache_.lock()->keys(out);

    return out;
}

auto Input::Keys(Set<crypto::Key>& out) const noexcept -> void
{
    cache_.lock()->keys(out);
}

auto Input::MergeMetadata(
    const api::Crypto& crypto,
    const internal::Input& rhs,
    const std::size_t index,
    const Log& log) noexcept -> void
{
    cache_.lock()->merge(crypto, rhs, index, log);
}

auto Input::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const std::size_t index,
    const Log& log) const noexcept -> opentxs::Amount
{
    return cache_.lock()->net_balance_change(crypto, nym, index, log);
}

auto Input::payload_bytes() const noexcept -> std::size_t
{
    if (0u < coinbase_.size()) {

        return coinbase_.size();
    } else {

        return script_.CalculateSize();
    }
}

auto Input::Print(const api::Crypto& crypto) const noexcept
    -> UnallocatedCString
{
    return Print(crypto, {}).c_str();
}

auto Input::Print(const api::Crypto& crypto, alloc::Default alloc)
    const noexcept -> CString
{
    // TODO allocator
    auto out = std::stringstream{};
    out << "    outpoint: " << previous_.str() << '\n';
    auto count{0};
    const auto total = witness_.size();
    using enum script::Position;

    for (const auto& witness : witness_) {
        out << "    witness " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << "      " << witness.asHex() << '\n';
    }

    if (Coinbase == script_.Role()) {
        out << "    coinbase: " << '\n';
        out << decode_coinbase() << '\n';
    } else {
        out << "    script: " << '\n';
        out << script_.Print();
    }

    out << "    sequence: " << std::to_string(sequence_) << '\n';
    out << "    value of consumed output: ";

    try {
        const auto& definition = blockchain::GetDefinition(chain_);
        out << definition.Format(Spends().Value()) << '\n';
    } catch (...) {
        out << "unknown\n";
    }

    out << "    associated keys: " << '\n';
    cache_.lock()->for_each_key([&](const auto& key) {
        out << "        * " << print(key, crypto) << '\n';
    });

    return CString{out.str(), alloc};
}

auto Input::ReplaceScript() noexcept -> bool
{
    try {
        const auto& output = cache_.lock()->spends();
        // TODO allocator
        auto subscript = output.Internal().SigningSubscript({});

        if (false == subscript.IsValid()) {
            throw std::runtime_error("Failed to obtain signing subscript");
        }

        script_ = std::move(subscript);
        cache_.lock()->reset_size();

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
                                        : script_.CalculateSize();
        serialize_compact_size(bytes, buf, "script size");

        if (false == normalized) {
            if (isCoinbase) {
                copy(coinbase_.Bytes(), buf, "coinbase");
            } else {
                if (false == script_.Serialize(buf.Write(bytes))) {

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

    if (coinbase_.empty()) {
        if (false == script_.Serialize(writer(*out.mutable_script()))) {

            return false;
        }
    } else {
        out.set_script(UnallocatedCString{coinbase_.Bytes()});
    }

    out.set_sequence(sequence_);
    auto& outpoint = *out.mutable_previous();
    outpoint.set_version(outpoint_version_);
    outpoint.set_txid(UnallocatedCString{previous_.Txid()});
    outpoint.set_index(previous_.Index());

    if (false == witness_.empty()) {
        auto& witness = *out.mutable_witness();
        witness.set_version(witness_version_);

        for (const auto& item : witness_) {
            witness.add_item(UnallocatedCString{item.Bytes()});
        }
    }

    cache_.lock()->for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, idx] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(BlockchainToUnit(chain_))));
        serializedKey.set_nym(
            api.Crypto().Blockchain().Owner(key).asBase58(api.Crypto()));
        serializedKey.set_subaccount(accountID.asBase58(api.Crypto()));
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(idx);
    });

    // TODO monotonic allocator
    for (const auto& id : get_pubkeys(api, {})) { out.add_pubkey_hash(id); }

    if (const auto& sh = get_script_hash(api); sh.has_value()) {
        out.set_script_hash(sh.value());
    }

    out.set_indexed(true);

    try {
        auto& spends = *out.mutable_spends();
        cache_.lock()->spends().Internal().Serialize(api, spends);
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

auto Input::SignatureVersion(alloc::Default alloc) const noexcept
    -> block::Input
{
    return SignatureVersion(
        factory::BitcoinScript(
            chain_,
            Vector<script::Element>{alloc},
            script::Position::Input,
            alloc),
        alloc);
}

auto Input::SignatureVersion(block::Script subscript, alloc::Default alloc)
    const noexcept -> block::Input
{
    auto* out = pmr::construct<Input>(alloc, *this, std::move(subscript));
    out->cache_.lock()->reset_size();

    return out;
}

Input::~Input() = default;
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
