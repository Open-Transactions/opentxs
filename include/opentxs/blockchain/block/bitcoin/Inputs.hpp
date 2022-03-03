// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Inputs;
}  // namespace internal
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin
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

    virtual ~Inputs() = default;

protected:
    Inputs() noexcept = default;

private:
    Inputs(const Inputs&) = delete;
    Inputs(Inputs&&) = delete;
    auto operator=(const Inputs&) -> Inputs& = delete;
    auto operator=(Inputs&&) -> Inputs& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin
