// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"  // IWYU pragma: associated

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::p2p::bitcoin::message
{
FilterPrefixBasic::FilterPrefixBasic(
    const blockchain::Type chain,
    const filter::Type type,
    const block::Hash& hash) noexcept(false)
    : type_(blockchain::internal::Serialize(chain, type))
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));

    if (hash.size() != hash_.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(hash_.data(), hash.data(), hash_.size());
}

FilterPrefixBasic::FilterPrefixBasic() noexcept
    : type_()
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));
}

auto FilterPrefixBasic::Hash() const noexcept -> filter::pHash
{
    return Data::Factory(hash_.data(), hash_.size());
}

auto FilterPrefixBasic::Type(const blockchain::Type chain) const noexcept
    -> filter::Type
{
    return blockchain::internal::Deserialize(chain, type_.value());
}

FilterPrefixChained::FilterPrefixChained(
    const blockchain::Type chain,
    const filter::Type type,
    const block::Hash& stop,
    const filter::Header& previous) noexcept(false)
    : type_(blockchain::internal::Serialize(chain, type))
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));

    if (stop.size() != hash_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    if (previous.size() != previous_.size()) {
        throw std::runtime_error("Invalid previous hash");
    }

    std::memcpy(hash_.data(), stop.data(), hash_.size());
    std::memcpy(previous_.data(), previous.data(), previous_.size());
}

FilterPrefixChained::FilterPrefixChained() noexcept
    : type_()
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));
}

auto FilterPrefixChained::Previous() const noexcept -> filter::pHeader
{
    return Data::Factory(previous_.data(), previous_.size());
}

auto FilterPrefixChained::Stop() const noexcept -> block::pHash
{
    return Data::Factory(hash_.data(), hash_.size());
}

auto FilterPrefixChained::Type(const blockchain::Type chain) const noexcept
    -> filter::Type
{
    return blockchain::internal::Deserialize(chain, type_.value());
}

FilterRequest::FilterRequest(
    const blockchain::Type chain,
    const filter::Type type,
    const block::Height start,
    const filter::Hash& stop) noexcept(false)
    : type_(blockchain::internal::Serialize(chain, type))
    , start_(static_cast<std::uint32_t>(start))
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));
    static_assert(sizeof(std::uint32_t) == sizeof(start_));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= start);

    if (stop.size() != stop_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    std::memcpy(stop_.data(), stop.data(), stop_.size());
}

FilterRequest::FilterRequest() noexcept
    : type_()
    , start_()
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));
}

auto FilterRequest::Start() const noexcept -> block::Height
{
    return start_.value();
}

auto FilterRequest::Stop() const noexcept -> filter::pHash
{
    return Data::Factory(stop_.data(), stop_.size());
}

auto FilterRequest::Type(const blockchain::Type chain) const noexcept
    -> filter::Type
{
    return blockchain::internal::Deserialize(chain, type_.value());
}

auto VerifyChecksum(
    const api::Session& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept -> bool
{
    auto checksum = Data::Factory();

    switch (params::Data::Chains().at(header.Network()).p2p_protocol_) {
        case p2p::Protocol::bitcoin: {
            if (0 == payload.size()) {
                checksum = Data::Factory("0x5df6e0e2", Data::Mode::Hex);
            } else {
                P2PMessageHash(
                    api,
                    header.Network(),
                    payload.Bytes(),
                    checksum->WriteInto());
            }
        } break;
        case p2p::Protocol::opentxs:
        case p2p::Protocol::ethereum:
        default: {
            LogError()(__func__)(": Unsupported type").Flush();
        }
    }

    return header.Checksum() == checksum;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
