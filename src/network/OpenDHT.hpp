// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <opendht.h>

#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include "internal/util/Flag.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/network/OpenDHT.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace dht
{
class DhtRunner;
}  // namespace dht

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
class DhtConfig;
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::implementation
{
class OpenDHT final : virtual public network::OpenDHT
{
public:
    auto Insert(
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        DhtDoneCallback cb = {}) const noexcept -> void final;
    auto Retrieve(
        const UnallocatedCString& key,
        DhtResultsCallback vcb,
        DhtDoneCallback dcb = {}) const noexcept -> void final;

    OpenDHT(const DhtConfig& config) noexcept;

    ~OpenDHT() final;

private:
    using Pointer = std::unique_ptr<dht::DhtRunner>;

    const DhtConfig& config_;
    Pointer node_;
    mutable OTFlag loaded_;
    mutable OTFlag ready_;
    mutable std::mutex init_;

    auto Init() const -> bool;

    OpenDHT() = delete;
    OpenDHT(const OpenDHT&) = delete;
    OpenDHT(OpenDHT&&) = delete;
    auto operator=(const OpenDHT&) -> OpenDHT& = delete;
    auto operator=(OpenDHT&&) -> OpenDHT& = delete;
};
}  // namespace opentxs::network::implementation
