// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include <GCS.pb.h>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/cfilter/GCSImp.hpp"
#include "blockchain/cfilter/GCSPrivate.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/cfilter/GCS.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/GCS.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const Vector<ByteArray>& elements,
    alloc::Default alloc) noexcept -> blockchain::cfilter::GCS
{
    using ReturnType = blockchain::cfilter::implementation::GCS;
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {
        auto effective = blockchain::cfilter::Targets{alloc};

        for (const auto& element : elements) {
            if (element.empty()) { continue; }

            effective.emplace_back(element.Bytes());
        }

        dedup(effective);
        const auto count = shorten(effective.size());
        auto hashed =
            gcs::HashedSetConstruct(api, key, count, fpRate, effective, alloc);
        auto compressed = gcs::GolombEncode(bits, hashed, alloc);

        return pmr::construct<ReturnType>(
            alloc,
            api,
            bits,
            fpRate,
            count,
            key,
            std::move(hashed),
            std::move(compressed));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto GCS(
    const api::Session& api,
    const proto::GCS& in,
    alloc::Default alloc) noexcept -> blockchain::cfilter::GCS
{
    using ReturnType = blockchain::cfilter::implementation::GCS;
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc,
            api,
            in.bits(),
            in.fprate(),
            in.count(),
            in.key(),
            in.filter());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto GCS(
    const api::Session& api,
    const ReadView in,
    alloc::Default alloc) noexcept -> blockchain::cfilter::GCS
{
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {
        const auto proto = proto::Factory<proto::GCS>(in.data(), in.size());

        if (false == proto::Validate(proto, VERBOSE)) {
            throw std::runtime_error{"invalid serialized gcs"};
        }

        return GCS(api, proto, alloc);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter,
    alloc::Default alloc) noexcept -> blockchain::cfilter::GCS
{
    using ReturnType = blockchain::cfilter::implementation::GCS;
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc, api, bits, fpRate, filterElementCount, key, filter);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const ReadView key,
    ReadView encoded,
    alloc::Default alloc) noexcept -> blockchain::cfilter::GCS
{
    using ReturnType = blockchain::cfilter::implementation::GCS;
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {
        const auto params = blockchain::internal::GetFilterParams(type);
        using blockchain::internal::DecodeCfilterElementCount;
        const auto elementCount = DecodeCfilterElementCount(encoded);

        return pmr::construct<ReturnType>(
            alloc,
            api,
            params.first,
            params.second,
            elementCount,
            key,
            encoded);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const blockchain::block::Block& block,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> blockchain::cfilter::GCS
{
    using ReturnType = blockchain::cfilter::implementation::GCS;
    using BlankType = blockchain::cfilter::GCSPrivate;

    try {
        if (blockchain::cfilter::Type::Basic_BIP158 == type) {

            throw std::runtime_error{
                "filter can not be constructed without previous outputs"};
        }

        const auto params = blockchain::internal::GetFilterParams(type);
        const auto input = block.Internal().ExtractElements(type, monotonic);
        auto elements = blockchain::cfilter::Targets{monotonic};
        elements.reserve(input.size());
        elements.clear();
        std::ranges::transform(
            input,
            std::back_inserter(elements),
            [](const auto& element) -> auto { return reader(element); });
        const auto count = shorten(elements.size());
        const auto key =
            blockchain::internal::BlockHashToFilterKey(block.ID().Bytes());
        auto hashed = gcs::HashedSetConstruct(
            api, key, count, params.second, elements, alloc);

        return pmr::construct<ReturnType>(
            alloc,
            api,
            params.first,
            params.second,
            count,
            key,
            std::move(hashed),
            gcs::GolombEncode(params.first, hashed, alloc));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}
}  // namespace opentxs::factory
