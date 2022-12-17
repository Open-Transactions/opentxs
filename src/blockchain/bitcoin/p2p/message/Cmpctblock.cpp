// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/p2p/message/Cmpctblock.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::Cmpctblock;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto data = ByteArray{extract_prefix(bytes, bytes.size(), "data")};
        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), std::move(data));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
    -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    return new ReturnType(api, network, raw_cmpctblock);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Cmpctblock::Cmpctblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock) noexcept
    : Message(api, network, bitcoin::Command::cmpctblock)
    , raw_cmpctblock_(raw_cmpctblock)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Cmpctblock::Cmpctblock(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& raw_cmpctblock) noexcept(false)
    : Message(api, std::move(header))
    , raw_cmpctblock_(raw_cmpctblock)
{
    verify_checksum();
}

auto Cmpctblock::payload(Writer&& out) const noexcept -> bool
{
    try {

        return copy(raw_cmpctblock_.Bytes(), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
