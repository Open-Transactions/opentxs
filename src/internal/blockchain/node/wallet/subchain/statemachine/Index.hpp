// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class DeterministicStateData;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Index final : public Job
{
public:
    class Imp;

    static auto DeterministicFactory(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const DeterministicStateData& deterministic) noexcept -> Index;
    static auto NotificationFactory(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const PaymentCode& code) noexcept -> Index;

    auto Init() noexcept -> void final;

    Index() = delete;
    Index(const Index&) = delete;
    Index(Index&& rhs) noexcept;
    auto operator=(const Index&) -> Index& = delete;
    auto operator=(Index&&) -> Index& = delete;

    ~Index() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;

    Index(boost::shared_ptr<Imp>&& imp) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
