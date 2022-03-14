// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
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

class Factory;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfheaders final : public internal::Cfheaders,
                        public implementation::Message
{
public:
    using BitcoinFormat = FilterPrefixChained;

    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, payload_.size());
    }
    auto Previous() const noexcept -> const cfilter::Header& final
    {
        return previous_;
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }
    auto Stop() const noexcept -> const block::Hash& final { return stop_; }
    auto Type() const noexcept -> cfilter::Type final { return type_; }

    Cfheaders(
        const api::Session& api,
        const blockchain::Type network,
        const cfilter::Type type,
        const block::Hash& stop,
        const ReadView previousHeader,
        const UnallocatedVector<cfilter::pHash>& headers) noexcept(false);
    Cfheaders(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const cfilter::Type type,
        const block::Hash& stop,
        const cfilter::Header& previous,
        const UnallocatedVector<cfilter::pHash>& headers) noexcept(false);

    ~Cfheaders() final = default;

private:
    const cfilter::Type type_;
    const block::pHash stop_;
    const cfilter::pHeader previous_;
    const UnallocatedVector<cfilter::pHash> payload_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Cfheaders(const Cfheaders&) = delete;
    Cfheaders(Cfheaders&&) = delete;
    auto operator=(const Cfheaders&) -> Cfheaders& = delete;
    auto operator=(Cfheaders&&) -> Cfheaders& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
