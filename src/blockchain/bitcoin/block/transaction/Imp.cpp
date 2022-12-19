// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemType

#include "blockchain/bitcoin/block/transaction/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>  // IWYU pragma: keep
#include <utility>

#include "TBB.hpp"
#include "blockchain/block/transaction/TransactionPrivate.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"   // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/core/Amount.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Transaction::default_version_{1};

Transaction::Transaction(
    const VersionNumber serializeVersion,
    const bool isGeneration,
    const std::int32_t version,
    const std::byte segwit,
    const std::uint32_t lockTime,
    const TransactionHash& txid,
    const TransactionHash& wtxid,
    const Time& time,
    std::string_view memo,
    std::span<blockchain::bitcoin::block::Input> inputs,
    std::span<blockchain::bitcoin::block::Output> outputs,
    Set<blockchain::Type> chains,
    block::Position&& minedPosition,
    std::optional<std::size_t>&& position,
    allocator_type alloc) noexcept(false)
    : blockchain::block::TransactionPrivate(alloc)
    , blockchain::block::implementation::Transaction(
          std::move(txid),
          std::move(wtxid),
          alloc)
    , TransactionPrivate(alloc)
    , position_(std::move(position))
    , serialize_version_(serializeVersion)
    , is_generation_(isGeneration)
    , version_(version)
    , segwit_flag_(segwit)
    , lock_time_(lockTime)
    , time_(time)
    , inputs_(move_construct<block::Input>(inputs, alloc))
    , outputs_(move_construct<block::Output>(outputs, alloc))
    , data_(memo, std::move(chains), std::move(minedPosition), alloc)
{
}

Transaction::Transaction(const Transaction& rhs, allocator_type alloc) noexcept
    : blockchain::block::TransactionPrivate(rhs, alloc)
    , blockchain::block::implementation::Transaction(rhs, alloc)
    , TransactionPrivate(rhs, alloc)
    , position_(rhs.position_)
    , serialize_version_(rhs.serialize_version_)
    , is_generation_(rhs.is_generation_)
    , version_(rhs.version_)
    , segwit_flag_(rhs.segwit_flag_)
    , lock_time_(rhs.lock_time_)
    , time_(rhs.time_)
    , inputs_(rhs.inputs_, alloc)
    , outputs_(rhs.outputs_, alloc)
    , data_(*rhs.data_.lock(), alloc)
{
}

auto Transaction::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    alloc::Default alloc) const noexcept -> Set<identifier::Nym>
{
    auto output = Set<identifier::Nym>{alloc};
    output.clear();
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Internal().AssociatedLocalNyms(crypto, output);
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Internal().AssociatedLocalNyms(crypto, output);
        });

    return output;
}

auto Transaction::AssociatePreviousOutput(
    const std::size_t index,
    const block::Output& output) noexcept -> bool
{
    if (index >= inputs_.size()) {
        LogError()(OT_PRETTY_CLASS())("invalid index").Flush();

        return {};
    }

    return inputs_[index].Internal().AssociatePreviousOutput(output);
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Client& api,
    const identifier::Nym& nym,
    alloc::Default alloc) const noexcept -> Set<identifier::Generic>
{
    auto output = Set<identifier::Generic>{alloc};
    output.clear();
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Internal().AssociatedRemoteContacts(api, output);
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Internal().AssociatedRemoteContacts(api, output);
        });
    output.erase(api.Contacts().ContactID(nym));

    return output;
}

auto Transaction::base_size() const noexcept -> std::size_t
{
    static constexpr auto fixed = sizeof(version_) + sizeof(lock_time_);

    return fixed + calculate_input_size(false) + calculate_output_size();
}

auto Transaction::BlockPosition() const noexcept -> std::optional<std::size_t>
{
    return position_;
}

auto Transaction::CalculateSize() const noexcept -> std::size_t
{
    return calculate_size(false, *data_.lock());
}

auto Transaction::calculate_input_size(const bool normalize) const noexcept
    -> std::size_t
{
    using Range = tbb::blocked_range<const block::Input*>;
    const auto& data = inputs_;
    const auto cs = blockchain::bitcoin::CompactSize(data.size());

    return cs.Size() +
           tbb::parallel_reduce(
               Range{data.data(), std::next(data.data(), data.size())},
               0_uz,
               [normalize](const Range& r, std::size_t init) {
                   for (const auto& i : r) {
                       init += i.Internal().CalculateSize(normalize);
                   }

                   return init;
               },
               [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_output_size() const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const block::Output*>;
    const auto& data = outputs_;
    const auto cs = blockchain::bitcoin::CompactSize(data.size());

    return cs.Size() +
           tbb::parallel_reduce(
               Range{data.data(), std::next(data.data(), data.size())},
               0_uz,
               [](const Range& r, std::size_t init) {
                   for (const auto& i : r) {
                       init += i.Internal().CalculateSize();
                   }

                   return init;
               },
               [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_size(const bool normalize, transaction::Data& data)
    const noexcept -> std::size_t
{
    return data.size(normalize, [&] {
        const bool isSegwit =
            (false == normalize) && (std::byte{0x00} != segwit_flag_);

        return sizeof(version_) + calculate_input_size(normalize) +
               calculate_output_size() +
               (isSegwit ? calculate_witness_size() : 0_uz) +
               sizeof(lock_time_);
    });
}

auto Transaction::calculate_witness_size(const WitnessItem& in) noexcept
    -> std::size_t
{
    return blockchain::bitcoin::CompactSize{in.size()}.Total();
}

auto Transaction::calculate_witness_size(
    std::span<const WitnessItem> in) noexcept -> std::size_t
{
    using Range = tbb::blocked_range<std::size_t>;
    const auto cs = blockchain::bitcoin::CompactSize{in.size()};

    return cs.Size() +
           tbb::parallel_reduce(
               Range{0_uz, in.size()},
               0_uz,
               [in](const Range& r, std::size_t init) {
                   for (auto i = r.begin(); i != r.end(); ++i) {
                       const auto& witness = in[i];
                       init += calculate_witness_size(witness);
                   }

                   return init;
               },
               [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::calculate_witness_size() const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const block::Input*>;
    const auto& data = inputs_;
    constexpr auto fixed = 2_uz;  // NOTE: segwit marker and flag

    return fixed +
           tbb::parallel_reduce(
               Range{data.data(), std::next(data.data(), data.size())},
               0_uz,
               [](const Range& r, std::size_t init) {
                   for (const auto& i : r) {
                       init += calculate_witness_size(i.Witness());
                   }

                   return init;
               },
               [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Transaction::Chains(allocator_type alloc) const noexcept
    -> Set<blockchain::Type>
{
    return data_.lock()->chains(alloc);
}

auto Transaction::ConfirmationHeight() const noexcept -> block::Height
{
    return data_.lock()->height();
}

auto Transaction::IDNormalized(const api::Factory& factory) const noexcept
    -> const identifier::Generic&
{
    auto handle = data_.lock();
    auto& data = *handle;

    return data.normalized([&] {
        auto preimage = Space{};
        const auto serialized = serialize(writer(preimage), true, data);

        OT_ASSERT(serialized);

        return factory.IdentifierFromPreimage(
            reader(preimage), identifier::Algorithm::sha256);
    });
}

auto Transaction::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Internal().ExtractElements(style, out);
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Internal().ExtractElements(style, out);
        });

    if (cfilter::Type::ES == style) {
        const auto* data = static_cast<const std::byte*>(id_.data());
        out.emplace_back(data, data + id_.size());
    }
}

auto Transaction::FindMatches(
    const api::Session& api,
    const cfilter::Type style,
    const Patterns& txos,
    const ParsedPatterns& elements,
    const Log& log,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Matches
{
    auto output = std::make_pair(InputMatches{alloc}, OutputMatches{alloc});
    FindMatches(api, style, txos, elements, log, output, monotonic);
    auto& [inputs, outputs] = output;
    dedup(inputs);
    dedup(outputs);

    return output;
}

auto Transaction::FindMatches(
    const api::Session& api,
    const cfilter::Type type,
    const Patterns& txos,
    const ParsedPatterns& elements,
    const Log& log,
    Matches& out,
    alloc::Default monotonic) const noexcept -> void
{
    log(OT_PRETTY_CLASS())("processing transaction ").asHex(ID()).Flush();

    auto index = 0_uz;
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Internal().FindMatches(
                api, id_, type, txos, elements, index, log, out, monotonic);
            ++index;
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Internal().FindMatches(
                api, id_, type, elements, log, out, monotonic);
        });
}

auto Transaction::ForTestingOnlyAddKey(
    const std::size_t index,
    const blockchain::crypto::Key& key) noexcept -> bool
{
    if (index >= outputs_.size()) {
        LogError()(OT_PRETTY_CLASS())("invalid index").Flush();

        return false;
    }

    outputs_[index].Internal().ForTestingOnlyAddKey(key);

    return true;
}

auto Transaction::GetPreimageBTC(
    const std::size_t index,
    const blockchain::bitcoin::SigHash& hashType) const noexcept -> Space
{
    if (SigHash::All != hashType.Type()) {
        // TODO
        LogError()(OT_PRETTY_CLASS())("Mode not supported").Flush();

        return {};
    }

    // TODO monotonic allocator
    auto copy = Transaction{*this, get_allocator()};
    copy.data_.lock()->reset_size();

    if (index >= inputs_.size()) {
        LogError()(OT_PRETTY_CLASS())("invalid index").Flush();

        return {};
    }

    auto& base = copy.inputs_[index];
    auto& input = base.Internal();

    if (false == input.ReplaceScript()) {
        LogError()(OT_PRETTY_CLASS())("Failed to initialize input script")
            .Flush();

        return {};
    }

    if (hashType.AnyoneCanPay()) {
        auto replace = decltype(copy.inputs_){copy.get_allocator()};
        replace.clear();
        replace.emplace_back(std::move(base));
        copy.inputs_.swap(replace);
    }

    auto output = Space{};
    copy.Serialize(writer(output));

    return output;
}

auto Transaction::IndexElements(const api::Session& api, alloc::Default alloc)
    const noexcept -> ElementHashes
{
    auto output = ElementHashes{alloc};
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Internal().IndexElements(api, output);
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Internal().IndexElements(api, output);
        });

    return output;
}

auto Transaction::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    auto out = Set<crypto::Key>{alloc};
    out.clear();
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            txin.Keys(out);
        });
    std::for_each(
        std::begin(outputs_), std::end(outputs_), [&](const auto& txout) {
            txout.Keys(out);
        });

    return out;
}

auto Transaction::Memo(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    return Memo(crypto, {}).c_str();
}

auto Transaction::Memo(
    const api::crypto::Blockchain& crypto,
    alloc::Default alloc) const noexcept -> CString
{
    auto memo = CString{alloc};
    memo = data_.lock()->memo();

    if (false == memo.empty()) { return memo; }

    for (const auto& output : outputs_) {
        memo = output.Note(crypto).c_str();

        if (false == memo.empty()) { return memo; }
    }

    return {};
}

auto Transaction::MinedPosition() const noexcept -> const block::Position&
{
    return data_.lock()->position();
}

auto Transaction::MergeMetadata(
    const api::crypto::Blockchain& crypto,
    const blockchain::Type chain,
    const internal::Transaction& rhs,
    const Log& log) noexcept -> void
{
    log(OT_PRETTY_CLASS())("merging transaction ").asHex(ID()).Flush();

    if (id_ != rhs.ID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong transaction").Flush();

        return;
    }

    const auto iSize = inputs_.size();
    const auto oSize = outputs_.size();
    const auto rTxin = rhs.Inputs();
    const auto rTxout = rhs.Outputs();

    if (iSize != rTxin.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of inputs").Flush();

        return;
    }

    if (oSize != rTxout.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number of outputs").Flush();

        return;
    }

    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, inputs_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& lTx = inputs_[i].Internal();
                const auto& rTx = rTxin[i].Internal();
                lTx.MergeMetadata(rTx, i, log);
            }
        });
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, outputs_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& lTx = outputs_[i].Internal();
                const auto& rTx = rTxout[i].Internal();
                lTx.MergeMetadata(rTx, log);
            }
        });
    data_.lock()->merge(crypto, rhs, log);
}

auto Transaction::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    const auto& log = LogTrace();
    log(OT_PRETTY_CLASS())("parsing transaction ")
        .asHex(ID())(" for balance change with respect to nym ")(nym)
        .Flush();
    auto index = 0_uz;
    // TODO index is only needed for logging purpose so remove this need and use
    // a parallel algorithm
    const auto spent = std::accumulate(
        std::begin(inputs_),
        std::end(inputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& txin) -> auto{
            return prev +
                   txin.Internal().NetBalanceChange(crypto, nym, index++, log);
        });
    const auto created = std::accumulate(
        std::begin(outputs_),
        std::end(outputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& txout) -> auto{
            return prev + txout.Internal().NetBalanceChange(crypto, nym, log);
        });
    const auto total = spent + created;
    log(OT_PRETTY_CLASS())
        .asHex(ID())(" total input contribution is ")(
            spent)(" and total output contribution is ")(
            created)(" for a net balance change of ")(total)
        .Flush();

    return total;
}

auto Transaction::Print() const noexcept -> UnallocatedCString
{
    return Print({}).c_str();
}

auto Transaction::Print(alloc::Default alloc) const noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << "  version: " << std::to_string(version_) << '\n';
    auto count{0};
    const auto inputs = Inputs().size();
    const auto outputs = Outputs().size();

    for (const auto& input : Inputs()) {
        out << "  input " << std::to_string(++count);
        out << " of " << std::to_string(inputs) << '\n';
        out << input.Print();
    }

    count = 0;

    for (const auto& output : Outputs()) {
        out << "  output " << std::to_string(++count);
        out << " of " << std::to_string(outputs) << '\n';
        out << output.Print();
    }

    out << "  locktime: " << std::to_string(lock_time_) << '\n';

    return {out.str().c_str(), alloc};
}

auto Transaction::serialize(
    Writer&& destination,
    const bool normalize,
    transaction::Data& data) const noexcept -> std::optional<std::size_t>
{
    try {
        const auto size = calculate_size(normalize, data);
        auto buf = reserve(std::move(destination), size, "transaction");
        const auto version = be::little_int32_buf_t{version_};
        serialize_object(version, buf, "version");
        const auto isSegwit = [&] {
            static constexpr auto marker = std::byte{0x00};
            const auto val = (false == normalize) && (marker != segwit_flag_);

            if (val) {
                serialize_object(marker, buf, "segwit marker");
                serialize_object(segwit_flag_, buf, "segwit flag");
            }

            return val;
        }();
        const auto inCount = blockchain::bitcoin::CompactSize(inputs_.size());
        serialize_compact_size(inCount, buf, "input count");
        std::for_each(std::begin(inputs_), std::end(inputs_), [&](auto& txin) {
            const auto& input = txin.Internal();
            const auto expected = input.CalculateSize(normalize);
            const auto wrote =
                normalize ? input.SerializeNormalized(buf.Write(expected))
                          : input.Serialize(buf.Write(expected));

            if ((false == wrote.has_value()) || (*wrote != expected)) {

                throw std::runtime_error{"failed to serialize input"};
            }
        });
        const auto outCount = blockchain::bitcoin::CompactSize(outputs_.size());
        serialize_compact_size(outCount, buf, "input count");
        std::for_each(
            std::begin(outputs_), std::end(outputs_), [&](auto& txout) {
                const auto& output = txout.Internal();
                const auto expected = output.CalculateSize();
                const auto wrote = output.Serialize(buf.Write(expected));

                if ((false == wrote.has_value()) || (*wrote != expected)) {

                    throw std::runtime_error{"failed to serialize output"};
                }
            });

        if (isSegwit) {
            for (const auto& input : inputs_) {
                const auto& witness = input.Witness();
                serialize_compact_size(witness.size(), buf, "witness count");

                for (const auto& push : witness) {
                    serialize_compact_size(push.size(), buf, "witness size");
                    copy(push.Bytes(), buf, "witness");
                }
            }
        }

        const auto lockTime = be::little_uint32_buf_t{lock_time_};
        serialize_object(lockTime, buf, "locktime");
        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Transaction::Serialize(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(std::move(destination), false, *data_.lock());
}

auto Transaction::Serialize(const api::Session& api) const noexcept
    -> std::optional<SerializeType>
{
    auto output = SerializeType{};
    output.set_version(std::max(default_version_, serialize_version_));
    auto handle = data_.lock();
    auto& data = *handle;

    // TODO monotonic allocator
    for (const auto chain : data.chains(get_allocator())) {
        output.add_chain(translate(UnitToClaim(BlockchainToUnit(chain))));
    }

    output.set_txid(UnallocatedCString{id_.Bytes()});
    output.set_txversion(version_);
    output.set_locktime(lock_time_);

    if (false == serialize(writer(*output.mutable_serialized()), false, data)) {
        return {};
    }

    auto index = std::uint32_t{0};

    try {
        std::for_each(std::begin(inputs_), std::end(inputs_), [&](auto& txin) {
            auto& out = *output.add_input();

            if (false == txin.Internal().Serialize(api, index++, out)) {
                throw;
            }
        });
    } catch (...) {

        return {};
    }

    try {
        std::for_each(
            std::begin(outputs_), std::end(outputs_), [&](auto& txout) {
                auto& out = *output.add_output();

                if (false == txout.Internal().Serialize(api, out)) { throw; }
            });
    } catch (...) {

        return {};
    }

    // TODO optional uint32 confirmations = 9;
    // TODO optional string blockhash = 10;
    // TODO optional uint32 blockindex = 11;
    // TODO optional uint64 fee = 12;
    output.set_time(Clock::to_time_t(time_));
    // TODO repeated string conflicts = 14;
    output.set_memo(UnallocatedCString{data.memo()});
    output.set_segwit_flag(std::to_integer<std::uint32_t>(segwit_flag_));
    output.set_wtxid(UnallocatedCString{Hash().Bytes()});
    output.set_is_generation(is_generation_);
    const auto& [height, hash] = data.position();

    if ((0 <= height) && (false == hash.IsNull())) {
        output.set_mined_height(height);
        output.set_mined_block(UnallocatedCString{hash.Bytes()});
    }

    return output;
}

auto Transaction::Serialize(EncodedTransaction& out) const noexcept -> bool
{
    try {
        out.version_ = version_;

        if (std::byte{0x0} != segwit_flag_) { out.segwit_flag_ = segwit_flag_; }

        {
            const auto inputs = inputs_.size();
            out.input_count_ = CompactSize{inputs};
            out.inputs_.reserve(inputs);
        }

        auto haveWitness{false};

        for (const auto& input : inputs_) {
            auto& next = out.inputs_.emplace_back();
            const auto& outpoint = input.PreviousOutput();
            static_assert(
                sizeof(outpoint.txid_) == sizeof(next.outpoint_.txid_));
            static_assert(
                sizeof(outpoint.index_) == sizeof(next.outpoint_.index_));
            std::memcpy(
                next.outpoint_.txid_.data(),
                outpoint.txid_.data(),
                outpoint.txid_.size());
            std::memcpy(
                next.outpoint_.index_.data(),
                outpoint.index_.data(),
                outpoint.index_.size());

            if (auto coinbase = input.Coinbase(); coinbase.empty()) {
                const auto& script = input.Script();
                next.cs_ = CompactSize{script.CalculateSize()};
                script.Serialize(next.script_.WriteInto());
            } else {
                next.cs_ = CompactSize{coinbase.size()};
                copy(coinbase, next.script_.WriteInto());
            }

            const auto& segwit = input.Witness();
            auto& witness = out.witnesses_.emplace_back();
            witness.cs_ = CompactSize{segwit.size()};

            for (const auto& item : segwit) {
                haveWitness = true;
                auto& val = witness.items_.emplace_back();
                val.cs_ = CompactSize{item.size()};
                val.item_.Assign(item);
            }

            next.sequence_ = input.Sequence();
        }

        if (false == haveWitness) { out.witnesses_.clear(); }

        {
            const auto outputs = outputs_.size();
            out.output_count_ = CompactSize{outputs};
            out.outputs_.reserve(outputs);
        }

        for (const auto& output : outputs_) {
            auto& next = out.outputs_.emplace_back();
            output.Value().Internal().SerializeBitcoin(
                preallocated(sizeof(next.value_), std::addressof(next.value_)));
            const auto& script = output.Script();
            next.cs_ = CompactSize{script.CalculateSize()};
            script.Serialize(next.script_.WriteInto());
        }

        out.lock_time_ = lock_time_;

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Transaction::SetKeyData(const KeyData& data) noexcept -> void
{
    std::for_each(std::begin(inputs_), std::end(inputs_), [&](auto& txin) {
        txin.Internal().SetKeyData(data);
    });
    std::for_each(std::begin(outputs_), std::end(outputs_), [&](auto& txout) {
        txout.Internal().SetKeyData(data);
    });
}

auto Transaction::SetMemo(const std::string_view memo) noexcept -> void
{
    data_.lock()->set_memo(memo);
}

auto Transaction::SetMinedPosition(const block::Position& pos) noexcept -> void
{
    return data_.lock()->set_position(pos);
}

auto Transaction::SetPosition(std::size_t position) noexcept -> void
{
    const_cast<std::optional<std::size_t>&>(position_) = position;
}

auto Transaction::vBytes(blockchain::Type chain) const noexcept -> std::size_t
{
    const auto& data = params::get(chain);
    const auto total = CalculateSize();

    if (data.SupportsSegwit()) {
        const auto& scale = data.SegwitScaleFactor();

        OT_ASSERT(0 < scale);

        const auto weight = (base_size() * (scale - 1u)) + total;
        static constexpr auto ceil = [](const auto a, const auto b) {
            return (a + (b - 1u)) / b;
        };

        return ceil(weight, scale);
    } else {

        return CalculateSize();
    }
}

Transaction::~Transaction() = default;
}  // namespace opentxs::blockchain::bitcoin::block::implementation
