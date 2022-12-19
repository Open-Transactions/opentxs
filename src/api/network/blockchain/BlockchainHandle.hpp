// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "opentxs/api/network/BlockchainHandle.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
class BlockchainHandle::Imp
{
public:
    std::shared_ptr<opentxs::blockchain::node::Manager> chain_;

    auto clone() const noexcept -> std::unique_ptr<Imp>
    {
        return std::make_unique<Imp>(*this);
    }

    // WARNING only use in the BlockchainHandle move constructor
    Imp() noexcept
        : chain_(nullptr)
    {
    }
    Imp(std::shared_ptr<opentxs::blockchain::node::Manager> chain) noexcept
        : chain_(std::move(chain))
    {
    }
};
}  // namespace opentxs::api::network
