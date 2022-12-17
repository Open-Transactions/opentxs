// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <memory>

#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::internal
{
class HeaderJob
{
public:
    class Imp;

    using Headers = Vector<block::Hash>;

    operator bool() const noexcept { return IsValid(); }

    auto IsValid() const noexcept -> bool;
    auto LastActivity() const noexcept -> std::chrono::seconds;
    auto Recent() const noexcept -> const Headers&;

    HeaderJob(std::unique_ptr<Imp> imp) noexcept;
    HeaderJob() noexcept;
    HeaderJob(const HeaderJob&) = delete;
    HeaderJob(HeaderJob&&) noexcept;
    auto operator=(const HeaderJob&) -> HeaderJob& = delete;
    auto operator=(HeaderJob&&) noexcept -> HeaderJob&;

    ~HeaderJob();

private:
    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::internal
