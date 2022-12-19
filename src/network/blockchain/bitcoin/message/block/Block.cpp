// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Block.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/block/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Block::Block(allocator_type alloc) noexcept
    : Block(MessagePrivate::Blank(alloc))
{
}

Block::Block(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Block::Block(Block&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Block::Block(Block&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}

auto Block::get() const noexcept -> ReadView
{
    return imp_->asBlockPrivate()->get();
}

auto Block::operator=(const Block& rhs) noexcept -> Block&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Block::operator=(Block&& rhs) noexcept -> Block&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Block::~Block() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal

/*
// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/Block.hpp"  // IWYU pragma:
associated

#include <stdexcept>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"
#include "internal/network/blockchain/bitcoin/message/Header.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/blockchain/bitcoin/message/Message.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PBlock(
    const api::Session& api,
    std::unique_ptr<network::blockchain::bitcoin::message::internal::Header>
        pHeader,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    ReadView bytes) -> network::blockchain::bitcoin::message::internal::Block*
{
    using namespace network::blockchain::bitcoin::message;

    try {
        using ReturnType = implementation::Block;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto block = extract_prefix(bytes, bytes.size(), "");
        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), ByteArray{block});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PBlock(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const Data& raw_block)
    -> network::blockchain::bitcoin::message::internal::Block*
{
    using namespace network::blockchain::bitcoin::message;
    using ReturnType = implementation::Block;

    return new ReturnType(api, network, raw_block);
}
}  // namespace opentxs::factory

namespace opentxs::network::blockchain::bitcoin::message::implementation
{
Block::Block(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const Data& block) noexcept
    : Message(api, network, Command::block)
    , payload_(block)
{
    init_hash();
}

Block::Block(
    const api::Session& api,
    internal::Header header,
    const Data& block) noexcept
    : Message(api, std::move(header))
    , payload_(block)
{
}

auto Block::payload(Writer&& out) const noexcept -> bool
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
}  // namespace opentxs::network::blockchain::bitcoin::message::implementation
*/
