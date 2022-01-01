// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <iosfwd>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

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
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Getblocks final : public implementation::Message
{
public:
    struct Raw {
        ProtocolVersionField version_;
        std::pmr::vector<BlockHeaderHashField> header_hashes_;
        BlockHeaderHashField stop_hash_;

        Raw(ProtocolVersionUnsigned version,
            const std::pmr::vector<OTData>& header_hashes,
            const Data& stop_hash) noexcept(false)
            : version_(version)
            , header_hashes_()
            , stop_hash_()
        {
            if (stop_hash.size() != sizeof(stop_hash_)) {
                throw std::runtime_error("Invalid stop hash");
            }

            std::memcpy(stop_hash_.data(), stop_hash.data(), stop_hash.size());

            for (const auto& hash : header_hashes) {
                BlockHeaderHashField tempHash;

                if (hash->size() != sizeof(tempHash)) {
                    throw std::runtime_error("Invalid hash");
                }

                std::memcpy(tempHash.data(), hash->data(), hash->size());
                header_hashes_.push_back(tempHash);
            }
        }
        Raw() noexcept
            : version_(2)  // TODO
            , header_hashes_()
            , stop_hash_()
        {
        }
    };

    auto getHashes() const noexcept -> const std::pmr::vector<OTData>&
    {
        return header_hashes_;
    }
    auto getStopHash() const noexcept -> OTData
    {
        return Data::Factory(stop_hash_);
    }
    auto hashCount() const noexcept -> std::size_t
    {
        return header_hashes_.size();
    }
    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;
    auto version() const noexcept -> bitcoin::ProtocolVersionUnsigned
    {
        return version_;
    }

    Getblocks(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::ProtocolVersionUnsigned version,
        const std::pmr::vector<OTData>& header_hashes,
        const Data& stop_hash) noexcept;
    Getblocks(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const bitcoin::ProtocolVersionUnsigned version,
        const std::pmr::vector<OTData>& header_hashes,
        const Data& stop_hash) noexcept(false);

    ~Getblocks() final = default;

private:
    const bitcoin::ProtocolVersionUnsigned version_;
    const std::pmr::vector<OTData> header_hashes_;
    const OTData stop_hash_;

    Getblocks(const Getblocks&) = delete;
    Getblocks(Getblocks&&) = delete;
    auto operator=(const Getblocks&) -> Getblocks& = delete;
    auto operator=(Getblocks&&) -> Getblocks& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
