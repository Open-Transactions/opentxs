// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/script/Imp.hpp"  // IWYU pragma: associated

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
#include <type_traits>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::bitcoin::block::implementation
{
Script::Script(
    const blockchain::Type chain,
    const script::Position role,
    std::span<value_type> elements,
    std::optional<std::size_t> size,
    allocator_type alloc) noexcept
    : ScriptPrivate(alloc)
    , chain_(chain)
    , role_(role)
    , elements_([&] {
        auto out = decltype(elements_){alloc};
        out.reserve(elements.size());
        out.clear();
        std::move(elements.begin(), elements.end(), std::back_inserter(out));

        return out;
    }())
    , type_(get_type(role_, elements_))
    , size_(size)
{
}

Script::Script(const Script& rhs, allocator_type alloc) noexcept
    : ScriptPrivate(rhs, alloc)
    , chain_(rhs.chain_)
    , role_(rhs.role_)
    , elements_(rhs.elements_, alloc)
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

auto Script::bytes(std::span<const value_type> script) noexcept -> std::size_t
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

auto Script::clone(allocator_type alloc) const noexcept -> Script*
{
    auto pmr = alloc::PMR<Script>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto Script::decode(const std::byte in) noexcept(false) -> script::OP
{
    using enum script::OP;
    static constexpr auto map =
        frozen::make_unordered_map<std::uint8_t, script::OP>({
            {value(ZERO), ZERO},
            {value(PUSHDATA_1), PUSHDATA_1},
            {value(PUSHDATA_2), PUSHDATA_2},
            {value(PUSHDATA_3), PUSHDATA_3},
            {value(PUSHDATA_4), PUSHDATA_4},
            {value(PUSHDATA_5), PUSHDATA_5},
            {value(PUSHDATA_6), PUSHDATA_6},
            {value(PUSHDATA_7), PUSHDATA_7},
            {value(PUSHDATA_8), PUSHDATA_8},
            {value(PUSHDATA_9), PUSHDATA_9},
            {value(PUSHDATA_10), PUSHDATA_10},
            {value(PUSHDATA_11), PUSHDATA_11},
            {value(PUSHDATA_12), PUSHDATA_12},
            {value(PUSHDATA_13), PUSHDATA_13},
            {value(PUSHDATA_14), PUSHDATA_14},
            {value(PUSHDATA_15), PUSHDATA_15},
            {value(PUSHDATA_16), PUSHDATA_16},
            {value(PUSHDATA_17), PUSHDATA_17},
            {value(PUSHDATA_18), PUSHDATA_18},
            {value(PUSHDATA_19), PUSHDATA_19},
            {value(PUSHDATA_20), PUSHDATA_20},
            {value(PUSHDATA_21), PUSHDATA_21},
            {value(PUSHDATA_22), PUSHDATA_22},
            {value(PUSHDATA_23), PUSHDATA_23},
            {value(PUSHDATA_24), PUSHDATA_24},
            {value(PUSHDATA_25), PUSHDATA_25},
            {value(PUSHDATA_26), PUSHDATA_26},
            {value(PUSHDATA_27), PUSHDATA_27},
            {value(PUSHDATA_28), PUSHDATA_28},
            {value(PUSHDATA_29), PUSHDATA_29},
            {value(PUSHDATA_30), PUSHDATA_30},
            {value(PUSHDATA_31), PUSHDATA_31},
            {value(PUSHDATA_32), PUSHDATA_32},
            {value(PUSHDATA_33), PUSHDATA_33},
            {value(PUSHDATA_34), PUSHDATA_34},
            {value(PUSHDATA_35), PUSHDATA_35},
            {value(PUSHDATA_36), PUSHDATA_36},
            {value(PUSHDATA_37), PUSHDATA_37},
            {value(PUSHDATA_38), PUSHDATA_38},
            {value(PUSHDATA_39), PUSHDATA_39},
            {value(PUSHDATA_40), PUSHDATA_40},
            {value(PUSHDATA_41), PUSHDATA_41},
            {value(PUSHDATA_42), PUSHDATA_42},
            {value(PUSHDATA_43), PUSHDATA_43},
            {value(PUSHDATA_44), PUSHDATA_44},
            {value(PUSHDATA_45), PUSHDATA_45},
            {value(PUSHDATA_46), PUSHDATA_46},
            {value(PUSHDATA_47), PUSHDATA_47},
            {value(PUSHDATA_48), PUSHDATA_48},
            {value(PUSHDATA_49), PUSHDATA_49},
            {value(PUSHDATA_50), PUSHDATA_50},
            {value(PUSHDATA_51), PUSHDATA_51},
            {value(PUSHDATA_52), PUSHDATA_52},
            {value(PUSHDATA_53), PUSHDATA_53},
            {value(PUSHDATA_54), PUSHDATA_54},
            {value(PUSHDATA_55), PUSHDATA_55},
            {value(PUSHDATA_56), PUSHDATA_56},
            {value(PUSHDATA_57), PUSHDATA_57},
            {value(PUSHDATA_58), PUSHDATA_58},
            {value(PUSHDATA_59), PUSHDATA_59},
            {value(PUSHDATA_60), PUSHDATA_60},
            {value(PUSHDATA_61), PUSHDATA_61},
            {value(PUSHDATA_62), PUSHDATA_62},
            {value(PUSHDATA_63), PUSHDATA_63},
            {value(PUSHDATA_64), PUSHDATA_64},
            {value(PUSHDATA_65), PUSHDATA_65},
            {value(PUSHDATA_66), PUSHDATA_66},
            {value(PUSHDATA_67), PUSHDATA_67},
            {value(PUSHDATA_68), PUSHDATA_68},
            {value(PUSHDATA_69), PUSHDATA_69},
            {value(PUSHDATA_70), PUSHDATA_70},
            {value(PUSHDATA_71), PUSHDATA_71},
            {value(PUSHDATA_72), PUSHDATA_72},
            {value(PUSHDATA_73), PUSHDATA_73},
            {value(PUSHDATA_74), PUSHDATA_74},
            {value(PUSHDATA_75), PUSHDATA_75},
            {value(PUSHDATA1), PUSHDATA1},
            {value(PUSHDATA2), PUSHDATA2},
            {value(PUSHDATA4), PUSHDATA4},
            {value(ONE_NEGATE), ONE_NEGATE},
            {value(RESERVED), RESERVED},
            {value(ONE), ONE},
            {value(TWO), TWO},
            {value(THREE), THREE},
            {value(FOUR), FOUR},
            {value(FIVE), FIVE},
            {value(SIX), SIX},
            {value(SEVEN), SEVEN},
            {value(EIGHT), EIGHT},
            {value(NINE), NINE},
            {value(TEN), TEN},
            {value(ELEVEN), ELEVEN},
            {value(TWELVE), TWELVE},
            {value(THIRTEEN), THIRTEEN},
            {value(FOURTEEN), FOURTEEN},
            {value(FIFTEEN), FIFTEEN},
            {value(SIXTEEN), SIXTEEN},
            {value(NOP), NOP},
            {value(VER), VER},
            {value(IF), IF},
            {value(NOTIF), NOTIF},
            {value(VERIF), VERIF},
            {value(VERNOTIF), VERNOTIF},
            {value(ELSE), ELSE},
            {value(ENDIF), ENDIF},
            {value(VERIFY), VERIFY},
            {value(RETURN), RETURN},
            {value(TOALTSTACK), TOALTSTACK},
            {value(FROMALTSTACK), FROMALTSTACK},
            {value(TWO_DROP), TWO_DROP},
            {value(TWO_DUP), TWO_DUP},
            {value(THREE_DUP), THREE_DUP},
            {value(TWO_OVER), TWO_OVER},
            {value(TWO_ROT), TWO_ROT},
            {value(TWO_SWAP), TWO_SWAP},
            {value(IFDUP), IFDUP},
            {value(DEPTH), DEPTH},
            {value(DROP), DROP},
            {value(DUP), DUP},
            {value(NIP), NIP},
            {value(OVER), OVER},
            {value(PICK), PICK},
            {value(ROLL), ROLL},
            {value(ROT), ROT},
            {value(SWAP), SWAP},
            {value(TUCK), TUCK},
            {value(CAT), CAT},
            {value(SUBSTR), SUBSTR},
            {value(LEFT), LEFT},
            {value(RIGHT), RIGHT},
            {value(SIZE), SIZE},
            {value(INVERT), INVERT},
            {value(AND), AND},
            {value(OR), OR},
            {value(XOR), XOR},
            {value(EQUAL), EQUAL},
            {value(EQUALVERIFY), EQUALVERIFY},
            {value(RESERVED1), RESERVED1},
            {value(RESERVED2), RESERVED2},
            {value(ONE_ADD), ONE_ADD},
            {value(ONE_SUB), ONE_SUB},
            {value(TWO_MUL), TWO_MUL},
            {value(TWO_DIV), TWO_DIV},
            {value(NEGATE), NEGATE},
            {value(ABS), ABS},
            {value(NOT), NOT},
            {value(ZERO_NOTEQUAL), ZERO_NOTEQUAL},
            {value(ADD), ADD},
            {value(SUB), SUB},
            {value(MUL), MUL},
            {value(DIV), DIV},
            {value(MOD), MOD},
            {value(LSHIFT), LSHIFT},
            {value(RSHIFT), RSHIFT},
            {value(BOOLAND), BOOLAND},
            {value(BOOLOR), BOOLOR},
            {value(NUMEQUAL), NUMEQUAL},
            {value(NUMEQUALVERIFY), NUMEQUALVERIFY},
            {value(NUMNOTEQUAL), NUMNOTEQUAL},
            {value(LESSTHAN), LESSTHAN},
            {value(GREATERTHAN), GREATERTHAN},
            {value(LESSTHANOREQUAL), LESSTHANOREQUAL},
            {value(GREATERTHANOREQUAL), GREATERTHANOREQUAL},
            {value(MIN), MIN},
            {value(MAX), MAX},
            {value(WITHIN), WITHIN},
            {value(RIPEMD160), RIPEMD160},
            {value(SHA1), SHA1},
            {value(SHA256), SHA256},
            {value(HASH160), HASH160},
            {value(HASH256), HASH256},
            {value(CODESEPARATOR), CODESEPARATOR},
            {value(CHECKSIG), CHECKSIG},
            {value(CHECKSIGVERIFY), CHECKSIGVERIFY},
            {value(CHECKMULTISIG), CHECKMULTISIG},
            {value(CHECKMULTISIGVERIFY), CHECKMULTISIGVERIFY},
            {value(NOP1), NOP1},
            {value(NOP2), NOP2},
            {value(NOP3), NOP3},
            {value(NOP4), NOP4},
            {value(NOP5), NOP5},
            {value(NOP6), NOP6},
            {value(NOP7), NOP7},
            {value(NOP8), NOP8},
            {value(NOP9), NOP9},
            {value(NOP10), NOP10},
            {value(PUBKEYHASH), PUBKEYHASH},
            {value(PUBKEY), PUBKEY},
            {value(INVALIDOPCODE), INVALIDOPCODE},
        });

    if (const auto* i = map.find(std::to_integer<std::uint8_t>(in));
        map.end() != i) {

        return i->second;
    } else {

        throw std::out_of_range{"unknown opcode"};
    }
}

auto Script::evaluate_data(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(2 <= script.size());

    using enum script::Pattern;

    for (auto i = 1_uz; i < script.size(); ++i) {
        if (false == is_data_push(script[i])) { return Custom; }
    }

    return NullData;
}

auto Script::evaluate_multisig(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(4 <= script.size());

    auto it = script.rbegin();
    std::advance(it, 1);
    const auto n = to_number(it->opcode_);
    using enum script::Pattern;

    if (0u == n) { return Malformed; }
    if ((3u + n) > script.size()) { return Malformed; }

    for (auto i = std::uint8_t{0}; i < n; ++i) {
        std::advance(it, 1);

        if (false == is_public_key(*it)) { return Malformed; }
    }

    std::advance(it, 1);
    const auto m = to_number(it->opcode_);

    if ((0u == m) || (m > n)) { return Malformed; }

    if ((3u + n) < script.size()) { return Custom; }

    return PayToMultisig;
}

auto Script::evaluate_pubkey(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(2 == script.size());

    using enum script::Pattern;

    if (is_public_key(script[0])) { return PayToPubkey; }

    return Custom;
}

auto Script::evaluate_pubkey_hash(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(5 == script.size());

    auto it = script.begin();
    using enum script::OP;
    using enum script::Pattern;

    if (DUP != it->opcode_) { return Custom; }

    std::advance(it, 1);

    if (HASH160 != it->opcode_) { return Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Custom; }

    std::advance(it, 1);

    if (EQUALVERIFY != it->opcode_) { return Custom; }

    return PayToPubkeyHash;
}

auto Script::evaluate_script_hash(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(3 == script.size());

    auto it = script.begin();
    using enum script::OP;
    using enum script::Pattern;

    if (HASH160 != it->opcode_) { return Custom; }

    std::advance(it, 1);

    if (false == is_hash160(*it)) { return Custom; }
    if (PUSHDATA_20 != it->opcode_) { return Custom; }

    return PayToScriptHash;
}

auto Script::evaluate_segwit(std::span<const value_type> script) noexcept
    -> script::Pattern
{
    OT_ASSERT(2 == script.size());

    const auto& opcode = script[0].opcode_;
    const auto& program = script[1].data_.value();
    using enum script::Pattern;

    switch (to_number(opcode)) {
        case 0: {
            switch (program.size()) {
                case 20: {
                    return PayToWitnessPubkeyHash;
                }
                case 32: {
                    return PayToWitnessScriptHash;
                }
                default: {
                }
            }
        } break;
        case 1: {
            switch (program.size()) {
                case 32: {
                    return PayToTaproot;
                }
                default: {
                }
            }
        } break;
        default: {
        }
    }

    return Custom;
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

            for (const auto& element : get()) {
                if (is_data_push(element)) {
                    const auto& data = element.data_.value();
                    const auto* it = static_cast<const std::byte*>(data.data());

                    switch (data.size()) {
                        case 65_uz: {
                            std::advance(it, 1_uz);
                            [[fallthrough]];
                        }
                        case 64_uz: {
                            out.emplace_back(it, it + 32_uz);
                            std::advance(it, 32_uz);
                            out.emplace_back(it, it + 32_uz);
                            [[fallthrough]];
                        }
                        case 33_uz:
                        case 32_uz:
                        case 20_uz: {
                            out.emplace_back(data.cbegin(), data.cend());
                        } break;
                        default: {
                        }
                    }
                }
            }

            // TODO monotonic allocator
            if (const auto subscript = RedeemScript({}); subscript.IsValid()) {
                subscript.Internal().ExtractElements(style, out);
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant:
        case cfilter::Type::Unknown:
        default: {
            using enum script::OP;

            if (RETURN == elements_.at(0).opcode_) {
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

auto Script::first_opcode(std::span<const value_type> script) noexcept
    -> script::OP
{
    OT_ASSERT(false == script.empty());

    return script.begin()->opcode_;
}

auto Script::get_data(const std::size_t position) const noexcept(false)
    -> ReadView
{
    const auto& data = elements_.at(position).data_;

    if (false == data.has_value()) {
        throw std::out_of_range("No data at specified script position");
    }

    return data.value().Bytes();
}

auto Script::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

auto Script::get_opcode(const std::size_t position) const noexcept(false)
    -> script::OP
{
    return elements_.at(position).opcode_;
}

auto Script::get_type(
    const script::Position role,
    std::span<const value_type> script) noexcept -> script::Pattern
{
    using enum script::Position;

    if (0 == script.size()) { return script::Pattern::Empty; }

    switch (role) {
        case Coinbase: {

            return script::Pattern::Coinbase;
        }
        case Input: {

            return script::Pattern::Input;
        }
        case Redeem:
        case Output: {
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
                return script::Pattern::Custom;
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
    using enum script::Pattern;

    if (PayToMultisig != Type()) { return false; }

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

auto Script::is_direct_push(const script::OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    const auto val = value(opcode);

    if ((0 < val) && (76 > val)) { return val; }

    return std::nullopt;
}

auto Script::is_push(const script::OP opcode) noexcept(false)
    -> std::optional<std::size_t>
{
    constexpr auto low = 75_uz;
    constexpr auto high = 79_uz;
    constexpr auto shift = low + 1_uz;
    constexpr auto one = 1_uz;
    const auto val = std::size_t{value(opcode)};

    if ((low < val) && (high > val)) { return one << (val - shift); }

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

auto Script::last_opcode(std::span<const value_type> script) noexcept
    -> script::OP
{
    OT_ASSERT(false == script.empty());

    return script.rbegin()->opcode_;
}

auto Script::LikelyPubkeyHashes(const api::Crypto& crypto) const noexcept
    -> UnallocatedVector<ByteArray>
{
    using enum script::Pattern;
    auto output = UnallocatedVector<ByteArray>{};

    switch (type_) {
        case PayToPubkeyHash: {
            const auto hash = PubkeyHash();

            OT_ASSERT(hash.has_value());

            output.emplace_back(hash.value());
        } break;
        case PayToMultisig: {
            for (auto i = std::uint8_t{0}; i < N().value(); ++i) {
                auto hash = ByteArray{};
                const auto key = MultisigPubkey(i);

                OT_ASSERT(key.has_value());

                blockchain::PubkeyHash(
                    crypto, chain_, key.value(), hash.WriteInto());
                output.emplace_back(std::move(hash));
            }
        } break;
        case PayToPubkey: {
            auto hash = ByteArray{};
            const auto key = Pubkey();

            OT_ASSERT(key.has_value());

            blockchain::PubkeyHash(
                crypto, chain_, key.value(), hash.WriteInto());
            output.emplace_back(std::move(hash));
        } break;
        case Coinbase:
        case PayToScriptHash:
        case Empty:
        case Malformed: {
        } break;
        case Custom:
        case NullData:
        case Input:
        case PayToWitnessPubkeyHash:  // TODO
        case PayToWitnessScriptHash:
        case PayToTaproot:
        case None:
        default: {
            for (const auto& element : elements_) {
                if (is_hash160(element)) {
                    OT_ASSERT(element.data_.has_value());

                    output.emplace_back(element.data_.value().Bytes());
                } else if (is_public_key(element)) {
                    OT_ASSERT(element.data_.has_value());

                    auto hash = ByteArray{};
                    blockchain::PubkeyHash(
                        crypto,
                        chain_,
                        element.data_.value().Bytes(),
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
    using enum script::Pattern;

    if (PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(0));
}

auto Script::MultisigPubkey(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    using enum script::Pattern;

    if (PayToMultisig != type_) { return {}; }

    const auto index = position + 1_uz;

    if (index > N()) { return {}; }

    return get_data(index);
}

auto Script::N() const noexcept -> std::optional<std::uint8_t>
{
    using enum script::Pattern;

    if (PayToMultisig != type_) { return {}; }

    return to_number(get_opcode(elements_.size() - 2));
}

auto Script::potential_data(std::span<const value_type> script) noexcept -> bool
{
    using enum script::OP;

    return (RETURN == first_opcode(script)) && (2 <= script.size());
}

auto Script::potential_multisig(std::span<const value_type> script) noexcept
    -> bool
{
    using enum script::OP;

    return (CHECKMULTISIG == last_opcode(script)) && (4 <= script.size());
}

auto Script::potential_pubkey(std::span<const value_type> script) noexcept
    -> bool
{
    using enum script::OP;

    return (CHECKSIG == last_opcode(script)) && (2 == script.size());
}

auto Script::potential_pubkey_hash(std::span<const value_type> script) noexcept
    -> bool
{
    using enum script::OP;

    return (CHECKSIG == last_opcode(script)) && (5 == script.size());
}

auto Script::potential_script_hash(std::span<const value_type> script) noexcept
    -> bool
{
    using enum script::OP;

    return (HASH160 == first_opcode(script)) &&
           (EQUAL == last_opcode(script)) && (3 == script.size());
}

auto Script::potential_segwit(std::span<const value_type> script) noexcept
    -> bool
{
    using enum script::OP;

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

    const auto& element = script[1];

    if (false == element.data_.has_value()) { return false; }

    const auto& program = element.data_.value();

    return (1u < program.size()) && (41u > program.size());
}

auto Script::Print() const noexcept -> UnallocatedCString
{
    return Print({}).c_str();
}

auto Script::Print(allocator_type alloc) const noexcept -> CString
{
    // TODO allocator
    auto output = std::stringstream{};

    for (const auto& [opcode, invalid, push, data] : elements_) {
        output << "      op: " << std::to_string(value(opcode));

        if (invalid) {
            output << " invalid: " << std::to_string(invalid.value());
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
            const auto& item = data.value();
            output << " (" << item.size() << ") bytes : " << item.asHex();
        }

        output << '\n';
    }

    return {output.str().c_str(), alloc};
}

auto Script::Pubkey() const noexcept -> std::optional<ReadView>
{
    using enum script::Pattern;

    switch (type_) {
        case PayToPubkey: {
            return get_data(0);
        }
        case PayToTaproot: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::PubkeyHash() const noexcept -> std::optional<ReadView>
{
    using enum script::Pattern;

    switch (type_) {
        case PayToPubkeyHash: {
            return get_data(2);
        }
        case PayToWitnessPubkeyHash: {
            return get_data(1);
        }
        default: {
            return {};
        }
    }
}

auto Script::RedeemScript(allocator_type alloc) const noexcept -> block::Script
{
    using enum script::Position;

    if (Input != role_) { return {alloc}; }
    if (0 == elements_.size()) { return {alloc}; }

    const auto& element = *elements_.crbegin();

    if (false == is_data_push(element)) { return {alloc}; }

    return factory::BitcoinScript(
        chain_, element.data_.value().Bytes(), Redeem, true, true, alloc);
}

auto Script::ScriptHash() const noexcept -> std::optional<ReadView>
{
    using enum script::Pattern;

    switch (type_) {
        case PayToScriptHash:
        case PayToWitnessScriptHash: {
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

            if (bytes.has_value()) { copy(bytes->Bytes(), buf, "argument"); }

            if (data.has_value()) { copy(data->Bytes(), buf, "data"); }
        }

        check_finished(buf);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Script::SigningSubscript(
    const blockchain::Type chain,
    alloc::Default alloc) const noexcept -> block::Script
{
    using enum script::OP;
    using enum script::Pattern;
    auto pmr = alloc::PMR<Script>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    switch (type_) {
        case PayToWitnessPubkeyHash: {
            auto elements = [&] {
                auto e = Vector<value_type>{alloc};
                e.emplace_back(internal::Opcode(DUP));
                e.emplace_back(internal::Opcode(HASH160));
                e.emplace_back(internal::PushData(PubkeyHash().value()));
                e.emplace_back(internal::Opcode(EQUALVERIFY));
                e.emplace_back(internal::Opcode(CHECKSIG));

                return e;
            }();

            pmr.construct(
                out, chain_, script::Position::Output, elements, std::nullopt);
        } break;
        default: {
            // TODO handle OP_CODESEPERATOR shit

            pmr.construct(out, *this);
        }
    }

    return out;
}

auto Script::to_number(const script::OP opcode) noexcept -> std::uint8_t
{
    using enum script::OP;

    if ((ONE <= opcode) && (SIXTEEN >= opcode)) {

        return value(opcode) - value(ONE) + 1u;
    }

    return 0;
}

auto Script::validate(std::span<const value_type> elements) noexcept -> bool
{
    for (const auto& element : elements) {
        if (false == validate(element, false)) { return false; }
    }

    return true;
}

auto Script::validate(
    const value_type& element,
    const bool checkForData) noexcept -> bool
{
    const auto& [opcode, invalid, bytes, data] = element;

    if (invalid.has_value()) { return !checkForData; }

    if (auto size = is_direct_push(opcode); size.has_value()) {
        if (bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }
        if (size != data.value().size()) { return false; }

        return true;
    } else if (auto push = is_push(opcode); push.has_value()) {
        if (false == bytes.has_value()) { return false; }
        if (false == data.has_value()) { return false; }

        auto pBytes = 0_uz;
        const auto& pushBytes = push.value();

        if (pushBytes != bytes->size()) { return false; }

        switch (pushBytes) {
            case 1:
            case 2:
            case 4: {
                auto buf = boost::endian::little_uint32_buf_t{};
                std::memcpy(
                    static_cast<void*>(&buf), bytes.value().data(), pushBytes);
                pBytes = buf.value();
            } break;
            default: {
                OT_FAIL;
            }
        }

        if (pBytes != data.value().size()) { return false; }

        return true;
    } else {

        return !checkForData;
    }
}

auto Script::Value(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    using enum script::Pattern;

    if (NullData != type_) { return {}; }

    const auto index = position + 1u;

    if (index > elements_.size()) { return {}; }

    return get_data(index);
}

Script::~Script() = default;
}  // namespace opentxs::blockchain::bitcoin::block::implementation
