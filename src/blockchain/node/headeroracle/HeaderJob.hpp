// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class HeaderJob::Imp
{
public:
    const bool valid_;
    const sTime start_;
    const Vector<block::Hash> previous_;
    std::optional<network::zeromq::socket::Raw> to_parent_;

    Imp() noexcept;
    Imp(bool valid,
        Vector<block::Hash>&& previous,
        const api::Session* api,
        std::string_view endpoint) noexcept;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) noexcept -> Imp& = delete;

    ~Imp();
};
}  // namespace opentxs::blockchain::node::internal
