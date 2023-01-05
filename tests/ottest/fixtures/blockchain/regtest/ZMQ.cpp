// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/ZMQ.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/Common.hpp"

namespace ottest
{
Regtest_fixture_zmq::Regtest_fixture_zmq()
    : Regtest_fixture_normal(
          ot_,
          0,
          [] {
              using enum ot::network::blockchain::Transport;
              auto out = ot::Options{};
              out.AddOTDHTListener(ipv4, "127.0.0.1", ipv4, "0.0.0.0");

              return out;
          }(),
          {})
    , zmq_listen_address_(miner_.Factory().BlockchainAddressZMQ(
          ot::network::blockchain::Protocol::bitcoin,
          ot::network::blockchain::Transport::ipv4,
          miner_.Factory().DataFromHex("0x7f000001").Bytes(),
          test_chain_,
          {},
          {},
          miner_.Network().OTDHT().CurvePublicKey()))
{
}

auto Regtest_fixture_zmq::Connect() noexcept -> bool
{
    return Connect(zmq_listen_address_);
}
}  // namespace ottest
