// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Getcfcheckpt final : public internal::Getcfcheckpt,
                           public implementation::Message
{
public:
    using BitcoinFormat = FilterPrefixBasic;

    auto Stop() const noexcept -> const block::Hash& final { return stop_; }
    auto Type() const noexcept -> cfilter::Type final { return type_; }

    Getcfcheckpt(
        const api::Session& api,
        const blockchain::Type network,
        const cfilter::Type type,
        const block::Hash& stop) noexcept;
    Getcfcheckpt(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const cfilter::Type type,
        block::Hash&& stop) noexcept;
    Getcfcheckpt(const Getcfcheckpt&) = delete;
    Getcfcheckpt(Getcfcheckpt&&) = delete;
    auto operator=(const Getcfcheckpt&) -> Getcfcheckpt& = delete;
    auto operator=(Getcfcheckpt&&) -> Getcfcheckpt& = delete;

    ~Getcfcheckpt() final = default;

private:
    const cfilter::Type type_;
    const block::Hash stop_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
