// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

#include "opentxs/blockchain/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blockchain
{
class BalanceOracle
{
public:
    using Balance = opentxs::blockchain::Balance;
    using Chain = opentxs::blockchain::Type;

    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void;

    BalanceOracle(const api::Session& api) noexcept;
    BalanceOracle() = delete;
    BalanceOracle(const BalanceOracle&) = delete;
    BalanceOracle(BalanceOracle&&) = delete;

    ~BalanceOracle();

private:
    class Imp;

    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::api::crypto::blockchain
