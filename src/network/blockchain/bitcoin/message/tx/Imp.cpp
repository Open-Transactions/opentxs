// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/tx/Imp.hpp"  // IWYU pragma: associated

#include <chrono>
#include <limits>
#include <utility>

#include "internal/blockchain/block/Parser.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::tx
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ByteArray payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , tx::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::tx,
          std::move(checksum),
          alloc)
    , payload_(std::move(payload), alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    allocator_type alloc) noexcept(false)
    : Message(api, chain, std::move(checksum), ByteArray{payload, alloc}, alloc)
{
    payload.remove_prefix(payload_.size());
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , tx::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , payload_(rhs.payload_, alloc)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    copy(payload_.Bytes(), buf, "tx");
}

auto Message::Transaction(alloc::Default alloc) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    auto out = opentxs::blockchain::block::Transaction{alloc};
    const auto result = opentxs::blockchain::block::Parser::Transaction(
        api_.Crypto(),
        chain_,
        std::numeric_limits<std::size_t>::max(),
        Clock::now(),
        payload_.Bytes(),
        out,
        {alloc, {}});  // TODO alloc::Strategy

    if (false == result) {
        LogError()()("failed to parse transaction").Flush();
    }

    return out;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::tx
