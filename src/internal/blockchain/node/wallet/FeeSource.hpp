// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocated.hpp"

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
class FeeSource;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeSource final : public Allocated
{
public:
    class Imp;

    auto Shutdown() noexcept -> void;
    auto get_allocator() const noexcept -> allocator_type final;

    FeeSource(Imp* imp) noexcept;
    FeeSource(FeeSource&& rhs) noexcept;
    FeeSource(FeeSource&& rhs, allocator_type alloc) noexcept;
    FeeSource(const FeeSource&) = delete;
    auto operator=(const FeeSource&) -> FeeSource& = delete;
    auto operator=(FeeSource&&) -> FeeSource& = delete;

    ~FeeSource() final;

private:
    Imp* imp_;
};
