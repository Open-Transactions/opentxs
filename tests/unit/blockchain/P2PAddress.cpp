// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

#include "BoostAsio.hpp"
#include "ottest/fixtures/blockchain/P2PAddress.hpp"

namespace ottest
{
using namespace std::literals;
using namespace boost::asio;
using enum opentxs::network::blockchain::Protocol;
using enum opentxs::network::blockchain::Transport;

TEST_F(P2PAddress, ipv4_standard)
{
    const auto address = ot_.Factory().BlockchainAddress(
        bitcoin,
        ip::address{ip::address_v4::from_string(address_4_)},
        standard_port_,
        chain_,
        {},
        {});

    EXPECT_TRUE(address.IsValid());
    EXPECT_EQ(address.Display(), endpoint_4_standard_);
    EXPECT_EQ(address.Port(), standard_port_);
    EXPECT_EQ(address.Style(), bitcoin);
    EXPECT_EQ(address.Type(), ipv4);
    EXPECT_EQ(address.Subtype(), invalid);
    EXPECT_TRUE(address.Key().empty());
}

TEST_F(P2PAddress, ipv4_zmq)
{
    const auto address = ot_.Factory().BlockchainAddressZMQ(
        bitcoin,
        ip::address{ip::address_v4::from_string(address_4_)},
        chain_,
        {},
        {},
        key_.Bytes());

    EXPECT_TRUE(address.IsValid());
    EXPECT_EQ(address.Display(), endpoint_4_zmq_);
    EXPECT_EQ(address.Port(), zmq_port_);
    EXPECT_EQ(address.Style(), bitcoin);
    EXPECT_EQ(address.Type(), zmq);
    EXPECT_EQ(address.Subtype(), ipv4);
    EXPECT_EQ(address.Key(), key_.Bytes());
}

TEST_F(P2PAddress, ipv6_standard)
{
    const auto address = ot_.Factory().BlockchainAddress(
        bitcoin,
        ip::address{ip::address_v6::from_string(address_6_)},
        standard_port_,
        chain_,
        {},
        {});

    EXPECT_TRUE(address.IsValid());
    EXPECT_EQ(address.Display(), endpoint_6_standard_);
    EXPECT_EQ(address.Port(), standard_port_);
    EXPECT_EQ(address.Style(), bitcoin);
    EXPECT_EQ(address.Type(), ipv6);
    EXPECT_EQ(address.Subtype(), invalid);
    EXPECT_TRUE(address.Key().empty());
}

TEST_F(P2PAddress, ipv6_zmq)
{
    const auto address = ot_.Factory().BlockchainAddressZMQ(
        bitcoin,
        ip::address{ip::address_v6::from_string(address_6_)},
        chain_,
        {},
        {},
        key_.Bytes());

    EXPECT_TRUE(address.IsValid());
    EXPECT_EQ(address.Display(), endpoint_6_zmq_);
    EXPECT_EQ(address.Port(), zmq_port_);
    EXPECT_EQ(address.Style(), bitcoin);
    EXPECT_EQ(address.Type(), zmq);
    EXPECT_EQ(address.Subtype(), ipv6);
    EXPECT_EQ(address.Key(), key_.Bytes());
}
}  // namespace ottest
