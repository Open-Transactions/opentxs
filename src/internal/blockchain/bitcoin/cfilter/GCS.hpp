// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class GCS;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::gcs
{
using Delta = std::uint64_t;
using Element = std::uint64_t;
using Elements = Vector<Element>;
using Hash = std::uint64_t;
using Hashes = Vector<Hash>;
using Range = std::uint64_t;

auto GolombDecode(
    const std::uint32_t N,
    const std::uint8_t P,
    const Vector<std::byte>& encoded,
    alloc::Default alloc) noexcept(false) -> Elements;
auto GolombEncode(
    const std::uint8_t P,
    const Elements& hashedSet,
    alloc::Default alloc) noexcept(false) -> Vector<std::byte>;
auto HashToRange(
    const api::Session& api,
    const ReadView key,
    const Range range,
    const ReadView item) noexcept(false) -> Element;
auto HashToRange(const Range range, const Hash hash) noexcept(false) -> Element;
auto HashedSetConstruct(
    const api::Session& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const blockchain::GCS::Targets& items,
    alloc::Default alloc) noexcept(false) -> Elements;
auto Siphash(
    const api::Session& api,
    const ReadView key,
    const ReadView item) noexcept(false) -> Hash;
}  // namespace opentxs::gcs

namespace opentxs::blockchain::internal
{
class GCS
{
public:
    using PrehashedMatches = Vector<gcs::Hashes::const_iterator>;

    virtual auto Match(const gcs::Hashes& prehashed) const noexcept
        -> PrehashedMatches = 0;
    virtual auto Range() const noexcept -> gcs::Range = 0;
    virtual auto Serialize(proto::GCS& out) const noexcept -> bool = 0;
    virtual auto Test(const gcs::Hashes& targets) const noexcept -> bool = 0;

    virtual ~GCS() = default;
};
}  // namespace opentxs::blockchain::internal
