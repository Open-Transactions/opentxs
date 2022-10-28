// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/regtest/Base.hpp"

namespace ottest
{
class OPENTXS_EXPORT Regtest_fixture_tcp : public Regtest_fixture_base
{
protected:
    using Regtest_fixture_base::Connect;
    auto Connect() noexcept -> bool final;

    Regtest_fixture_tcp();

private:
    const ot::blockchain::p2p::Address tcp_listen_address_;
};
}  // namespace ottest
