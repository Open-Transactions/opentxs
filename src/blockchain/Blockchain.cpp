// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"

constexpr auto BITMASK(std::uint64_t n) noexcept -> std::uint64_t
{
    constexpr auto one = std::uint64_t{1u};

    return (one << n) - one;
}

namespace bmp = boost::multiprecision;

namespace opentxs::blockchain
{
auto GetDefinition(blockchain::Type in) noexcept -> const display::Definition&
{
    return display::GetDefinition(BlockchainToUnit(in));
}

}  // namespace opentxs::blockchain

namespace opentxs::blockchain::internal
{
BitReader::BitReader(const Vector<std::byte>& data)
    : raw_data_(data)
    , data_(reinterpret_cast<const std::uint8_t*>(raw_data_.data()))
    , len_(raw_data_.size())
    , accum_(0)
    , n_(0)
{
}

auto BitReader::eof() -> bool { return (len_ == 0u && n_ == 0u); }

auto BitReader::read(std::size_t nbits) -> std::uint64_t
{
    // nbits is the number of bits the programmer is trying to read from our
    // internal data and, interpreted as big endian, return to the caller in
    // native format. Let's say he's trying to read 19 bits.
    OT_ASSERT(nbits < 32u);

    auto ret = std::uint64_t{0u};

    // The first loop iteration, nbits is therefore 19.
    while (nbits) {

        // n_ starts out as zero. Therefore this if() will resolve to true.
        // This is because accum_ contains no data, since all the input data
        // is in the ByteArray member raw_data_.
        if (!n_) {
            // Let's say the raw data contains 500 bytes. so len_ is 500,
            // and definitely larger than 4 bytes.
            if (len_ > 4u) {
                // We grab the next 4 bytes from the raw data into a
                // uint32_t. The first byte is left-shifted 24 bits. The
                // second 16 bits, the third 8 bits, and the fourth 0 bits,
                // which are all OR'd together.
                accum_ = (static_cast<std::uint32_t>(data_[0]) << 24u) |
                         (static_cast<std::uint32_t>(data_[1]) << 16u) |
                         (static_cast<std::uint32_t>(data_[2]) << 8u) |
                         (static_cast<std::uint32_t>(data_[3]));
                // The data iterator is incremented by 4 bytes.
                data_ += 4u;
                // The length we're trying to read is decremented by 4
                // bytes.
                len_ -= 4u;
                // The number of bits in accum_ are incremented by 32.
                n_ += 32u;
            }
            // If the raw data didn't even contain 4 bytes, then does it at
            // least contain more than 0?
            else if (len_ > 0u) {
                // Dereference data, grab one byte to accum_, and increment
                // the data pointer.
                accum_ = *data_++;
                // We read one byte, so decrement len_ which is the number
                // of bytes needing to be read from raw_data_.
                --len_;
                // n_ records that we now have 8 more bits of data in
                // accum_.
                n_ += 8u;
            } else {
                return 0u;
            }
        }

        // n_ is the number of bits in accum_, and nbits is the number of
        // bits the programmer is trying to read. We choose the smaller. So
        // if there are 10 bits in accum_, and he's trying to read 19,
        // 'toread' gets set to 10.
        const auto toread = std::min<std::size_t>(n_, nbits);
        // ret is 0 on the first loop iteration. So this does nothing. But
        // subsequent iterations will, say, left-shift 10 bits to make room
        // for the 10 being read from accum_.
        ret <<= toread;
        // Ret is OR'd with accum_. accum_ is right-shifted by, say, from
        // (19 bits available) to (10 bits we're reading), subtracting that
        // results in 9 bits remaining to slice off accum_ before OR'ing
        // them into return value.
        ret |= (accum_ >> (n_ - toread));
        // We've read, say 10 bits, so n_ is reduced from, say 19 to 9. That
        // is, 9 bits left in accum_ to be read.
        n_ -= toread;
        // nbits (number we're trying to read) is decremented by the number
        // we've just read so far. Eventually it will read zero and this
        // loop will end.
        nbits -= toread;
        // accum_ has, say, 9 bits left to read. So all the other bits are
        // bitmasked away.
        accum_ &= BITMASK(n_);
    }

    return ret;
}

// output will contain the result after flush.
BitWriter::BitWriter(Vector<std::byte>& output)
    : output_(output)
    , accum_(0)
    , n_(0)
{
}

void BitWriter::flush()
{
    if (n_ > 0u) {
        // Assert n is smaller than 8 because if it were larger, it should
        // already have been handled in the write function. Remember, n_ is
        // the number of bits stored in accum_.
        OT_ASSERT(n_ < 8u);

        auto result{static_cast<std::uint8_t>(accum_ & BITMASK(n_))};
        result <<= (8u - n_);

        output_.emplace_back(std::byte{result});

        // Since we read all the n_ bits out of accum_, we set both back to
        // 0.
        n_ = 0u;
        accum_ = 0u;
    }
}

void BitWriter::write(std::size_t nbits, std::uint64_t value)
{
    // Let's say we're writing 19 bits. nbits starts at 19.
    while (nbits) {
        // nb is number of bits we'll actually write to accum_ before we're
        // forced to flush accum_ to the output_ data member. n_ is the
        // number of bits we have queued in the accum_ member, which will be
        // concatenated to the output_ member on flush. Let's say there are
        // 7 bits sitting in accum_ now, and so n_ would contain 7. (the bit
        // count) and accum_ contains the 7 bits themselves. nb is the
        // smaller of (64 - 7) and 19. Why? 64 bits minus 7 (57) represents
        // the number of bits left in accum_ for writing to, before it runs
        // out of space and would have to be flushed to the output_ variable
        // ByteArray. Why do I care if 19 is smaller than 57? Well if I'm
        // writing 19 bits, and there are 57 still available to write in
        // before flushing, then I know I'm writing all 19 bits. But what if
        // there were only 10 bits left in accum_ instead of 57? Then I
        // could only write 10 of my 19 bits, before flushing to output_ and
        // finally grabbing the last 9 bits into accum_, with n_ finally set
        // to 9. Therefore, nb is the number of bits we'll ACTUALLY write
        // before flushing to do the rest.

        auto nb = std::min(ACCUM_BITS - n_, nbits);

        // Next we bitshift accum_ to make room for the new bits being
        // written into it. Since nb is the number of bits actually being
        // writte, that is how much we bitshift accum_.
        accum_ <<= nb;

        // Next we take the value being written, and bitmask it for the
        // number of bits we're grabbing from it. For example, if we're
        // writing 19 bits from 'value', then I bitmask that variable to
        // only select out those 19 bits.
        value &= BITMASK(nbits);

        // accum_, already bitshifted to make space, has value added to its
        // empty bits. Why (nbits - nb)? nbits is the number that I WANT to
        // write, whereas nb is the actual available bits in accum_. If I
        // want to write 19 (nbits), but nb available is 10, then 19-10 = 9.
        // That's the number of bits we don't have room for yet. So here
        // accum_ is OR'd with value right-shifted by 9 bits. We've cut off
        // the 9 bits we don't have room for yet, leaving the bitmasked 10
        // bits that we DO have room to write. Those are the bits that get
        // OR'd into accum_.
        accum_ |= value >> (nbits - nb);

        // Since we have written 10 bits to accum_, then n_ is increased
        // by 10.
        n_ += nb;

        // Since we have written nb bits to accum_, the number of bits we
        // are TRYING to write (nbits) is decremented by nb. For example, 19
        // bits (trying to write) is reduced by 10 bits (actually wrote)
        // leaving 9 bits (remaining still to write).
        nbits -= nb;

        // BY THIS POINT, what we've done is either write ALL of our new
        // bits to accum_, OR wrote as many as would fit.
        while (n_ >= 8u) {
            // Why n_ - 8? n_ contains the number of bits in accum_. We know
            // that it's at least 8 (or larger). Say there are 19 bits in
            // accum_. n_ -8 would be 11. So accum_ is right-shifted 11 bits
            // in order to get the most significant 8 bits first. Those 8
            // bits are bitmasked and copied into result.
            std::uint8_t result = (accum_ >> (n_ - 8u)) & BITMASK(8u);

            // result is then concatenated to output_.
            output_.emplace_back(std::byte{result});
            // n_ (the number of bits containing data in accum_) is
            // decremented by 8, so from 19 to 11.
            n_ -= 8u;

            // Here accum_ is bitmasked by n_, or say 11 bits. That's
            // because the most significant 8 bits from the original 19 were
            // already carved off and appended to our output_ data. The
            // remaining 11 bits (n_ is 11) are the ones left in accum_.
            accum_ &= BITMASK(n_);
        }
    }
}

SerializedBloomFilter::SerializedBloomFilter(
    const std::uint32_t tweak,
    const BloomUpdateFlag update,
    const std::size_t functionCount) noexcept
    : function_count_(static_cast<std::uint32_t>(functionCount))
    , tweak_(tweak)
    , flags_(static_cast<std::uint8_t>(update))
{
    static_assert(9u == sizeof(SerializedBloomFilter));
}

SerializedBloomFilter::SerializedBloomFilter() noexcept
    : function_count_()
    , tweak_()
    , flags_()
{
    static_assert(9u == sizeof(SerializedBloomFilter));
}

auto BlockHashToFilterKey(const ReadView hash) noexcept(false) -> ReadView
{
    if (16u > hash.size()) { throw std::runtime_error("Hash too short"); }

    return ReadView{hash.data(), 16u};
}

auto DecodeSerializedCfilter(const ReadView bytes) noexcept(false)
    -> std::pair<std::uint32_t, ReadView>
{
    auto output = std::pair<std::uint32_t, ReadView>{};
    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());
    auto expectedSize = 1_uz;

    if (expectedSize > bytes.size()) {
        throw std::runtime_error("Empty input");
    }

    auto elementCount = 0_uz;
    auto csBytes = 0_uz;
    const auto haveElementCount = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, bytes.size(), elementCount, csBytes);

    if (false == haveElementCount) {
        throw std::runtime_error("Failed to decode CompactSize");
    }

    const auto dataSize = bytes.size() - (1_uz + csBytes);
    output.first = shorten(elementCount);
    output.second = {reinterpret_cast<const char*>(it), dataSize};

    return output;
}

auto DefaultFilter(const Type type) noexcept -> cfilter::Type
{
    try {

        return params::Chains().at(type).default_filter_type_;
    } catch (...) {
        return cfilter::Type::Unknown;
    }
}

auto Deserialize(const Type chain, const std::uint8_t type) noexcept
    -> cfilter::Type
{
    switch (type) {
        case 0: {
            return DefaultFilter(chain);
        }
        case 88: {
            return cfilter::Type::ES;
        }
        default: {
            return cfilter::Type::Unknown;
        }
    }
}

auto Deserialize(const api::Session& api, const ReadView in) noexcept
    -> block::Position
{
    auto output = block::Position{};

    if ((nullptr == in.data()) || (0u == in.size())) { return output; }

    if (in.size() < sizeof(output.height_)) { return output; }

    const auto size = in.size() - sizeof(output.height_);
    const auto* it = reinterpret_cast<const std::byte*>(in.data());
    std::memcpy(&output.height_, it, sizeof(output.height_));

    if (0u < size) {
        std::advance(it, sizeof(output.height_));
        auto bytes = output.hash_.WriteInto()(size);

        if (false == bytes.valid(size)) { return output; }

        std::memcpy(bytes, it, bytes);
    }

    return output;
}

auto FilterHashToHeader(
    const api::Session& api,
    const ReadView hash,
    const ReadView previous) noexcept -> cfilter::Header
{
    static const auto blank = cfilter::Header{};
    auto preimage = api.Factory().DataFromBytes(hash);
    auto output = cfilter::Header{};

    if (0u == previous.size()) {
        preimage.Concatenate(blank.data(), blank.size());
    } else {
        preimage.Concatenate(previous.data(), previous.size());
    }

    FilterHash(api, Type::Bitcoin, preimage.Bytes(), output.WriteInto());

    return output;
}

auto FilterToHash(const api::Session& api, const ReadView filter) noexcept
    -> cfilter::Hash
{
    auto output = cfilter::Hash{};
    FilterHash(api, Type::Bitcoin, filter, output.WriteInto());

    return output;
}

auto FilterToHeader(
    const api::Session& api,
    const ReadView filter,
    const ReadView previous) noexcept -> cfilter::Header
{
    return FilterHashToHeader(api, FilterToHash(api, filter).Bytes(), previous);
}

auto GetFilterParams(const cfilter::Type type) noexcept(false) -> FilterParams
{
    static const auto gcs_bits_ = UnallocatedMap<cfilter::Type, std::uint8_t>{
        {cfilter::Type::Basic_BIP158, 19u},
        {cfilter::Type::Basic_BCHVariant, 19u},
        {cfilter::Type::ES, 29u},
    };
    static const auto gcs_fp_rate_ =
        UnallocatedMap<cfilter::Type, std::uint32_t>{
            {cfilter::Type::Basic_BIP158, 784931u},
            {cfilter::Type::Basic_BCHVariant, 784931u},
            {cfilter::Type::ES, 803769307u},
        };

    return {gcs_bits_.at(type), gcs_fp_rate_.at(type)};
}

auto Serialize(const Type chain, const cfilter::Type type) noexcept(false)
    -> std::uint8_t
{
    return params::Bip158().at(chain).at(type);
}

auto Serialize(const block::Position& in) noexcept -> Space
{
    auto output = space(sizeof(in.height_) + in.hash_.size());
    auto* it = output.data();
    std::memcpy(it, &in.height_, sizeof(in.height_));
    std::advance(it, sizeof(in.height_));
    std::memcpy(it, in.hash_.data(), in.hash_.size());

    return output;
}

auto Ticker(const Type chain) noexcept -> UnallocatedCString
{
    return TickerSymbol(chain);
}
}  // namespace opentxs::blockchain::internal

namespace opentxs::blockchain::p2p
{
const UnallocatedMap<Service, UnallocatedCString> service_name_map_{
    {Service::None, "none"},
    {Service::Avalanche, "Avalanche"},
    {Service::BitcoinCash, "Bitcoin Cash"},
    {Service::Bloom, "Bloom"},
    {Service::CompactFilters, "Compact Filters"},
    {Service::Graphene, "Graphene"},
    {Service::Limited, "Limited"},
    {Service::Network, "Network"},
    {Service::Segwit2X, "Segwit2X"},
    {Service::UTXO, "GetUTXO"},
    {Service::WeakBlocks, "Weak blocks"},
    {Service::Witness, "Witness"},
    {Service::XThin, "XThin"},
    {Service::XThinner, "XThinner"},
};

auto DisplayService(const Service service) noexcept -> UnallocatedCString
{
    try {

        return service_name_map_.at(service);
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs::blockchain::p2p
#undef BITMASK
