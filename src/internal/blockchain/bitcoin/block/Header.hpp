// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/bitcoin/block/Header.hpp"

#include <memory>

#include "internal/blockchain/block/Header.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Header : virtual public blockchain::block::internal::Header
{
public:
    static auto Blank() noexcept -> Header&;

    virtual auto clone_bitcoin() const noexcept
        -> std::unique_ptr<block::Header> = 0;

    ~Header() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
