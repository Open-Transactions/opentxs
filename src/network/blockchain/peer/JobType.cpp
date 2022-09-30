// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "network/blockchain/peer/JobType.hpp"  // IWYU pragma: associated

namespace opentxs::network::blockchain::internal
{
auto Peer::Imp::JobType::operator()(const std::monostate&) const noexcept
    -> std::string_view
{
    return "null job"sv;
}

auto Peer::Imp::JobType::operator()(
    const opentxs::blockchain::node::internal::HeaderJob&) const noexcept
    -> std::string_view
{
    return "headers job"sv;
}

auto Peer::Imp::JobType::operator()(
    const opentxs::blockchain::node::internal::BlockBatch&) const noexcept
    -> std::string_view
{
    return "block job"sv;
}

auto Peer::Imp::JobType::operator()(
    const opentxs::blockchain::node::CfheaderJob&) const noexcept
    -> std::string_view
{
    return "cfheader job"sv;
}

auto Peer::Imp::JobType::operator()(
    const opentxs::blockchain::node::CfilterJob&) const noexcept
    -> std::string_view
{
    return "cfilter job"sv;
}

auto Peer::Imp::JobType::get() noexcept -> const JobType&
{
    static const auto visitor = JobType{};

    return visitor;
}
}  // namespace opentxs::network::blockchain::internal
