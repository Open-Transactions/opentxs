// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/pkt/block/Parser.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/pkt/block/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::pkt::block
{
Parser::Parser(
    const api::Crypto& crypto,
    blockchain::Type type,
    alloc::Default alloc) noexcept
    : bitcoin::block::ParserBase(crypto, type, alloc)
    , proofs_()
    , proof_bytes_()
{
}

auto Parser::construct_block(blockchain::block::Block& out) noexcept -> bool
{
    const auto count = transactions_.size();
    out = {factory::PktBlock(
        chain_,
        std::move(header_),
        std::move(proofs_),
        make_index(txids_),
        make_index(wtxids_),
        get_transactions(),
        std::optional<std::size_t>{proof_bytes_},
        CalculatedSize{bytes_, CompactSize{count}},
        alloc_)};

    return out.IsValid();
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
            const auto& encodedType = data_[0];
            static_assert(sizeof(encodedType) == sizeof(ProofType));
            const auto type = static_cast<ProofType>(data_[0]);

            if (construct) { proof->first = type; }

            data_.remove_prefix(proofType);
            const auto proofBytes = parse_size("proof size");
            check("proof", proofBytes);
            auto val = data_.substr(0_uz, proofBytes);

            if (construct && (false == copy(val, proof->second.WriteInto()))) {
                throw std::runtime_error("failed to copy proof");
            }

            data_.remove_prefix(proofBytes);

            if (type == terminal_proof_) { break; }
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
