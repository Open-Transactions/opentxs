// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Inputs;
}  // namespace internal

class Input;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Inputs
{
public:
    using value_type = Input;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Inputs, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Inputs& = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Inputs& = 0;

    Inputs(const Inputs&) = delete;
    Inputs(Inputs&&) = delete;
    auto operator=(const Inputs&) -> Inputs& = delete;
    auto operator=(Inputs&&) -> Inputs& = delete;

    virtual ~Inputs() = default;

protected:
    Inputs() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block
