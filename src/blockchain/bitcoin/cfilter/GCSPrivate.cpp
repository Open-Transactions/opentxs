// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/cfilter/GCSPrivate.hpp"  // IWYU pragma: associated
#include "internal/blockchain/Blockchain.hpp"         // IWYU pragma: associated

#include <GCS.pb.h>
#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/GCS.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace be = boost::endian;
namespace bmp = boost::multiprecision;

namespace opentxs
{
constexpr auto bitmask(const std::uint64_t n) -> std::uint64_t
{
    return (1u << n) - 1u;
}

constexpr auto range(std::uint32_t N, std::uint32_t M) noexcept -> gcs::Range
{
    return gcs::Range{N} * gcs::Range{M};
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
class GCS final : public blockchain::GCSPrivate
{
public:
    auto clone(allocator_type alloc) const noexcept
        -> std::unique_ptr<GCSPrivate> final
    {
        return std::make_unique<GCS>(*this, alloc);
    }
    auto Compressed(Writer&& out) const noexcept -> bool final;
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto Encode(Writer&& out) const noexcept -> bool final;
    auto Hash() const noexcept -> cfilter::Hash final;
    auto Header(const cfilter::Header& previous) const noexcept
        -> cfilter::Header final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Match(
        const Targets& targets,
        allocator_type alloc,
        allocator_type monotonic) const noexcept -> Matches final;
    auto Match(const gcs::Hashes& prehashed, alloc::Default monotonic)
        const noexcept -> PrehashedMatches final;
    auto Range() const noexcept -> gcs::Range final;
    auto size() const noexcept -> std::size_t final
    {
        return compressed_.size();
    }
    auto Serialize(proto::GCS& out) const noexcept -> bool final;
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Test(const Data& target, allocator_type monotonic) const noexcept
        -> bool final;
    auto Test(const ReadView target, allocator_type monotonic) const noexcept
        -> bool final;
    auto Test(const Vector<ByteArray>& targets, allocator_type monotonic)
        const noexcept -> bool final;
    auto Test(const Vector<Space>& targets, allocator_type monotonic)
        const noexcept -> bool final;
    auto Test(const gcs::Hashes& targets, alloc::Default monotonic)
        const noexcept -> bool final;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        const ReadView encoded,
        allocator_type alloc)
    noexcept(false);
    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        gcs::Elements&& hashed,
        Vector<std::byte>&& compressed,
        allocator_type alloc)
    noexcept(false);
    GCS(const GCS& rhs, allocator_type alloc = {}) noexcept;
    GCS() = delete;
    GCS(GCS&&) = delete;
    auto operator=(const GCS&) -> GCS& = delete;
    auto operator=(GCS&&) -> GCS& = delete;

    ~GCS() final = default;

private:
    using Key = std::array<std::byte, 16>;

    const VersionNumber version_;
    const api::Session& api_;
    const std::uint8_t bits_;
    const std::uint32_t false_positive_rate_;
    const std::uint32_t count_;
    const Key key_;
    const Vector<std::byte> compressed_;
    mutable std::optional<gcs::Elements> elements_;

    static auto transform(
        const Vector<ByteArray>& in,
        allocator_type alloc) noexcept -> Targets;
    static auto transform(
        const Vector<Space>& in,
        allocator_type alloc) noexcept -> Targets;

    auto decompress() const noexcept -> const gcs::Elements&;
    auto hashed_set_construct(
        const Vector<ByteArray>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(
        const Vector<Space>& elements,
        allocator_type alloc) const noexcept -> gcs::Elements;
    auto hashed_set_construct(const gcs::Hashes& targets, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto hashed_set_construct(const Targets& elements, allocator_type alloc)
        const noexcept -> gcs::Elements;
    auto test(const gcs::Elements& targetHashes, allocator_type monotonic)
        const noexcept -> bool;
    auto hash_to_range(const ReadView in) const noexcept -> gcs::Range;

    GCS(const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        const ReadView key,
        Vector<std::byte>&& encoded,
        allocator_type alloc)
    noexcept(false);
    GCS(const VersionNumber version,
        const api::Session& api,
        const std::uint8_t bits,
        const std::uint32_t fpRate,
        const std::uint32_t count,
        std::optional<gcs::Elements>&& elements,
        Vector<std::byte>&& compressed,
        ReadView key,
        allocator_type alloc)
    noexcept(false);
};
}  // namespace opentxs::blockchain::implementation

namespace opentxs::factory
{
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const Vector<ByteArray>& elements,
    alloc::Default alloc) noexcept -> blockchain::GCS
{
    using ReturnType = blockchain::implementation::GCS;

    try {
        auto effective = blockchain::GCS::Targets{alloc};

        for (const auto& element : elements) {
            if (element.empty()) { continue; }

            effective.emplace_back(element.Bytes());
        }

        dedup(effective);

        const auto count = shorten(effective.size());
        auto hashed =
            gcs::HashedSetConstruct(api, key, count, fpRate, effective, alloc);
        auto compressed = gcs::GolombEncode(bits, hashed, alloc);

        return std::make_unique<ReturnType>(
                   api,
                   bits,
                   fpRate,
                   count,
                   key,
                   std::move(hashed),
                   std::move(compressed),
                   alloc)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}

auto GCS(
    const api::Session& api,
    const proto::GCS& in,
    alloc::Default alloc) noexcept -> blockchain::GCS
{
    using ReturnType = blockchain::implementation::GCS;

    try {

        return std::make_unique<ReturnType>(
                   api,
                   in.bits(),
                   in.fprate(),
                   in.count(),
                   in.key(),
                   in.filter(),
                   alloc)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}

auto GCS(
    const api::Session& api,
    const ReadView in,
    alloc::Default alloc) noexcept -> blockchain::GCS
{
    try {
        const auto proto = proto::Factory<proto::GCS>(in.data(), in.size());

        if (false == proto::Validate(proto, VERBOSE)) {
            throw std::runtime_error{"invalid serialized gcs"};
        }

        return GCS(api, proto, alloc);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}

auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter,
    alloc::Default alloc) noexcept -> blockchain::GCS
{
    using ReturnType = blockchain::implementation::GCS;

    try {

        return std::make_unique<ReturnType>(
                   api, bits, fpRate, filterElementCount, key, filter, alloc)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const ReadView key,
    ReadView encoded,
    alloc::Default alloc) noexcept -> blockchain::GCS
{
    using ReturnType = blockchain::implementation::GCS;
    const auto params = blockchain::internal::GetFilterParams(type);

    try {
        using blockchain::internal::DecodeCfilterElementCount;
        const auto elementCount = DecodeCfilterElementCount(encoded);

        return std::make_unique<ReturnType>(
                   api,
                   params.first,
                   params.second,
                   elementCount,
                   key,
                   encoded,
                   alloc)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const blockchain::block::Block& block,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> blockchain::GCS
{
    using ReturnType = blockchain::implementation::GCS;

    if (blockchain::cfilter::Type::Basic_BIP158 == type) {
        LogError()("opentxs::factory::")(__func__)(
            ": Filter can not be constructed without previous outputs")
            .Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }

    try {
        const auto params = blockchain::internal::GetFilterParams(type);
        const auto input = block.Internal().ExtractElements(type, monotonic);
        auto elements = blockchain::GCS::Targets{monotonic};
        elements.reserve(input.size());
        elements.clear();
        std::transform(
            std::begin(input), std::end(input), std::back_inserter(elements), [
            ](const auto& element) -> auto{ return reader(element); });
        const auto count = shorten(elements.size());
        const auto key =
            blockchain::internal::BlockHashToFilterKey(block.ID().Bytes());
        auto hashed = gcs::HashedSetConstruct(
            api, key, count, params.second, elements, alloc);

        return std::make_unique<ReturnType>(
                   api,
                   params.first,
                   params.second,
                   count,
                   key,
                   std::move(hashed),
                   gcs::GolombEncode(params.first, hashed, alloc),
                   alloc)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::GCSPrivate>(alloc).release();
    }
}
}  // namespace opentxs::factory

namespace opentxs::gcs
{
using BitReader = blockchain::internal::BitReader;
using BitWriter = blockchain::internal::BitWriter;

static auto golomb_decode(const std::uint8_t P, BitReader& stream) noexcept(
    false) -> Delta
{
    auto quotient = Delta{0};

    while (1 == stream.read(1)) { quotient++; }

    auto remainder = stream.read(P);

    return Delta{(quotient << P) + remainder};
}

static auto golomb_encode(
    const std::uint8_t P,
    const Delta value,
    BitWriter& stream) noexcept -> void
{
    auto remainder = Delta{value & bitmask(P)};
    auto quotient = Delta{value >> P};

    while (quotient > 0) {
        stream.write(1, 1);
        --quotient;
    }

    stream.write(1, 0);
    stream.write(P, remainder);
}

auto GolombDecode(
    const std::uint32_t N,
    const std::uint8_t P,
    const Vector<std::byte>& encoded,
    alloc::Default alloc) noexcept(false) -> Elements
{
    auto output = Elements{alloc};
    auto stream = BitReader{encoded};
    auto last = Element{0};

    for (auto i = 0_uz; i < N; ++i) {
        auto delta = golomb_decode(P, stream);
        auto value = last + delta;
        output.emplace_back(value);
        last = value;
    }

    return output;
}

auto GolombEncode(
    const std::uint8_t P,
    const Elements& hashedSet,
    alloc::Default alloc) noexcept(false) -> Vector<std::byte>
{
    auto output = Vector<std::byte>{alloc};
    output.reserve(hashedSet.size() * P * 2u);
    auto stream = BitWriter{output};
    auto last = Element{0};

    for (const auto& item : hashedSet) {
        auto delta = Delta{item - last};

        if (delta != 0) { golomb_encode(P, delta, stream); }

        last = item;
    }

    stream.flush();

    return output;
}

auto HashToRange(
    const api::Session& api,
    const ReadView key,
    const Range range,
    const ReadView item) noexcept(false) -> Element
{
    return HashToRange(range, Siphash(api, key, item));
}

auto HashToRange(const Range range, const Hash hash) noexcept(false) -> Element
{
    return ((bmp::uint128_t{hash} * bmp::uint128_t{range}) >> 64u)
        .convert_to<Element>();
}

auto HashedSetConstruct(
    const api::Session& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const blockchain::GCS::Targets& items,
    alloc::Default alloc) noexcept(false) -> Elements
{
    auto output = Elements{alloc};
    std::transform(
        std::begin(items),
        std::end(items),
        std::back_inserter(output),
        [&](const auto& item) {
            return HashToRange(api, key, range(N, M), item);
        });
    std::sort(output.begin(), output.end());

    return output;
}

auto Siphash(
    const api::Session& api,
    const ReadView key,
    const ReadView item) noexcept(false) -> Hash
{
    if (16 != key.size()) { throw std::runtime_error("Invalid key"); }

    auto output = Hash{};
    auto writer = preallocated(sizeof(output), &output);

    if (false ==
        api.Crypto().Hash().HMAC(
            crypto::HashType::SipHash24, key, item, std::move(writer))) {
        throw std::runtime_error("siphash failed");
    }

    return output;
}
}  // namespace opentxs::gcs

namespace opentxs::blockchain::implementation
{
GCS::GCS(
    const VersionNumber version,
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const std::uint32_t count,
    std::optional<gcs::Elements>&& elements,
    Vector<std::byte>&& compressed,
    ReadView key,
    allocator_type alloc) noexcept(false)
    : GCSPrivate(alloc)
    , version_(version)
    , api_(api)
    , bits_(bits)
    , false_positive_rate_(fpRate)
    , count_(count)
    , key_()
    , compressed_(std::move(compressed), alloc)
    , elements_(std::move(elements))
{
    static_assert(16u == sizeof(key_));

    if (false == copy(key, writer(const_cast<Key&>(key_)))) {
        throw std::runtime_error(
            "Invalid key size: " + std::to_string(key.size()));
    }
}

GCS::GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const std::uint32_t count,
    const ReadView key,
    const ReadView encoded,
    allocator_type alloc) noexcept(false)
    : GCS(
          1,
          api,
          bits,
          fpRate,
          count,
          std::nullopt,
          [&] {
              auto out = Vector<std::byte>{alloc};
              copy(encoded, writer(out));

              return out;
          }(),
          key,
          alloc)
{
}

GCS::GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const std::uint32_t count,
    const ReadView key,
    Vector<std::byte>&& encoded,
    allocator_type alloc) noexcept(false)
    : GCS(1,
          api,
          bits,
          fpRate,
          count,
          std::nullopt,
          std::move(encoded),
          key,
          alloc)
{
}

GCS::GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const std::uint32_t count,
    const ReadView key,
    gcs::Elements&& hashed,
    Vector<std::byte>&& compressed,
    allocator_type alloc) noexcept(false)
    : GCS(1,
          api,
          bits,
          fpRate,
          count,
          std::move(hashed),
          std::move(compressed),
          key,
          alloc)
{
}

GCS::GCS(const GCS& rhs, allocator_type alloc) noexcept
    : GCS(
          rhs.version_,
          rhs.api_,
          rhs.bits_,
          rhs.false_positive_rate_,
          rhs.count_,
          [&]() -> std::optional<gcs::Elements> {
              if (rhs.elements_.has_value()) {

                  return gcs::Elements{rhs.elements_.value(), alloc};
              } else {

                  return std::nullopt;
              }
          }(),
          Vector<std::byte>{rhs.compressed_, alloc},
          reader(rhs.key_),
          alloc)
{
}

auto GCS::Compressed(Writer&& out) const noexcept -> bool
{
    return copy(reader(compressed_), std::move(out));
}

auto GCS::decompress() const noexcept -> const gcs::Elements&
{
    if (false == elements_.has_value()) {
        auto& set = elements_;
        set = gcs::GolombDecode(count_, bits_, compressed_, alloc_);
        std::sort(set.value().begin(), set.value().end());
    }

    return elements_.value();
}

auto GCS::Encode(Writer&& cb) const noexcept -> bool
{
    using CompactSize = network::blockchain::bitcoin::CompactSize;
    const auto bytes = CompactSize{count_}.Encode();
    const auto max = std::numeric_limits<std::size_t>::max() - bytes.size();

    if (max < compressed_.size()) {
        LogError()(OT_PRETTY_CLASS())("filter is too large to encode").Flush();

        return false;
    }

    const auto target = bytes.size() + compressed_.size();
    auto out = cb.Reserve(target);

    if (false == out.IsValid(target)) {
        LogError()(OT_PRETTY_CLASS())("failed to allocate space for output")
            .Flush();

        return false;
    }

    auto* i = out.as<std::byte>();
    std::memcpy(i, bytes.data(), bytes.size());
    std::advance(i, bytes.size());

    if (0_uz < compressed_.size()) {
        std::memcpy(i, compressed_.data(), compressed_.size());
        std::advance(i, compressed_.size());
    }

    return true;
}

auto GCS::Hash() const noexcept -> cfilter::Hash
{
    auto preimage = Vector<std::byte>{get_allocator()};
    Encode(writer(preimage));

    return internal::FilterToHash(api_, reader(preimage));
}

auto GCS::hashed_set_construct(
    const Vector<ByteArray>& elements,
    allocator_type alloc) const noexcept -> gcs::Elements
{
    return hashed_set_construct(transform(elements, alloc), alloc);
}

auto GCS::hashed_set_construct(
    const Vector<Space>& elements,
    allocator_type alloc) const noexcept -> gcs::Elements
{
    return hashed_set_construct(transform(elements, alloc), alloc);
}

auto GCS::hashed_set_construct(const gcs::Hashes& targets, allocator_type alloc)
    const noexcept -> gcs::Elements
{
    auto out = gcs::Elements{alloc};
    out.reserve(targets.size());
    const auto range = Range();
    std::transform(
        targets.begin(),
        targets.end(),
        std::back_inserter(out),
        [&](const auto& hash) { return gcs::HashToRange(range, hash); });

    return out;
}

auto GCS::hashed_set_construct(const Targets& elements, allocator_type alloc)
    const noexcept -> gcs::Elements
{
    return gcs::HashedSetConstruct(
        api_, reader(key_), count_, false_positive_rate_, elements, alloc);
}

auto GCS::hash_to_range(const ReadView in) const noexcept -> gcs::Range
{
    return gcs::HashToRange(api_, reader(key_), Range(), in);
}

auto GCS::Header(const cfilter::Header& previous) const noexcept
    -> cfilter::Header
{
    auto preimage = Vector<std::byte>{get_allocator()};
    Encode(writer(preimage));

    return internal::FilterToHeader(api_, reader(preimage), previous.Bytes());
}

auto GCS::Match(
    const Targets& targets,
    allocator_type alloc,
    allocator_type monotonic) const noexcept -> Matches
{
    static constexpr auto reserveMatches = 16_uz;
    auto output = Matches{alloc};
    output.reserve(reserveMatches);
    using Map = opentxs::Map<gcs::Element, Matches>;
    auto hashed = gcs::Elements{monotonic};
    hashed.reserve(targets.size());
    hashed.clear();
    auto matches = gcs::Elements{monotonic};
    matches.reserve(reserveMatches);
    matches.clear();
    auto map = Map{monotonic};

    for (auto i = targets.cbegin(); i != targets.cend(); ++i) {
        const auto& hash = hashed.emplace_back(hash_to_range(*i));
        map[hash].emplace_back(i);
    }

    dedup(hashed);
    const auto& set = decompress();
    std::set_intersection(
        std::begin(hashed),
        std::end(hashed),
        std::begin(set),
        std::end(set),
        std::back_inserter(matches));

    for (const auto& match : matches) {
        auto& values = map.at(match);
        std::copy(values.begin(), values.end(), std::back_inserter(output));
    }

    return output;
}

auto GCS::Match(const gcs::Hashes& prehashed, alloc::Default monotonic)
    const noexcept -> PrehashedMatches
{
    static constexpr auto reserveMatches = 16_uz;
    auto output = PrehashedMatches{prehashed.get_allocator()};
    output.reserve(reserveMatches);
    using Map = opentxs::Map<gcs::Element, PrehashedMatches>;
    auto hashed = gcs::Elements{monotonic};
    hashed.reserve(prehashed.size());
    hashed.clear();
    auto matches = gcs::Elements{monotonic};
    matches.reserve(reserveMatches);
    matches.clear();
    auto map = Map{monotonic};
    const auto range = Range();

    for (auto i = prehashed.cbegin(); i != prehashed.cend(); ++i) {
        const auto& hash = hashed.emplace_back(gcs::HashToRange(range, *i));
        map[hash].emplace_back(i);
    }

    dedup(hashed);
    const auto& set = decompress();
    std::set_intersection(
        std::begin(hashed),
        std::end(hashed),
        std::begin(set),
        std::end(set),
        std::back_inserter(matches));

    for (const auto& match : matches) {
        auto& values = map.at(match);
        std::copy(values.begin(), values.end(), std::back_inserter(output));
    }

    return output;
}

auto GCS::Range() const noexcept -> gcs::Range
{
    return range(count_, false_positive_rate_);
}

auto GCS::Serialize(proto::GCS& output) const noexcept -> bool
{
    output.set_version(version_);
    output.set_bits(bits_);
    output.set_fprate(false_positive_rate_);
    output.set_key(reinterpret_cast<const char*>(key_.data()), key_.size());
    output.set_count(count_);
    output.set_filter(
        reinterpret_cast<const char*>(compressed_.data()), compressed_.size());

    return true;
}

auto GCS::Serialize(Writer&& out) const noexcept -> bool
{
    auto proto = proto::GCS{};

    if (false == Serialize(proto)) { return false; }

    return proto::write(proto, std::move(out));
}

auto GCS::Test(const Data& target, allocator_type monotonic) const noexcept
    -> bool
{
    return Test(target.Bytes(), monotonic);
}

auto GCS::Test(const ReadView target, allocator_type monotonic) const noexcept
    -> bool
{
    const auto input = [&] {
        auto out = Targets{monotonic};
        out.clear();
        out.emplace_back(target);

        return out;
    }();
    const auto set = hashed_set_construct(input, monotonic);

    OT_ASSERT(1 == set.size());

    const auto& hash = set.front();

    for (const auto& element : decompress()) {
        if (element == hash) {

            return true;
        } else if (element > hash) {

            return false;
        }
    }

    return false;
}

auto GCS::Test(const Vector<ByteArray>& targets, allocator_type monotonic)
    const noexcept -> bool
{
    return test(hashed_set_construct(targets, monotonic), monotonic);
}

auto GCS::Test(const Vector<Space>& targets, allocator_type monotonic)
    const noexcept -> bool
{
    return test(hashed_set_construct(targets, monotonic), monotonic);
}

auto GCS::Test(const gcs::Hashes& targets, alloc::Default monotonic)
    const noexcept -> bool
{
    return test(hashed_set_construct(targets, monotonic), monotonic);
}

auto GCS::test(const gcs::Elements& targets, allocator_type monotonic)
    const noexcept -> bool
{
    const auto& set = decompress();
    auto matches = Vector<gcs::Element>{monotonic};
    matches.reserve(std::min(targets.size(), set.size()));
    matches.clear();
    std::set_intersection(
        std::begin(targets),
        std::end(targets),
        std::begin(set),
        std::end(set),
        std::back_inserter(matches));

    return false == matches.empty();
}

auto GCS::transform(const Vector<ByteArray>& in, allocator_type alloc) noexcept
    -> Targets
{
    auto output = Targets{alloc};
    output.reserve(in.size());
    output.clear();
    std::transform(
        std::begin(in),
        std::end(in),
        std::back_inserter(output),
        [](const auto& i) { return i.Bytes(); });

    return output;
}

auto GCS::transform(const Vector<Space>& in, allocator_type alloc) noexcept
    -> Targets
{
    auto output = Targets{alloc};
    output.reserve(in.size());
    output.clear();
    std::transform(
        std::begin(in),
        std::end(in),
        std::back_inserter(output),
        [](const auto& i) { return reader(i); });

    return output;
}
}  // namespace opentxs::blockchain::implementation
