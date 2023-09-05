// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/crypto/Element.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace crypto
{
struct ElementPrivate;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainAddress;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Element : virtual public crypto::Element {
    using Txid = opentxs::blockchain::block::TransactionHash;
    using SerializedType = proto::BlockchainAddress;

    enum class Availability {
        NeverUsed,
        Reissue,
        StaleUnconfirmed,
        MetadataConflict,
        Reserved,
        Used,
    };

    virtual auto Elements() const noexcept -> UnallocatedSet<ByteArray> = 0;
    virtual auto ID() const noexcept -> const identifier::Account& = 0;
    virtual auto IncomingTransactions() const noexcept
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto IsAvailable(
        const identifier::Generic& contact,
        const std::string_view memo) const noexcept -> Availability = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Serialize() const noexcept -> SerializedType = 0;

    virtual auto Confirm(const block::TransactionHash& tx) noexcept -> bool = 0;
    virtual auto Reserve(const Time time) noexcept -> bool = 0;
    virtual auto SetContact(const identifier::Generic& id) noexcept -> void = 0;
    virtual auto SetLabel(const std::string_view label) noexcept -> void = 0;
    virtual auto SetMetadata(
        const identifier::Generic& contact,
        const std::string_view label) noexcept -> void = 0;
    virtual auto Unconfirm(
        const block::TransactionHash& tx,
        const Time time) noexcept -> bool = 0;
    virtual auto Unreserve() noexcept -> bool = 0;
};
}  // namespace opentxs::blockchain::crypto::internal
