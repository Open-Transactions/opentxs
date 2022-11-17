// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"

namespace ot = opentxs;

namespace ottest
{
const auto txid_hex_ = ot::UnallocatedCString{
    "b9451ab8cb828275480da016e97368fdfbfbd9e27dd9bd5d3e6d56d8cd14f301"};
const auto transaction_hex_ = ot::UnallocatedCString{
    "01000000035a19f341c42071f9cec7df37c4853c95d6aecc95e3bf19e3181d30d99552b8c9"
    "000000008a473044022025bca5dc0fe42aca5f07c9b3fe1b3f72113ffbc3522f8d3ebb2457"
    "f5bdf8f9b2022030ff687c00a63e810b21e447d3a57b2749ebea553cab763eb9b99e1b9839"
    "653b014104469f7eb54b90d90106b1a5412b41a23516028e81ad35e0418a4460707ae39a4b"
    "f0101b632260fb08979aba0ceea576b5400c7cf30b539b055ec4c0b96ab00984ffffffff5b"
    "72d3f4b6b72b3511bddd9994f28a91cc03212f200f71b91df13e711d58c1da000000008c49"
    "3046022100fbef2589b7c52a3be0fd8dd3624445da9c8930f0e51f6a33d76dc0ca0304473d"
    "0221009ec433ca6a9f16184db46468ff39cafaa9643021e0c66a1de1e6f9a6120927900141"
    "04b27f4de096ac6431eec4b807a0d3db3e9f9be48faab692d5559624acb1faf4334dd440eb"
    "f32a81506b7c49d8cf40e4b3f5c6b6e99fcb6d3e8a298174bd2b348dffffffff292e947388"
    "51718433a3168e43cab1c6a811e9a0f35b06b6cec60fea9abe0f43010000008a4730440220"
    "582813f2c2d7cbb84521f81d6c2a1147e5296e90bee05f583b3df108fdac72010220232b43"
    "a2e596cef59f82c8bfff1a310d85e7beb3e607076ff8966d6d374dc12b014104a8514ca511"
    "37c6d8a4befa476a7521197b886fceafa9f5c2830bea6df62792a6dd46f2b26812b250f13f"
    "ad473e5cab6dcceaa2d53cf2c82e8e03d95a0e70836bffffffff0240420f00000000001976"
    "a914429e6bd3c9a9ca4be00a4b2b02fd4f5895c1405988ac4083e81c000000001976a914e5"
    "5756cb5395a4b39369d0f1f0a640c12fd867b288ac00000000"};
const auto mutated_transaction_hex_ = ot::UnallocatedCString{
    "01000000035a19f341c42071f9cec7df37c4853c95d6aecc95e3bf19e3181d30d99552b8c9"
    "000000008c4d47003044022025bca5dc0fe42aca5f07c9b3fe1b3f72113ffbc3522f8d3ebb"
    "2457f5bdf8f9b2022030ff687c00a63e810b21e447d3a57b2749ebea553cab763eb9b99e1b"
    "9839653b014104469f7eb54b90d90106b1a5412b41a23516028e81ad35e0418a4460707ae3"
    "9a4bf0101b632260fb08979aba0ceea576b5400c7cf30b539b055ec4c0b96ab00984ffffff"
    "ff5b72d3f4b6b72b3511bddd9994f28a91cc03212f200f71b91df13e711d58c1da00000000"
    "8c493046022100fbef2589b7c52a3be0fd8dd3624445da9c8930f0e51f6a33d76dc0ca0304"
    "473d0221009ec433ca6a9f16184db46468ff39cafaa9643021e0c66a1de1e6f9a612092790"
    "014104b27f4de096ac6431eec4b807a0d3db3e9f9be48faab692d5559624acb1faf4334dd4"
    "40ebf32a81506b7c49d8cf40e4b3f5c6b6e99fcb6d3e8a298174bd2b348dffffffff292e94"
    "738851718433a3168e43cab1c6a811e9a0f35b06b6cec60fea9abe0f43010000008a473044"
    "0220582813f2c2d7cbb84521f81d6c2a1147e5296e90bee05f583b3df108fdac7201022023"
    "2b43a2e596cef59f82c8bfff1a310d85e7beb3e607076ff8966d6d374dc12b014104a8514c"
    "a51137c6d8a4befa476a7521197b886fceafa9f5c2830bea6df62792a6dd46f2b26812b250"
    "f13fad473e5cab6dcceaa2d53cf2c82e8e03d95a0e70836bffffffff0240420f0000000000"
    "1976a914429e6bd3c9a9ca4be00a4b2b02fd4f5895c1405988ac4083e81c000000001976a9"
    "14e55756cb5395a4b39369d0f1f0a640c12fd867b288ac00000000"};
const auto outpoint_hex_1_ = ot::UnallocatedCString{
    "5a19f341c42071f9cec7df37c4853c95d6aecc95e3bf19e3181d30d99552b8c900000000"};
const auto outpoint_hex_2_ = ot::UnallocatedCString{
    "5b72d3f4b6b72b3511bddd9994f28a91cc03212f200f71b91df13e711d58c1da00000000"};
const auto outpoint_hex_3_ = ot::UnallocatedCString{
    "292e94738851718433a3168e43cab1c6a811e9a0f35b06b6cec60fea9abe0f4301000000"};
const auto in_hex_1_ = ot::UnallocatedCString{
    "473044022025bca5dc0fe42aca5f07c9b3fe1b3f72113ffbc3522f8d3ebb2457f5bdf8f9b2"
    "022030ff687c00a63e810b21e447d3a57b2749ebea553cab763eb9b99e1b9839653b014104"
    "469f7eb54b90d90106b1a5412b41a23516028e81ad35e0418a4460707ae39a4bf0101b6322"
    "60fb08979aba0ceea576b5400c7cf30b539b055ec4c0b96ab00984"};
const auto in_hex_2_ = ot::UnallocatedCString{
    "493046022100fbef2589b7c52a3be0fd8dd3624445da9c8930f0e51f6a33d76dc0ca030447"
    "3d0221009ec433ca6a9f16184db46468ff39cafaa9643021e0c66a1de1e6f9a61209279001"
    "4104b27f4de096ac6431eec4b807a0d3db3e9f9be48faab692d5559624acb1faf4334dd440"
    "ebf32a81506b7c49d8cf40e4b3f5c6b6e99fcb6d3e8a298174bd2b348d"};
const auto in_hex_3_ = ot::UnallocatedCString{
    "4730440220582813f2c2d7cbb84521f81d6c2a1147e5296e90bee05f583b3df108fdac7201"
    "0220232b43a2e596cef59f82c8bfff1a310d85e7beb3e607076ff8966d6d374dc12b014104"
    "a8514ca51137c6d8a4befa476a7521197b886fceafa9f5c2830bea6df62792a6dd46f2b268"
    "12b250f13fad473e5cab6dcceaa2d53cf2c82e8e03d95a0e70836b"};
const auto vbyte_test_transaction_hex_ = ot::UnallocatedCString{
    "0100000000010115e180dc28a2327e687facc33f10f2a20da717e5548406f7ae8b4c811072"
    "f85603000000171600141d7cd6c75c2e86f4cbf98eaed221b30bd9a0b928ffffffff019cae"
    "f505000000001976a9141d7cd6c75c2e86f4cbf98eaed221b30bd9a0b92888ac0248304502"
    "2100f764287d3e99b1474da9bec7f7ed236d6c81e793b20c4b5aa1f3051b9a7daa63022016"
    "a198031d5554dbb855bdbe8534776a4be6958bd8d530dc001c32b828f6f0ab0121038262a6"
    "c6cec93c2d3ecd6c6072efea86d02ff8e3328bbd0242b20af3425990ac00000000"};

struct Test_BitcoinTransaction : public ::testing::Test {
    static const ot::Vector<ot::UnallocatedCString> bip143_;

    const ot::api::session::Client& api_;
    const ot::ByteArray tx_id_;
    const ot::ByteArray tx_bytes_;
    const ot::ByteArray mutated_bytes_;
    const ot::ByteArray outpoint_1_;
    const ot::ByteArray outpoint_2_;
    const ot::ByteArray outpoint_3_;
    const ot::ByteArray in_script_1_;
    const ot::ByteArray in_script_2_;
    const ot::ByteArray in_script_3_;

    using Pattern = ot::blockchain::bitcoin::block::Script::Pattern;
    using Position = ot::blockchain::bitcoin::block::Script::Position;

    Test_BitcoinTransaction()
        : api_(ot::Context().StartClientSession(0))
        , tx_id_(api_.Factory().DataFromHex(txid_hex_))
        , tx_bytes_(api_.Factory().DataFromHex(transaction_hex_))
        , mutated_bytes_(api_.Factory().DataFromHex(mutated_transaction_hex_))
        , outpoint_1_(api_.Factory().DataFromHex(outpoint_hex_1_))
        , outpoint_2_(api_.Factory().DataFromHex(outpoint_hex_2_))
        , outpoint_3_(api_.Factory().DataFromHex(outpoint_hex_3_))
        , in_script_1_(api_.Factory().DataFromHex(in_hex_1_))
        , in_script_2_(api_.Factory().DataFromHex(in_hex_2_))
        , in_script_3_(api_.Factory().DataFromHex(in_hex_3_))
    {
    }
};

// https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki
const ot::Vector<ot::UnallocatedCString> Test_BitcoinTransaction::bip143_{
    "01000000000102fff7f7881a8099afa6940d42d1e7f6362bec38171ea3edf43354"
    "1db4e4ad969f00000000494830450221008b9d1dc26ba6a9cb62127b02742fa9d7"
    "54cd3bebf337f7a55d114c8e5cdd30be022040529b194ba3f9281a99f2b1c0a19c"
    "0489bc22ede944ccf4ecbab4cc618ef3ed01eeffffffef51e1b804cc89d182d279"
    "655c3aa89e815b1b309fe287d9b2b55d57b90ec68a0100000000ffffffff02202c"
    "b206000000001976a9148280b37df378db99f66f85c95a783a76ac7a6d5988ac90"
    "93510d000000001976a9143bde42dbee7e4dbe6a21b2d50ce2f0167faa815988ac"
    "000247304402203609e17b84f6a7d30c80bfa610b5b4542f32a8a0d5447a12fb13"
    "66d7f01cc44a0220573a954c4518331561406f90300e8f3358f51928d43c212a8c"
    "aed02de67eebee0121025476c2e83188368da1ff3e292e7acafcdb3566bb0ad253"
    "f62fc70f07aeee635711000000",  // Native P2WPKH
    "01000000000101db6b1b20aa0fd7b23880be2ecbd4a98130974cf4748fb66092ac"
    "4d3ceb1a5477010000001716001479091972186c449eb1ded22b78e40d009bdf00"
    "89feffffff02b8b4eb0b000000001976a914a457b684d7f0d539a46a45bbc043f3"
    "5b59d0d96388ac0008af2f000000001976a914fd270b1ee6abcaea97fea7ad0402"
    "e8bd8ad6d77c88ac02473044022047ac8e878352d3ebbde1c94ce3a10d057c2417"
    "5747116f8288e5d794d12d482f0220217f36a485cae903c713331d877c1f64677e"
    "3622ad4010726870540656fe9dcb012103ad1d8e89212f0b92c74d23bb710c0066"
    "2ad1470198ac48c43f7d6f93a2a2687392040000",  // P2SH-P2WPKH
    "01000000000102fe3dc9208094f3ffd12645477b3dc56f60ec4fa8e6f5d67c565d"
    "1c6b9216b36e000000004847304402200af4e47c9b9629dbecc21f73af989bdaa9"
    "11f7e6f6c2e9394588a3aa68f81e9902204f3fcf6ade7e5abb1295b6774c8e0abd"
    "94ae62217367096bc02ee5e435b67da201ffffffff0815cf020f013ed6cf91d29f"
    "4202e8a58726b1ac6c79da47c23d1bee0a6925f80000000000ffffffff0100f205"
    "2a010000001976a914a30741f8145e5acadf23f751864167f32e0963f788ac0003"
    "47304402200de66acf4527789bfda55fc5459e214fa6083f936b430a762c629656"
    "216805ac0220396f550692cd347171cbc1ef1f51e15282e837bb2b30860dc77c8f"
    "78bc8501e503473044022027dc95ad6b740fe5129e7e62a75dd00f291a2aeb1200"
    "b84b09d9e3789406b6c002201a9ecd315dd6a0e632ab20bbb98948bc0c6fb204f2"
    "c286963bb48517a7058e27034721026dccc749adc2a9d0d89497ac511f760f45c4"
    "7dc5ed9cf352a58ac706453880aeadab210255a9626aebf5e29c0e6538428ba0d1"
    "dcf6ca98ffdf086aa8ced5e0d0215ea465ac00000000",  // Native P2WSH (1)
    "01000000000102e9b542c5176808107ff1df906f46bb1f2583b16112b95ee53806"
    "65ba7fcfc0010000000000ffffffff80e68831516392fcd100d186b3c2c7b95c80"
    "b53c77e77c35ba03a66b429a2a1b0000000000ffffffff02809698000000000019"
    "76a914de4b231626ef508c9a74a8517e6783c0546d6b2888ac8096980000000000"
    "1976a9146648a8cd4531e1ec47f35916de8e259237294d1e88ac02483045022100"
    "f6a10b8604e6dc910194b79ccfc93e1bc0ec7c03453caaa8987f7d6c3413566002"
    "206216229ede9b4d6ec2d325be245c5b508ff0339bf1794078e20bfe0babc7ffe6"
    "83270063ab68210392972e2eb617b2388771abe27235fd5ac44af8e61693261550"
    "447a4c3e39da98ac024730440220032521802a76ad7bf74d0e2c218b72cf0cbc86"
    "7066e2e53db905ba37f130397e02207709e2188ed7f08f4c952d9d13986da50450"
    "2b8c3be59617e043552f506c46ff83275163ab68210392972e2eb617b2388771ab"
    "e27235fd5ac44af8e61693261550447a4c3e39da98ac00000000",  // Native
                                                             // P2WSH
                                                             // (2)
    "0100000000010136641869ca081e70f394c6948e8af409e18b619df2ed74aa106c"
    "1ca29787b96e0100000023220020a16b5755f7f6f96dbd65f5f0d6ab9418b89af4"
    "b1f14a1bb8a09062c35f0dcb54ffffffff0200e9a435000000001976a914389ffc"
    "e9cd9ae88dcc0631e88a821ffdbe9bfe2688acc0832f05000000001976a9147480"
    "a33f950689af511e6e84c138dbbd3c3ee41588ac080047304402206ac44d672dac"
    "41f9b00e28f4df20c52eeb087207e8d758d76d92c6fab3b73e2b0220367750dbbe"
    "19290069cba53d096f44530e4f98acaa594810388cf7409a1870ce014730440220"
    "68c7946a43232757cbdf9176f009a928e1cd9a1a8c212f15c1e11ac9f2925d9002"
    "205b75f937ff2f9f3c1246e547e54f62e027f64eefa2695578cc6432cdabce2715"
    "02473044022059ebf56d98010a932cf8ecfec54c48e6139ed6adb0728c09cbe1e4"
    "fa0915302e022007cd986c8fa870ff5d2b3a89139c9fe7e499259875357e20fcbb"
    "15571c76795403483045022100fbefd94bd0a488d50b79102b5dad4ab6ced30c40"
    "69f1eaa69a4b5a763414067e02203156c6a5c9cf88f91265f5a942e96213afae16"
    "d83321c8b31bb342142a14d16381483045022100a5263ea0553ba89221984bd7f0"
    "b13613db16e7a70c549a86de0cc0444141a407022005c360ef0ae5a5d4f9f2f87a"
    "56c1546cc8268cab08c73501d6b3be2e1e1a8a08824730440220525406a1482936"
    "d5a21888260dc165497a90a15669636d8edca6b9fe490d309c022032af0c646a34"
    "a44d1f4576bf6a4a74b67940f8faa84c7df9abe12a01a11e2b4783cf56210307b8"
    "ae49ac90a048e9b53357a2354b3334e9c8bee813ecb98e99a7e07e8c3ba32103b2"
    "8f0c28bfab54554ae8c658ac5c3e0ce6e79ad336331f78c428dd43eea8449b2103"
    "4b8113d703413d57761b8b9781957b8c0ac1dfe69f492580ca4195f50376ba4a21"
    "033400f6afecb833092a9a21cfdf1ed1376e58c5d1f47de74683123987e967a8f4"
    "2103a6d48b1131e94ba04d9737d61acdaa1322008af9602b3b14862c07a1789aac"
    "162102d8b661b0b3302ee2f162b09e07a55ad5dfbe673a9f01d9f0c19617681024"
    "306b56ae00000000",  // P2SH-P2WSH
    "0100000000010169c12106097dc2e0526493ef67f21269fe888ef05c7a3a5dacab"
    "38e1ac8387f14c1d000000ffffffff01010000000000000000034830450220487f"
    "b382c4974de3f7d834c1b617fe15860828c7f96454490edd6d891556dcc9022100"
    "baf95feb48f845d5bfc9882eb6aeefa1bc3790e39f59eaa46ff7f15ae626c53e01"
    "2102a9781d66b61fb5a7ef00ac5ad5bc6ffc78be7b44a566e3c87870e1079368df"
    "4c4aad4830450220487fb382c4974de3f7d834c1b617fe15860828c7f96454490e"
    "dd6d891556dcc9022100baf95feb48f845d5bfc9882eb6aeefa1bc3790e39f59ea"
    "a46ff7f15ae626c53e0100000000",  // No FindAndDelete
};

TEST_F(Test_BitcoinTransaction, serialization)
{
    const auto transaction = ot::factory::BitcoinTransaction(
        api_.Crypto(),
        ot::blockchain::Type::Bitcoin,
        std::numeric_limits<std::size_t>::max(),
        ot::Clock::now(),
        tx_bytes_.Bytes());

    ASSERT_TRUE(transaction);
    EXPECT_EQ(tx_id_, transaction->ID());
    EXPECT_EQ(transaction->Locktime(), 0);
    EXPECT_EQ(transaction->Version(), 1);

    {
        const auto& inputs = transaction->Inputs();

        ASSERT_EQ(3, inputs.size());

        {
            const auto& input1 = inputs.at(0);

            ASSERT_EQ(sizeof(input1.PreviousOutput()), outpoint_1_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input1.PreviousOutput(),
                    outpoint_1_.data(),
                    outpoint_1_.size()),
                0);

            const auto& script1 = input1.Script();

            EXPECT_EQ(Pattern::Input, script1.Type());
            EXPECT_EQ(Position::Input, script1.Role());

            auto bytes1 = ot::Space{};

            EXPECT_TRUE(script1.Serialize(ot::writer(bytes1)));
            ASSERT_EQ(bytes1.size(), in_script_1_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes1.data(), in_script_1_.data(), in_script_1_.size()),
                0);
            EXPECT_EQ(input1.Sequence(), 4294967295u);
        }

        {
            const auto& input2 = inputs.at(1);

            ASSERT_EQ(sizeof(input2.PreviousOutput()), outpoint_2_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input2.PreviousOutput(),
                    outpoint_2_.data(),
                    outpoint_2_.size()),
                0);

            const auto& script2 = input2.Script();

            EXPECT_EQ(Pattern::Input, script2.Type());
            EXPECT_EQ(Position::Input, script2.Role());

            auto bytes2 = ot::Space{};

            EXPECT_TRUE(script2.Serialize(ot::writer(bytes2)));
            ASSERT_EQ(bytes2.size(), in_script_2_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes2.data(), in_script_2_.data(), in_script_2_.size()),
                0);
            EXPECT_EQ(4294967295u, input2.Sequence());
        }

        {
            const auto& input3 = inputs.at(2);

            ASSERT_EQ(sizeof(input3.PreviousOutput()), outpoint_3_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input3.PreviousOutput(),
                    outpoint_3_.data(),
                    outpoint_3_.size()),
                0);

            const auto& script3 = input3.Script();

            EXPECT_EQ(Pattern::Input, script3.Type());
            EXPECT_EQ(Position::Input, script3.Role());

            auto bytes3 = ot::Space{};

            EXPECT_TRUE(script3.Serialize(ot::writer(bytes3)));
            ASSERT_EQ(bytes3.size(), in_script_3_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes3.data(), in_script_3_.data(), in_script_3_.size()),
                0);
            EXPECT_EQ(4294967295u, input3.Sequence());
        }
    }

    {
        const auto& outputs = transaction->Outputs();

        ASSERT_EQ(2, outputs.size());

        {
            const auto& output1 = outputs.at(0);

            EXPECT_EQ(1000000, output1.Value());

            const auto& script4 = output1.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script4.Type());
            EXPECT_EQ(Position::Output, script4.Role());
            EXPECT_TRUE(script4.PubkeyHash().has_value());
        }

        {
            const auto& output2 = outputs.at(1);

            EXPECT_EQ(485000000, output2.Value());

            const auto& script5 = output2.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script5.Type());
            EXPECT_EQ(Position::Output, script5.Role());
            EXPECT_TRUE(script5.PubkeyHash().has_value());
        }
    }

    auto raw = api_.Factory().Data();

    ASSERT_TRUE(transaction->Serialize(raw.WriteInto()));
    EXPECT_EQ(tx_bytes_.size(), raw.size());
    EXPECT_EQ(tx_bytes_, raw);

    auto bytes = ot::Space{};

    ASSERT_TRUE(transaction->Serialize(ot::writer(bytes)));
    ASSERT_EQ(bytes.size(), tx_bytes_.size());
    EXPECT_EQ(std::memcmp(bytes.data(), tx_bytes_.data(), tx_bytes_.size()), 0);

    auto transaction2 = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::UnitTest, ot::reader(bytes), false);

    ASSERT_TRUE(transaction2);
    EXPECT_EQ(transaction2->Locktime(), 0);
    EXPECT_EQ(transaction2->Version(), 1);

    {
        const auto& inputs = transaction2->Inputs();

        ASSERT_EQ(3, inputs.size());

        {
            const auto& input1 = inputs.at(0);

            ASSERT_EQ(sizeof(input1.PreviousOutput()), outpoint_1_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input1.PreviousOutput(),
                    outpoint_1_.data(),
                    outpoint_1_.size()),
                0);

            const auto& script1 = input1.Script();

            EXPECT_EQ(Pattern::Input, script1.Type());
            EXPECT_EQ(Position::Input, script1.Role());

            auto bytes1 = ot::Space{};

            EXPECT_TRUE(script1.Serialize(ot::writer(bytes1)));
            ASSERT_EQ(bytes1.size(), in_script_1_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes1.data(), in_script_1_.data(), in_script_1_.size()),
                0);
            EXPECT_EQ(4294967295u, input1.Sequence());
        }

        {
            const auto& input2 = inputs.at(1);

            ASSERT_EQ(sizeof(input2.PreviousOutput()), outpoint_2_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input2.PreviousOutput(),
                    outpoint_2_.data(),
                    outpoint_2_.size()),
                0);

            const auto& script2 = input2.Script();

            EXPECT_EQ(Pattern::Input, script2.Type());
            EXPECT_EQ(Position::Input, script2.Role());

            auto bytes2 = ot::Space{};

            EXPECT_TRUE(script2.Serialize(ot::writer(bytes2)));
            ASSERT_EQ(bytes2.size(), in_script_2_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes2.data(), in_script_2_.data(), in_script_2_.size()),
                0);
            EXPECT_EQ(4294967295u, input2.Sequence());
        }

        {
            const auto& input3 = inputs.at(2);

            ASSERT_EQ(sizeof(input3.PreviousOutput()), outpoint_3_.size());
            EXPECT_EQ(
                std::memcmp(
                    &input3.PreviousOutput(),
                    outpoint_3_.data(),
                    outpoint_3_.size()),
                0);

            const auto& script3 = input3.Script();

            EXPECT_EQ(Pattern::Input, script3.Type());
            EXPECT_EQ(Position::Input, script3.Role());

            auto bytes3 = ot::Space{};

            EXPECT_TRUE(script3.Serialize(ot::writer(bytes3)));
            ASSERT_EQ(bytes3.size(), in_script_3_.size());
            EXPECT_EQ(
                std::memcmp(
                    bytes3.data(), in_script_3_.data(), in_script_3_.size()),
                0);
            EXPECT_EQ(4294967295u, input3.Sequence());
        }
    }

    {
        const auto& outputs = transaction2->Outputs();

        ASSERT_EQ(2, outputs.size());

        {
            const auto& output1 = outputs.at(0);

            EXPECT_EQ(1000000, output1.Value());

            const auto& script4 = output1.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script4.Type());
            EXPECT_EQ(Position::Output, script4.Role());
            EXPECT_TRUE(script4.PubkeyHash().has_value());
        }

        {
            const auto& output2 = outputs.at(1);

            EXPECT_EQ(485000000, output2.Value());

            const auto& script5 = output2.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script5.Type());
            EXPECT_EQ(Position::Output, script5.Role());
            EXPECT_TRUE(script5.PubkeyHash().has_value());
        }
    }
}

TEST_F(Test_BitcoinTransaction, normalized_id)
{
    const auto transaction1 = ot::factory::BitcoinTransaction(
        api_.Crypto(),
        ot::blockchain::Type::Bitcoin,
        std::numeric_limits<std::size_t>::max(),
        ot::Clock::now(),
        tx_bytes_.Bytes());
    const auto transaction2 = ot::factory::BitcoinTransaction(
        api_.Crypto(),
        ot::blockchain::Type::Bitcoin,
        std::numeric_limits<std::size_t>::max(),
        ot::Clock::now(),
        mutated_bytes_.Bytes());

    ASSERT_TRUE(transaction1);
    ASSERT_TRUE(transaction2);
    EXPECT_EQ(
        transaction1->IDNormalized(api_.Factory()),
        transaction2->IDNormalized(api_.Factory()));

    auto id1 = api_.Factory().Data();
    auto id2 = api_.Factory().Data();

    ASSERT_TRUE(api_.Crypto().Hash().Digest(
        ot::crypto::HashType::Sha256D, tx_bytes_.Bytes(), id1.WriteInto()));
    ASSERT_TRUE(api_.Crypto().Hash().Digest(
        ot::crypto::HashType::Sha256D,
        mutated_bytes_.Bytes(),
        id2.WriteInto()));
    EXPECT_EQ(id1, tx_id_);
    EXPECT_NE(id1, id2);
}

TEST_F(Test_BitcoinTransaction, vbytes)
{
    const auto bytes = api_.Factory().DataFromHex(vbyte_test_transaction_hex_);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);

    EXPECT_EQ(tx->vBytes(ot::blockchain::Type::Bitcoin), 136u);
    EXPECT_EQ(tx->vBytes(ot::blockchain::Type::BitcoinCash), 218u);
}

TEST_F(Test_BitcoinTransaction, native_p2wpkh)
{
    const auto& hex = bip143_.at(0);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}

TEST_F(Test_BitcoinTransaction, p2sh_p2wpkh)
{
    const auto& hex = bip143_.at(1);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}

TEST_F(Test_BitcoinTransaction, native_p2wsh)
{
    const auto& hex = bip143_.at(2);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}

TEST_F(Test_BitcoinTransaction, native_p2wsh_anyonecanpay)
{
    const auto& hex = bip143_.at(3);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}

TEST_F(Test_BitcoinTransaction, p2sh_p2wpsh)
{
    const auto& hex = bip143_.at(4);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}

TEST_F(Test_BitcoinTransaction, no_find_and_delete)
{
    const auto& hex = bip143_.at(5);
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, bytes.Bytes(), false);

    ASSERT_TRUE(tx);
    // TODO check input, output, and witness sizes
}
}  // namespace ottest
