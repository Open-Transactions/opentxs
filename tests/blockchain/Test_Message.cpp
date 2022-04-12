// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <utility>

#include "1_Internal.hpp"  // IWYU pragma: keep
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace boost
{
namespace asio
{
namespace ip
{
class tcp;
}  // namespace ip
}  // namespace asio
}  // namespace boost

namespace b = ot::blockchain;
namespace bb = b::bitcoin;
namespace bc = b::node;
namespace bp = b::p2p::bitcoin;
namespace zmq = ot::network::zeromq;

using boost::asio::ip::tcp;

namespace ottest
{
class Test_Message : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    void test_service_bit(const bp::Service service)
    {
        auto bytes = bp::GetServiceBytes({service});
        auto services = bp::GetServices(bytes);

        EXPECT_EQ(1, services.count(service));
    }

    Test_Message()
        : api_(ot::Context().StartClientSession(0))
    {
    }
};

TEST_F(Test_Message, init_opentxs) {}

TEST_F(Test_Message, service_bits)
{
    ot::UnallocatedVector<bp::Service> services{
        bp::Service::Bit1,  bp::Service::Bit2,  bp::Service::Bit3,
        bp::Service::Bit4,  bp::Service::Bit5,  bp::Service::Bit6,
        bp::Service::Bit7,  bp::Service::Bit8,  bp::Service::Bit9,
        bp::Service::Bit10, bp::Service::Bit11, bp::Service::Bit12,
        bp::Service::Bit13, bp::Service::Bit14, bp::Service::Bit15,
        bp::Service::Bit16, bp::Service::Bit17, bp::Service::Bit18,
        bp::Service::Bit19, bp::Service::Bit20, bp::Service::Bit21,
        bp::Service::Bit22, bp::Service::Bit23, bp::Service::Bit24,
        bp::Service::Bit25, bp::Service::Bit26, bp::Service::Bit27,
        bp::Service::Bit28, bp::Service::Bit29, bp::Service::Bit30,
        bp::Service::Bit31, bp::Service::Bit32, bp::Service::Bit33,
        bp::Service::Bit34, bp::Service::Bit35, bp::Service::Bit36,
        bp::Service::Bit37, bp::Service::Bit38, bp::Service::Bit39,
        bp::Service::Bit40, bp::Service::Bit41, bp::Service::Bit42,
        bp::Service::Bit43, bp::Service::Bit44, bp::Service::Bit45,
        bp::Service::Bit46, bp::Service::Bit47, bp::Service::Bit48,
        bp::Service::Bit49, bp::Service::Bit50, bp::Service::Bit51,
        bp::Service::Bit52, bp::Service::Bit53, bp::Service::Bit54,
        bp::Service::Bit55, bp::Service::Bit56, bp::Service::Bit57,
        bp::Service::Bit58, bp::Service::Bit59, bp::Service::Bit60,
        bp::Service::Bit61, bp::Service::Bit62, bp::Service::Bit63,
        bp::Service::Bit64,
    };

    for (const auto& service : services) { test_service_bit(service); }
}

TEST_F(Test_Message, getblocks)
{
    namespace bitcoin = ot::blockchain::p2p::bitcoin;

    bitcoin::ProtocolVersionUnsigned version{2};
    ot::UnallocatedVector<ot::OTData> header_hashes;
    for (int ii = 0; ii < 10; ii++) {
        ot::OTData header_hash = ot::Data::Factory();
        header_hash->Randomize(32);
        header_hashes.push_back(header_hash);
    }
    ot::OTData stop_hash = ot::Data::Factory();
    stop_hash->Randomize(32);

    std::unique_ptr<bitcoin::message::Getblocks> pMessage{
        ot::factory::BitcoinP2PGetblocks(
            api_,
            ot::blockchain::Type::BitcoinCash,
            version,
            header_hashes,
            stop_hash)};
    ASSERT_TRUE(pMessage);

    const auto payload = pMessage->payload();

    auto frame = [&] {
        auto out = opentxs::network::zeromq::Message{};
        pMessage->header().Serialize(out.AppendBytes());

        return out;
    }();

    std::unique_ptr<bitcoin::Header> pHeader{ot::factory::BitcoinP2PHeader(
        api_, ot::blockchain::Type::BitcoinCash, frame.at(0))};

    if (payload->size() > 0) {
        std::unique_ptr<bitcoin::Message> pLoadedMsg{
            ot::factory::BitcoinP2PMessage(
                api_,
                std::move(pHeader),
                70015,
                payload->data(),
                payload->size())};
        ASSERT_TRUE(pMessage->payload() == pLoadedMsg->payload());
    } else {
        std::unique_ptr<bitcoin::Message> pLoadedMsg{
            ot::factory::BitcoinP2PMessage(api_, std::move(pHeader), 70015)};
        ASSERT_TRUE(pMessage->payload() == pLoadedMsg->payload());
    }
}
}  // namespace ottest
