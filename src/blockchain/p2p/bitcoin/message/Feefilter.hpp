// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "1_Internal.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
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

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Feefilter final : public implementation::Message
{
public:
    auto feeRate() const noexcept -> std::uint64_t { return fee_rate_; }

    Feefilter(
        const api::Session& api,
        const blockchain::Type network,
        const std::uint64_t fee_rate) noexcept;
    Feefilter(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const std::uint64_t fee_rate) noexcept(false);

    ~Feefilter() final = default;

private:
    const std::uint64_t fee_rate_{};

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Feefilter(const Feefilter&) = delete;
    Feefilter(Feefilter&&) = delete;
    auto operator=(const Feefilter&) -> Feefilter& = delete;
    auto operator=(Feefilter&&) -> Feefilter& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
