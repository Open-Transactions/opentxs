// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Tx.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"  // IWYU pragma: keep
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PTx(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>
{
    using ReturnType = blockchain::p2p::bitcoin::message::Tx;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    try {
        return std::make_unique<ReturnType>(
            api,
            std::move(pHeader),
            ReadView{static_cast<const char*>(payload), size});
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
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
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
Tx::Tx(
    const api::Session& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    : Message(api, network, bitcoin::Command::tx)
    , payload_(api_.Factory().Data(transaction))
{
    init_hash();
}

Tx::Tx(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const ReadView transaction) noexcept(false)
    : Message(api, std::move(header))
    , payload_(api_.Factory().Data(transaction))
{
    verify_checksum();
}

auto Tx::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (false == copy(payload_->Bytes(), out)) {
            throw std::runtime_error{"failed to serialize payload"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Tx::Transaction() const noexcept
    -> std::unique_ptr<const block::bitcoin::Transaction>
{
    return api_.Factory().BitcoinTransaction(
        header_->Network(), payload_->Bytes(), false);
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
