// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "blockchain/GCS.hpp"  // IWYU pragma: associated

#include <boost/cstdint.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/GCS.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/GCS.pb.h"
#include "util/Container.hpp"

namespace be = boost::endian;
namespace bmp = boost::multiprecision;

namespace opentxs
{
constexpr auto bitmask(const std::uint64_t n) -> std::uint64_t
{
    return (1u << n) - 1u;
}

constexpr auto range(std::uint32_t N, std::uint32_t M) noexcept -> std::uint64_t
{
    return std::uint64_t{N} * std::uint64_t{M};
}
}  // namespace opentxs

namespace opentxs::factory
{
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const UnallocatedVector<OTData>& elements) noexcept
    -> std::unique_ptr<blockchain::GCS>
{
    using ReturnType = blockchain::implementation::GCS;

    try {
        auto effective = UnallocatedVector<ReadView>{};

        for (const auto& element : elements) {
            if (element->empty()) { continue; }

            effective.emplace_back(element->Bytes());
        }

        dedup(effective);

        return std::make_unique<ReturnType>(api, bits, fpRate, key, effective);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto GCS(const api::Session& api, const proto::GCS& in) noexcept
    -> std::unique_ptr<blockchain::GCS>
{
    using ReturnType = blockchain::implementation::GCS;

    try {
        return std::make_unique<ReturnType>(
            api, in.bits(), in.fprate(), in.count(), in.key(), in.filter());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto GCS(const api::Session& api, const ReadView in) noexcept
    -> std::unique_ptr<blockchain::GCS>
{
    try {
        const auto proto = proto::Factory<proto::GCS>(in.data(), in.size());

        if (false == proto::Validate(proto, VERBOSE)) {
            throw std::runtime_error{"invalid serialized gcs"};
        }

        return GCS(api, proto);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter) noexcept -> std::unique_ptr<blockchain::GCS>
{
    using ReturnType = blockchain::implementation::GCS;

    try {
        return std::make_unique<ReturnType>(
            api, bits, fpRate, filterElementCount, key, filter);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::filter::Type type,
    const ReadView key,
    const ReadView encoded) noexcept -> std::unique_ptr<blockchain::GCS>
{
    using ReturnType = blockchain::implementation::GCS;
    const auto params = blockchain::internal::GetFilterParams(type);

    try {
        const auto [elements, bytes] =
            blockchain::internal::DecodeSerializedCfilter(encoded);

        return std::make_unique<ReturnType>(
            api, params.first, params.second, elements, key, bytes);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::filter::Type type,
    const blockchain::block::Block& block) noexcept
    -> std::unique_ptr<blockchain::GCS>
{
    using ReturnType = blockchain::implementation::GCS;

    if (blockchain::filter::Type::Basic_BIP158 == type) {
        LogError()("opentxs::factory::")(__func__)(
            ": Filter can not be constructed without previous outputs")
            .Flush();

        return nullptr;
    }

    try {
        const auto params = blockchain::internal::GetFilterParams(type);
        const auto input = block.Internal().ExtractElements(type);
        auto elements = UnallocatedVector<ReadView>{};
        std::transform(
            std::begin(input), std::end(input), std::back_inserter(elements), [
            ](const auto& element) -> auto { return reader(element); });

        return std::make_unique<ReturnType>(
            api,
            params.first,
            params.second,
            blockchain::internal::BlockHashToFilterKey(block.ID().Bytes()),
            elements);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory

namespace opentxs::gcs
{
using BitReader = blockchain::internal::BitReader;
using BitWriter = blockchain::internal::BitWriter;

auto golomb_decode(const std::uint8_t P, BitReader& stream) noexcept(false)
    -> std::uint64_t;
auto golomb_encode(
    const std::uint8_t P,
    const std::uint64_t value,
    BitWriter& stream) noexcept -> void;
auto siphash(
    const api::Session& api,
    const ReadView key,
    const ReadView item) noexcept(false) -> std::uint64_t;

auto golomb_decode(const std::uint8_t P, BitReader& stream) noexcept(false)
    -> std::uint64_t
{
    auto quotient = std::uint64_t{0};

    while (1 == stream.read(1)) { quotient++; }

    auto remainder = stream.read(P);

    return std::uint64_t{(quotient << P) + remainder};
}

auto golomb_encode(
    const std::uint8_t P,
    const std::uint64_t value,
    BitWriter& stream) noexcept -> void
{
    auto remainder = std::uint64_t{value & bitmask(P)};
    auto quotient = std::uint64_t{value >> P};

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
    const Space& encoded) noexcept(false) -> UnallocatedVector<std::uint64_t>
{
    auto output = UnallocatedVector<std::uint64_t>{};
    auto stream = BitReader(encoded);
    auto last = std::uint64_t{0};

    for (auto i = std::size_t{0}; i < N; ++i) {
        auto delta = golomb_decode(P, stream);
        auto value = last + delta;
        output.emplace_back(value);
        last = value;
    }

    return output;
}

auto GolombEncode(
    const std::uint8_t P,
    const UnallocatedVector<std::uint64_t>& hashedSet) noexcept(false) -> Space
{
    auto output = Space{};
    output.reserve(hashedSet.size() * P * 2u);
    auto stream = BitWriter{output};
    auto last = std::uint64_t{0};

    for (const auto& item : hashedSet) {
        auto delta = std::uint64_t{item - last};

        if (delta != 0) { golomb_encode(P, delta, stream); }

        last = item;
    }

    stream.flush();

    return output;
}

auto siphash(
    const api::Session& api,
    const ReadView key,
    const ReadView item) noexcept(false) -> std::uint64_t
{
    if (16 != key.size()) { throw std::runtime_error("Invalid key"); }

    auto output = std::uint64_t{};
    auto writer = preallocated(sizeof(output), &output);

    if (false == api.Crypto().Hash().HMAC(
                     crypto::HashType::SipHash24, key, item, writer)) {
        throw std::runtime_error("siphash failed");
    }

    return output;
}

auto HashToRange(
    const api::Session& api,
    const ReadView key,
    const std::uint64_t range,
    const ReadView item) noexcept(false) -> std::uint64_t
{
    return ((bmp::uint128_t{siphash(api, key, item)} * bmp::uint128_t{range}) >>
            64u)
        .convert_to<std::uint64_t>();
}

auto HashedSetConstruct(
    const api::Session& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const UnallocatedVector<ReadView> items) noexcept(false)
    -> UnallocatedVector<std::uint64_t>
{
    auto output = UnallocatedVector<std::uint64_t>{};
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
}  // namespace opentxs::gcs

namespace opentxs::blockchain::implementation
{
GCS::GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const std::uint32_t filterElementCount,
    const ReadView key,
    const ReadView encoded) noexcept(false)
    : version_(1)
    , api_(api)
    , bits_(bits)
    , false_positive_rate_(fpRate)
    , count_(filterElementCount)
    , elements_()
    , compressed_(api_.Factory().Data(encoded))
    , key_(api_.Factory().Data(key))
{
    if (16u != key_->size()) {
        throw std::runtime_error(
            "Invalid key size: " + std::to_string(key_->size()));
    }
}

GCS::GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const UnallocatedVector<ReadView>& elements) noexcept(false)
    : version_(1)
    , api_(api)
    , bits_(bits)
    , false_positive_rate_(fpRate)
    , count_(static_cast<std::uint32_t>(elements.size()))
    , elements_(gcs::HashedSetConstruct(
          api_,
          key,
          static_cast<std::uint32_t>(elements.size()),
          false_positive_rate_,
          elements))
    , compressed_(
          api_.Factory().Data(reader(gcs::GolombEncode(bits_, *elements_))))
    , key_(api_.Factory().Data(key))
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // std::size_t might be 32 bit
    if (std::numeric_limits<std::uint32_t>::max() < elements.size()) {
        throw std::runtime_error(
            "Too many elements: " + std::to_string(elements.size()));
    }
#pragma GCC diagnostic pop

    if (16u != key_->size()) {
        throw std::runtime_error(
            "Invalid key size: " + std::to_string(key_->size()));
    }
}

auto GCS::Compressed() const noexcept -> Space
{
    return {
        reinterpret_cast<const std::byte*>(compressed_->data()),
        reinterpret_cast<const std::byte*>(compressed_->data()) +
            compressed_->size()};
}

auto GCS::decompress() const noexcept -> const Elements&
{
    if (false == elements_.has_value()) {
        auto& set = const_cast<std::optional<Elements>&>(elements_);
        set = gcs::GolombDecode(
            count_,
            bits_,
            Space{
                reinterpret_cast<const std::byte*>(compressed_->data()),
                reinterpret_cast<const std::byte*>(compressed_->data()) +
                    compressed_->size()});
        std::sort(set.value().begin(), set.value().end());
    }

    return elements_.value();
}

auto GCS::Encode() const noexcept -> OTData
{
    using CompactSize = network::blockchain::bitcoin::CompactSize;
    const auto bytes = CompactSize{count_}.Encode();
    auto output = Data::Factory(bytes.data(), bytes.size());
    output->Concatenate(compressed_->data(), compressed_->size());

    return output;
}

auto GCS::Hash() const noexcept -> OTData
{
    return internal::FilterToHash(api_, Encode()->Bytes());
}

auto GCS::hashed_set_construct(const UnallocatedVector<OTData>& elements)
    const noexcept -> UnallocatedVector<std::uint64_t>
{
    return hashed_set_construct(transform(elements));
}

auto GCS::hashed_set_construct(const UnallocatedVector<Space>& elements)
    const noexcept -> UnallocatedVector<std::uint64_t>
{
    return hashed_set_construct(transform(elements));
}

auto GCS::hashed_set_construct(const UnallocatedVector<ReadView>& elements)
    const noexcept -> UnallocatedVector<std::uint64_t>
{
    return gcs::HashedSetConstruct(
        api_, key_->Bytes(), count_, false_positive_rate_, elements);
}

auto GCS::hash_to_range(const ReadView in) const noexcept -> std::uint64_t
{
    return gcs::HashToRange(
        api_, key_->Bytes(), range(count_, false_positive_rate_), in);
}

auto GCS::Header(const ReadView previous) const noexcept -> OTData
{
    return internal::FilterToHeader(api_, Encode()->Bytes(), previous);
}

auto GCS::Match(const Targets& targets) const noexcept -> Matches
{
    auto output = Matches{};
    auto hashed = UnallocatedVector<std::uint64_t>{};
    auto matches = UnallocatedVector<std::uint64_t>{};
    auto map = UnallocatedMap<std::uint64_t, Targets::const_iterator>{};
    const auto& set = decompress();

    for (auto i = targets.cbegin(); i != targets.cend(); ++i) {
        const auto& hash = hashed.emplace_back(hash_to_range(*i));
        map.emplace(hash, i);
    }

    std::sort(std::begin(hashed), std::end(hashed));
    std::set_intersection(
        std::begin(hashed),
        std::end(hashed),
        std::begin(set),
        std::end(set),
        std::back_inserter(matches));
    std::transform(
        std::begin(matches),
        std::end(matches),
        std::back_inserter(output),
        [&](const auto& in) { return map.at(in); });

    return output;
}

auto GCS::Serialize(proto::GCS& output) const noexcept -> bool
{
    output.set_version(version_);
    output.set_bits(bits_);
    output.set_fprate(false_positive_rate_);
    output.set_key(key_->data(), key_->size());
    output.set_count(count_);
    output.set_filter(
        static_cast<const char*>(compressed_->data()), compressed_->size());

    return true;
}

auto GCS::Serialize(AllocateOutput out) const noexcept -> bool
{
    auto proto = proto::GCS{};

    if (false == Serialize(proto)) { return false; }

    return proto::write(proto, out);
}

auto GCS::Test(const Data& target) const noexcept -> bool
{
    return Test(target.Bytes());
}

auto GCS::Test(const ReadView target) const noexcept -> bool
{
    const auto set = hashed_set_construct({target});

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

auto GCS::Test(const UnallocatedVector<OTData>& targets) const noexcept -> bool
{
    return test(hashed_set_construct(targets));
}

auto GCS::Test(const UnallocatedVector<Space>& targets) const noexcept -> bool
{
    return test(hashed_set_construct(targets));
}

auto GCS::test(const UnallocatedVector<std::uint64_t>& targets) const noexcept
    -> bool
{
    const auto& set = decompress();
    auto alloc = alloc::BoostMonotonic{1024};
    auto matches = Vector<std::uint64_t>{&alloc};
    std::set_intersection(
        std::begin(targets),
        std::end(targets),
        std::begin(set),
        std::end(set),
        std::back_inserter(matches));

    return 0 < matches.size();
}

auto GCS::transform(const UnallocatedVector<OTData>& in) noexcept
    -> UnallocatedVector<ReadView>
{
    auto output = UnallocatedVector<ReadView>{};
    std::transform(
        std::begin(in),
        std::end(in),
        std::back_inserter(output),
        [](const auto& i) { return i->Bytes(); });

    return output;
}

auto GCS::transform(const UnallocatedVector<Space>& in) noexcept
    -> UnallocatedVector<ReadView>
{
    auto output = UnallocatedVector<ReadView>{};
    std::transform(
        std::begin(in),
        std::end(in),
        std::back_inserter(output),
        [](const auto& i) { return reader(i); });

    return output;
}
}  // namespace opentxs::blockchain::implementation
