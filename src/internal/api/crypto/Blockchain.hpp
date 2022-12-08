// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto

class ByteArray;
class Contact;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::internal
{
class Blockchain : virtual public api::crypto::Blockchain
{
public:
    virtual auto BalanceOracleEndpoint() const noexcept -> std::string_view = 0;
    virtual auto Contacts() const noexcept -> const api::session::Contacts& = 0;
    virtual auto KeyEndpoint() const noexcept -> std::string_view = 0;
    virtual auto KeyGenerated(
        const Chain chain,
        const identifier::Nym& account,
        const identifier::Generic& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void = 0;
    virtual auto IndexItem(const ReadView bytes) const noexcept
        -> opentxs::blockchain::block::ElementHash = 0;
    auto Internal() const noexcept -> const Blockchain& final { return *this; }
    virtual auto NewNym(const identifier::Nym& id) const noexcept -> void = 0;
    using crypto::Blockchain::NewPaymentCodeSubaccount;
    virtual auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> identifier::Generic = 0;
    using crypto::Blockchain::PaymentCodeSubaccount;
    virtual auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto ProcessContact(const Contact& contact) const noexcept
        -> bool = 0;
    virtual auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child) const noexcept -> bool = 0;
    virtual auto ProcessTransactions(
        const Chain chain,
        Set<opentxs::blockchain::block::Transaction>&& transactions,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    /// Throws std::runtime_error if type is invalid
    virtual auto PubkeyHash(
        const opentxs::blockchain::Type chain,
        const Data& pubkey) const noexcept(false) -> ByteArray = 0;
    virtual auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const identifier::Generic& account,
        const opentxs::blockchain::crypto::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void = 0;
    virtual auto UpdateElement(
        UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> Blockchain& final { return *this; }
    virtual auto Start(std::shared_ptr<const api::Session> api) noexcept
        -> void = 0;

    ~Blockchain() override = default;
};
}  // namespace opentxs::api::crypto::internal
