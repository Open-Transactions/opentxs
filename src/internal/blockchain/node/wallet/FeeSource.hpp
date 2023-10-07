// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeSource
{
public:
    class Imp;

    auto Init() noexcept -> void;

    FeeSource(std::shared_ptr<Imp> imp) noexcept;
    FeeSource(const FeeSource&) = delete;
    FeeSource(FeeSource&&) = delete;
    auto operator=(const FeeSource&) -> FeeSource& = delete;
    auto operator=(FeeSource&&) -> FeeSource& = delete;

    ~FeeSource();

private:
    std::shared_ptr<Imp> imp_;
};
