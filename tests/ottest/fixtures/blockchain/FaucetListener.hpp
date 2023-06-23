// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <string_view>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT FaucetListener
{
public:
    auto GetFuture() const noexcept
        -> std::shared_future<ot::blockchain::block::TransactionHash>;

    FaucetListener(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& localNym,
        std::string_view name) noexcept;

    ~FaucetListener();

private:
    class Imp;

    std::shared_ptr<Imp> imp_;
};
}  // namespace ottest
