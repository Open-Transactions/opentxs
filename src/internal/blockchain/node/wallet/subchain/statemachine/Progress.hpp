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
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Progress final : public Job
{
public:
    auto Init() noexcept -> void final;

    Progress(const std::shared_ptr<const SubchainStateData>& parent) noexcept;
    Progress() = delete;
    Progress(const Progress&) = delete;
    Progress(Progress&&) = delete;
    auto operator=(const Progress&) -> Progress& = delete;
    auto operator=(Progress&&) -> Progress& = delete;

    ~Progress() final;

private:
    class Imp;

    std::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
