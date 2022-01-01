// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Blockchain.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session
}  // namespace api

namespace contact
{
class Contact;
}  // namespace contact

namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::crypto::internal
{
class Blockchain : virtual public api::crypto::Blockchain
{
public:
    virtual auto Contacts() const noexcept -> const api::session::Contacts& = 0;
    virtual auto KeyEndpoint() const noexcept -> const std::string& = 0;
    virtual auto KeyGenerated(const Chain chain) const noexcept -> void = 0;
    auto Internal() const noexcept -> const Blockchain& final { return *this; }
    virtual auto NewNym(const identifier::Nym& id) const noexcept -> void = 0;
    using crypto::Blockchain::NewPaymentCodeSubaccount;
    virtual auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier = 0;
    using crypto::Blockchain::PaymentCodeSubaccount;
    virtual auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto ProcessContact(const contact::Contact& contact) const noexcept
        -> bool = 0;
    virtual auto ProcessMergedContact(
        const contact::Contact& parent,
        const contact::Contact& child) const noexcept -> bool = 0;
    virtual auto ProcessTransaction(
        const Chain chain,
        const opentxs::blockchain::block::bitcoin::Transaction& transaction,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    /// Throws std::runtime_error if type is invalid
    virtual auto PubkeyHash(
        const opentxs::blockchain::Type chain,
        const Data& pubkey) const noexcept(false) -> OTData = 0;
    virtual auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const opentxs::blockchain::crypto::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void = 0;
    virtual auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void = 0;
    virtual auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void = 0;
    virtual auto UpdateElement(
        std::pmr::vector<ReadView>& pubkeyHashes) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> Blockchain& final { return *this; }

    ~Blockchain() override = default;
};
}  // namespace opentxs::api::crypto::internal
