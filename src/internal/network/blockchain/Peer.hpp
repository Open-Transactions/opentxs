// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

namespace opentxs::network::blockchain::internal
{
class Peer
{
public:
    class Imp;

    auto Start() noexcept -> void;

    Peer(std::shared_ptr<Imp>&& imp) noexcept;
    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) noexcept;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;

    ~Peer();

private:
    std::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::network::blockchain::internal
