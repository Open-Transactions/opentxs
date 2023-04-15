// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>

#include "blockchain/crypto/AccountIndex.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

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
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blockchain
{
class Wallets
{
public:
    using AccountData = crypto::Blockchain::AccountData;

    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto AccountList() const noexcept -> UnallocatedSet<identifier::Generic>;
    auto Get(const opentxs::blockchain::Type chain) noexcept
        -> opentxs::blockchain::crypto::Wallet&;
    auto LookupAccount(const identifier::Generic& id) const noexcept
        -> AccountData;

    Wallets(
        const api::Session& api,
        const api::session::Contacts& contacts,
        api::crypto::Blockchain& parent) noexcept;

private:
    const api::Session& api_;
    const api::session::Contacts& contacts_;
    api::crypto::Blockchain& parent_;
    opentxs::blockchain::crypto::AccountIndex index_;
    mutable std::mutex lock_;
    mutable bool populated_;
    mutable UnallocatedMap<
        opentxs::blockchain::Type,
        std::unique_ptr<opentxs::blockchain::crypto::Wallet>>
        lists_;

    auto get(const Lock& lock, const opentxs::blockchain::Type chain)
        const noexcept -> opentxs::blockchain::crypto::Wallet&;
    auto populate() const noexcept -> void;
    auto populate(const Lock& lock) const noexcept -> void;
};
}  // namespace opentxs::api::crypto::blockchain
