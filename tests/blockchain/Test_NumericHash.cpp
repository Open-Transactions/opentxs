// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/endian/buffers.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs

namespace be = boost::endian;
namespace ot = opentxs;

namespace ottest
{
class Test_NumericHash : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    Test_NumericHash()
        : api_(ot::Context().StartClientSession(0))
    {
    }
};

TEST_F(Test_NumericHash, init_opentxs) {}

TEST_F(Test_NumericHash, number_low_1)
{
    // Little endian
    const auto raw = ot::Data::Factory("0x01", ot::Data::Mode::Hex);
    const ot::UnallocatedCString decimal{"1"};
    // Big endian
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000000000001"};

    const ot::OTNumericHash number{ot::factory::NumericHash(raw)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, number_low_32)
{
    // Little endian
    const auto raw = ot::Data::Factory(
        "0x0100000000000000000000000000000000000000000000000000000000000000",
        ot::Data::Mode::Hex);
    const ot::UnallocatedCString decimal{"1"};
    // Big endian
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000000000001"};

    const ot::OTNumericHash number{ot::factory::NumericHash(raw)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, number_high)
{
    // Little endian
    const auto raw = ot::Data::Factory(
        "0xf1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
        ot::Data::Mode::Hex);
    const ot::UnallocatedCString decimal{
        "115792089237316195423570985008687907853269984665"
        "640564039457584007913129639921"};
    // Big endian
    const ot::UnallocatedCString hex{
        "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1"};

    const ot::OTNumericHash number{ot::factory::NumericHash(raw)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, nBits_1)
{
    const std::int32_t nBits{83923508};  // 0x05009234
    const ot::UnallocatedCString decimal{"2452881408"};
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000092340000"};

    const ot::OTNumericHash number{ot::factory::NumericHashNBits(nBits)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, nBits_2)
{
    const std::int32_t nBits{68301910};  // 0x04123456
    const ot::UnallocatedCString decimal{"305419776"};
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000012345600"};

    const ot::OTNumericHash number{ot::factory::NumericHashNBits(nBits)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, nBits_3)
{
    const std::int32_t nBits{404472624};  // 0x81bc330
    const ot::UnallocatedCString decimal{
        "680733321990486529407107157001552378184394215934016880640"};
    const ot::UnallocatedCString hex{
        "00000000000000001bc330000000000000000000000000000000000000000000"};

    const ot::OTNumericHash number{ot::factory::NumericHashNBits(nBits)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, nBits_4)
{
    const std::int32_t nBits{453248203};  // 0x1b0404cb
    const ot::UnallocatedCString decimal{
        "1653206561150525499452195696179626311675293455763937233695932416"};
    const ot::UnallocatedCString hex{
        "00000000000404cb000000000000000000000000000000000000000000000000"};

    const ot::OTNumericHash number{ot::factory::NumericHashNBits(nBits)};

    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
}

TEST_F(Test_NumericHash, nBits_5)
{
    const std::int32_t nBits{486604799};  // 0x1d00ffff
    const ot::UnallocatedCString decimal{
        "26959535291011309493156476344723991336010898738574164086137773096960"};
    const ot::UnallocatedCString hex{
        "00000000ffff0000000000000000000000000000000000000000000000000000"};

    const ot::OTNumericHash number{ot::factory::NumericHashNBits(nBits)};
    const auto work = std::unique_ptr<opentxs::blockchain::Work>(
        ot::factory::Work(ot::blockchain::Type::Bitcoin, number));

    ASSERT_TRUE(work);
    EXPECT_EQ(decimal, number->Decimal());
    EXPECT_EQ(hex, number->asHex());
    EXPECT_STREQ("1", work->Decimal().c_str());
}
}  // namespace ottest
