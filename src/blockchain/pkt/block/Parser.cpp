// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "blockchain/pkt/block/Parser.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string_view>

#include "blockchain/pkt/block/Block.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::pkt::block
{
Parser::Parser(const api::Crypto& crypto, blockchain::Type type) noexcept
    : bitcoin::block::ParserBase(crypto, type)
    , proofs_()
    , proof_bytes_()
{
}

auto Parser::construct_block(
    std::shared_ptr<bitcoin::block::Block>& out) noexcept -> bool
{
    try {
        const auto count = transactions_.size();
        auto tx = get_transactions();
        using Type = pkt::block::Block;
        out = std::make_shared<Type>(
            chain_,
            std::move(header_),
            std::move(proofs_),
            std::move(txids_),
            std::move(tx),
            proof_bytes_,
            Type::CalculatedSize{bytes_, CompactSize{count}});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return out.operator bool();
}

auto Parser::find_payload() noexcept -> bool
{
    using opentxs::network::blockchain::bitcoin::DecodeCompactSize;

    try {
        const auto construct = constructing();
        const auto start{data_};

        while (true) {
            // TODO presumably the generation transaction contains a commitment
            // of some type for this data that should be checked
            auto* proof = [&]() -> Proof* {
                if (construct) {

                    return std::addressof(proofs_.emplace_back());
                } else {

                    return nullptr;
                }
            }();
            constexpr auto proofType = 1_uz;
            check("proof type", proofType);
            const auto type = static_cast<std::byte>(data_[0]);

            if (construct) { proof->first = type; }

            data_.remove_prefix(proofType);
            const auto proofBytes = parse_size("proof size", nullptr, nullptr);
            check("proof", proofBytes);
            auto val = data_.substr(0_uz, proofBytes);

            if (construct && (false == copy(val, writer(proof->second)))) {
                throw std::runtime_error("failed to copy proof");
            }

            data_.remove_prefix(proofBytes);
            constexpr auto terminalType = std::byte{0x0};

            if (type == terminalType) { break; }
        }

        if (construct) {
            proof_bytes_ =
                convert_to_size(std::distance(start.data(), data_.data()));
        }

        return bitcoin::block::ParserBase::find_payload();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::pkt::block
