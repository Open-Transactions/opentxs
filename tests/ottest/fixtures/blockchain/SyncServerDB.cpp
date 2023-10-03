// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/SyncServerDB.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>

#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

std::unique_ptr<Listener> SyncServerDB::listener_p_{};

auto SyncServerDB::count(
    const Endpoints& endpoints,
    std::string_view value) noexcept -> std::size_t
{
    return std::count(endpoints.begin(), endpoints.end(), value);
}

void SyncServerDB::cleanup() noexcept { listener_p_.reset(); }

SyncServerDB::SyncServerDB()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , listener_([&]() -> auto& {
        if (!listener_p_) {
            listener_p_ = std::make_unique<Listener>(
                api_, api_.Endpoints().BlockchainSyncServerUpdated().data());
        }

        return *listener_p_;
    }())
{
}
}  // namespace ottest
