// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"

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
class OPENTXS_EXPORT BlockchainHandle
{
public:
    class Imp;

    operator bool() const noexcept;
    operator const blockchain::node::Manager&() const noexcept;

    auto get() const noexcept -> const blockchain::node::Manager&;
    auto IsValid() const noexcept -> bool;

    auto swap(BlockchainHandle& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT BlockchainHandle(Imp* imp) noexcept;
    BlockchainHandle() = delete;
    BlockchainHandle(const BlockchainHandle&) noexcept;
    BlockchainHandle(BlockchainHandle&&) noexcept;
    auto operator=(const BlockchainHandle&) noexcept -> BlockchainHandle&;
    auto operator=(BlockchainHandle&&) noexcept -> BlockchainHandle&;

    ~BlockchainHandle();

private:
    Imp* imp_;
};

auto swap(BlockchainHandle& lhs, BlockchainHandle& rhs) noexcept -> void;
}  // namespace opentxs::api::network
