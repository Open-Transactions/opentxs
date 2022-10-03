// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace cfilter
{
class Hash;
}  // namespace cfilter

namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

class Factory;
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
    auto begin() const noexcept -> const_iterator final { return {this, 0}; }
    auto end() const noexcept -> const_iterator final
    {
        return {this, payload_.size()};
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
        const cfilter::Header& previous,
        Vector<cfilter::Hash>&& hashes) noexcept;
    Cfheaders(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const cfilter::Type type,
        const block::Hash& stop,
        const cfilter::Header& previous,
        Vector<cfilter::Hash>&& hashes) noexcept;
    Cfheaders(const Cfheaders&) = delete;
    Cfheaders(Cfheaders&&) = delete;
    auto operator=(const Cfheaders&) -> Cfheaders& = delete;
    auto operator=(Cfheaders&&) -> Cfheaders& = delete;

    ~Cfheaders() final = default;

private:
    const cfilter::Type type_;
    const block::Hash stop_;
    const cfilter::Header previous_;
    const Vector<cfilter::Hash> payload_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
