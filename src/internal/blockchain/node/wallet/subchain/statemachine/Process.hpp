// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

#include "opentxs/blockchain/Blockchain.hpp"

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
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Process
{
public:
    enum class State {
        normal,
        reorg,
    };

    auto VerifyState(const State state) const noexcept -> void;

    Process(const boost::shared_ptr<const SubchainStateData>& parent) noexcept;
    Process() = delete;
    Process(const Process&) = delete;
    Process(Process&&) = delete;
    Process& operator=(const Process&) = delete;
    Process& operator=(Process&&) = delete;

    ~Process();

private:
    class Imp;

    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet