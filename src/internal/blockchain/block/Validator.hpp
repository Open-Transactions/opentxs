// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Block;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class Validator
{
public:
    virtual auto Validate(const protocol::bitcoin::base::block::Block& block)
        const noexcept -> bool
    {
        return true;
    }

    virtual ~Validator() = default;
};
}  // namespace opentxs::blockchain::block
