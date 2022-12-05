// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/p2p/message/Tx.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PTx(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>
{
    try {
        using ReturnType = blockchain::p2p::bitcoin::message::Tx;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto data = extract_prefix(bytes, bytes.size(), "tx");
        check_finished(bytes);

        return std::make_unique<ReturnType>(api, std::move(pHeader), data);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PTx(
    const api::Session& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>
{
    using ReturnType = blockchain::p2p::bitcoin::message::Tx;

    return std::make_unique<ReturnType>(api, network, transaction);
}

auto BitcoinP2PTxTemp(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept -> blockchain::p2p::bitcoin::message::internal::Tx*
{
    return BitcoinP2PTx(api, std::move(pHeader), version, bytes).release();
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
Tx::Tx(
    const api::Session& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    : Message(api, network, bitcoin::Command::tx)
    , payload_(api_.Factory().DataFromBytes(transaction))
{
    init_hash();
}

Tx::Tx(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const ReadView transaction) noexcept(false)
    : Message(api, std::move(header))
    , payload_(api_.Factory().DataFromBytes(transaction))
{
    verify_checksum();
}

auto Tx::payload(Writer&& out) const noexcept -> bool
{
    try {
        if (false == copy(payload_.Bytes(), std::move(out))) {
            throw std::runtime_error{"failed to serialize payload"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Tx::Transaction() const noexcept
    -> std::unique_ptr<const blockchain::bitcoin::block::Transaction>
{
    return api_.Factory().BitcoinTransaction(
        header_->Network(), payload_.Bytes(), false);
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
