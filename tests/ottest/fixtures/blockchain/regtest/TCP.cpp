// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/TCP.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/Common.hpp"

namespace ottest
{
Regtest_fixture_tcp::Regtest_fixture_tcp()
    : Regtest_fixture_base(ot_, false, 1, ot::Options{}, ot::Options{})
    , tcp_listen_address_(miner_.Factory().BlockchainAddress(
          ot::network::blockchain::Protocol::bitcoin,
          ot::network::blockchain::Transport::ipv4,
          miner_.Factory().DataFromHex("0x7f000001"),
          18444,
          test_chain_,
          {},
          {}))
{
}

auto Regtest_fixture_tcp::Connect() noexcept -> bool
{
    return Connect(tcp_listen_address_);
}
}  // namespace ottest
