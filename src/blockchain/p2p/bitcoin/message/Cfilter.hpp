// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
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

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfilter final : public internal::Cfilter, public implementation::Message
{
public:
    using BitcoinFormat = FilterPrefixBasic;

    auto Bits() const noexcept -> std::uint8_t final { return params_.first; }
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto FPRate() const noexcept -> std::uint32_t final
    {
        return params_.second;
    }
    auto Filter() const noexcept -> ReadView final { return reader(filter_); }
    auto Hash() const noexcept -> const block::Hash& final { return hash_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    Cfilter(
        const api::Session& api,
        const blockchain::Type network,
        const filter::Type type,
        const block::Hash& hash,
        const std::uint32_t count,
        const Space& compressed) noexcept;
    Cfilter(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const block::Hash& hash,
        const std::uint32_t count,
        Space&& compressed) noexcept;

    ~Cfilter() final = default;

private:
    const filter::Type type_;
    const block::pHash hash_;
    const std::uint32_t count_;
    const Space filter_;
    const blockchain::internal::FilterParams params_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Cfilter(const Cfilter&) = delete;
    Cfilter(Cfilter&&) = delete;
    auto operator=(const Cfilter&) -> Cfilter& = delete;
    auto operator=(Cfilter&&) -> Cfilter& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
