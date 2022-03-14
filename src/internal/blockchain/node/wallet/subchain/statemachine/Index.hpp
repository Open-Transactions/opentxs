// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

#include "opentxs/blockchain/block/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Index
{
public:
    class Imp;

    enum class State {
        normal,
        reorg,
        shutdown,
    };

    static auto DeterministicFactory(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const DeterministicStateData& deterministic) noexcept -> Index;
    static auto NotificationFactory(
        const boost::shared_ptr<const SubchainStateData>& parent,
        const PaymentCode& code) noexcept -> Index;

    auto VerifyState(const State state) const noexcept -> void;

    auto ProcessReorg(const block::Position& parent) noexcept -> void;

    Index() = delete;
    Index(const Index&) = delete;
    Index(Index&&) noexcept;
    Index& operator=(const Index&) = delete;
    Index& operator=(Index&&) = delete;

    ~Index();

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;

    Index(boost::shared_ptr<Imp>&& imp) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
