// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/blockchain/BloomUpdateFlag.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
const auto params_ = ot::blockchain::internal::GetFilterParams(
    ot::blockchain::filter::Type::Basic_BIP158);
using Hash = ot::OTData;
auto stress_test_ = std::pmr::vector<Hash>{};

class Test_Filters : public ::testing::Test
{
public:
    struct TestData {
        std::string block_hash_;
        std::string block_;
        std::pmr::vector<std::string> previous_;
        std::pmr::vector<std::string> outputs_;
        std::string previous_header_;
        std::string filter_;
        std::string header_;
    };
    using TestMap = std::pmr::map<ot::blockchain::block::Height, TestData>;

    static const TestMap gcs_;

    const ot::api::session::Client& api_;

    auto TestGCSBlock(const ot::blockchain::block::Height height) const -> bool
    {
        const auto& vector = gcs_.at(0);
        const auto block =
            api_.Factory().Data(vector.block_hash_, ot::StringStyle::Hex);
        auto elements = std::pmr::vector<ot::OTData>{};

        for (const auto& element : vector.previous_) {
            elements.emplace_back(
                api_.Factory().Data(element, ot::StringStyle::Hex));
        }

        for (const auto& element : vector.outputs_) {
            elements.emplace_back(
                api_.Factory().Data(element, ot::StringStyle::Hex));
        }

        const auto pGCS = ot::factory::GCS(
            api_,
            params_.first,
            params_.second,
            ot::blockchain::internal::BlockHashToFilterKey(block->Bytes()),
            elements);

        EXPECT_TRUE(pGCS);

        if (false == bool(pGCS)) { return false; }

        const auto& gcs = *pGCS;
        const auto encoded = gcs.Encode();

        EXPECT_EQ(vector.filter_, encoded->asHex());

        return true;
    }

    Test_Filters()
        : api_(ot::Context().StartClientSession(0))
    {
    }
};

const Test_Filters::TestMap Test_Filters::gcs_{
    {0,
     {"43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae1801010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {},
      {"4104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f"
       "6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac"},
      "0000000000000000000000000000000000000000000000000000000000000000",
      "019dfca8",
      "21584579b7eb08997773e5aeff3a7f932700042d0ed2a6129012b7d7ae81b750"}},
    {2,
     {"000000006c02c8ea6e4ff69651f7fcde348fb9d557a06e6957b65552002a7820",
      "",
      {},
      {},
      "d7bdac13a59d745b1add0d2ce852f1a0442e8945fc1bf3848d3cbffd88c24fe1",
      "0174a170",
      "186afd11ef2b5e7e3504f2e8cbf8df28a1fd251fe53d60dff8b1467d1b386cf0"}},
    {3,
     {"000000008b896e272758da5297bcd98fdc6d97c9b765ecec401e286dc1fdbe10",
      "",
      {},
      {},
      "186afd11ef2b5e7e3504f2e8cbf8df28a1fd251fe53d60dff8b1467d1b386cf0",
      "016cf7a0",
      "8d63aadf5ab7257cb6d2316a57b16f517bff1c6388f124ec4c04af1212729d2a"}},
    {15007,
     {"0000000038c44c703bae0f98cdd6bf30922326340a5996cc692aaae8bacf47ad",
      "",
      {},
      {},
      "18b5c2b0146d2d09d24fb00ff5b52bd0742f36c9e65527abdb9de30c027a4748",
      "013c3710",
      "07384b01311867949e0c046607c66b7a766d338474bb67f66c8ae9dbd454b20e"}},
    {49291,
     {"9ca177e19c17543f146fd91ece9816e7d6f619a1b5b4281bca7db01800000000",
      "",
      {"5221033423007d8f263819a2e42becaaf5b06f34cb09919e06304349d950668209eaed2"
       "1021d69e2b68c3960903b702af7829fadcd80bd89b158150c85c4a75b2c8cb9c39452a"
       "e",
       "52210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f817982"
       "1021d69e2b68c3960903b702af7829fadcd80bd89b158150c85c4a75b2c8cb9c39452a"
       "e",
       "522102a7ae1e0971fc1689bd66d2a7296da3a1662fd21a53c9e38979e0f090a375c12d2"
       "1022adb62335f41eb4e27056ac37d462cda5ad783fa8e0e526ed79c752475db285d52a"
       "e",
       "52210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f817982"
       "1022adb62335f41eb4e27056ac37d462cda5ad783fa8e0e526ed79c752475db285d52a"
       "e",
       "512103b9d1d0e2b4355ec3cdef7c11a5c0beff9e8b8d8372ab4b4e0aaf30e8017300195"
       "1ae",
       "76a9149144761ebaccd5b4bbdc2a35453585b5637b2f8588ac",
       "522103f1848b40621c5d48471d9784c8174ca060555891ace6d2b03c58eece946b1a912"
       "1020ee5d32b54d429c152fdc7b1db84f2074b0564d35400d89d11870f9273ec140c52a"
       "e",
       "76a914f4fa1cc7de742d135ea82c17adf0bb9cf5f4fb8388ac"},
      {"2102971dd6034ed0cf52450b608d196c07d6345184fcb14deb277a6b82d526a6163dac",
       "76a91445db0b779c0b9fa207f12a8218c94fc77aff504588ac"},
      "ed47705334f4643892ca46396eb3f4196a5e30880589e4009ef38eae895d4a13",
      "0afbc2920af1b027f31f87b592276eb4c32094bb4d3697021b4c6380",
      "b6d98692cec5145f67585f3434ec3c2b3030182e1cb3ec58b855c5c164dfaaa3"}},
    {180480,
     {"00000000fd3ceb2404ff07a785c7fdcc76619edc8ed61bd25134eaa22084366a",
      "",
      {"",
       "",
       "",
       "76a9142903b138c24be9e070b3e73ec495d77a204615e788ac",
       "76a91433a1941fd9a37b9821d376f5a51bd4b52fa50e2888ac",
       "76a914e4374e8155d0865742ca12b8d4d14d41b57d682f88ac",
       "76a914001fa7459a6cfc64bdc178ba7e7a21603bb2568f88ac",
       "76a914f6039952bc2b307aeec5371bfb96b66078ec17f688ac"},
      {},
      "d34ef98386f413769502808d4bac5f20f8dfd5bffc9eedafaa71de0eb1f01489",
      "0db414c859a07e8205876354a210a75042d0463404913d61a8e068e58a3ae2aa080026",
      "c582d51c0ca365e3fcf36c51cb646d7f83a67e867cb4743fd2128e3e022b700c"}},
    {926485,
     {"000000000000015d6077a411a8f5cc95caf775ccf11c54e27df75ce58d187313",
      "",
      {"a914feb8a29635c56d9cd913122f90678756bf23887687",
       "76a914c01a7ca16b47be50cbdbc60724f701d52d75156688ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac",
       "76a914913bcc2be49cb534c20474c4dee1e9c4c317e7eb88ac"},
      {},
      "8f13b9a9c85611635b47906c3053ac53cfcec7211455d4cb0d63dc9acc13d472",
      "09027acea61b6cc3fb33f5d52f7d088a6b2f75d234e89ca800",
      "546c574a0472144bcaf9b6aeabf26372ad87c7af7d1ee0dbfae5e099abeae49c"}},
    {987876,
     {"0000000000000c00901f2049055e2a437c819d79a3d54fd63e6af796cd7b8a79",
      "",
      {},
      {},
      "fe4d230dbb0f4fec9bed23a5283e08baf996e3f32b93f52c7de1f641ddfd04ad",
      "010c0b40",
      "0965a544743bbfa36f254446e75630c09404b3d164a261892372977538928ed5"}},
    {1263442,
     {"000000006f27ddfe1dd680044a34548f41bed47eba9e6f0b310da21423bc5f33",
      "",
      {"002027a5000c7917f785d8fc6e5a55adfca8717ecb973ebb7743849ff956d896a7ed"},
      {},
      "31d66d516a9eda7de865df29f6ef6cb8e4bf9309e5dac899968a9a62a5df61e3",
      "0385acb4f0fe889ef0",
      "4e6d564c2a2452065c205dd7eb2791124e0c4e0dbb064c410c24968572589dec"}},
    {1414221,
     {"0000000000000027b2b3b3381f114f674f481544ff2be37ae3788d7e078383b1",
      "",
      {},
      {},
      "5e5e12d90693c8e936f01847859404c67482439681928353ca1296982042864e",
      "00",
      "021e8882ef5a0ed932edeebbecfeda1d7ce528ec7b3daa27641acf1189d7b5dc"}},
};

TEST_F(Test_Filters, bloom_filter)
{
    std::string s1("blah");
    std::string s2("foo");
    std::string s3("justus");
    std::string s4("fellowtraveler");

    const auto object1(ot::Data::Factory(s1.data(), s1.length()));
    const auto object2(ot::Data::Factory(s2.data(), s2.length()));
    const auto object3(ot::Data::Factory(s3.data(), s3.length()));
    const auto object4(ot::Data::Factory(s4.data(), s4.length()));

    ot::OTBloomFilter pFilter{ot::factory::BloomFilter(
        api_, 9873485, ot::blockchain::BloomUpdateFlag::None, 5, 0.001)};

    pFilter->AddElement(object1);
    pFilter->AddElement(object4);

    EXPECT_TRUE(pFilter->Test(object1));
    EXPECT_FALSE(pFilter->Test(object2));
    EXPECT_FALSE(pFilter->Test(object3));
    EXPECT_TRUE(pFilter->Test(object4));
}

TEST_F(Test_Filters, bitstreams)
{
    {
        auto result = ot::Space{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.flush();

        ASSERT_EQ(1, result.size());
        EXPECT_EQ(229, std::to_integer<std::uint8_t>(result.at(0)));
    }

    {
        auto result = ot::Space{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.flush();

        ASSERT_EQ(2, result.size());
        EXPECT_EQ(185, std::to_integer<std::uint8_t>(result.at(0)));
        EXPECT_EQ(64, std::to_integer<std::uint8_t>(result.at(1)));

        auto reader = ot::blockchain::internal::BitReader{result};

        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
    }

    {
        auto result = ot::Space{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(19, 498577u);
        writer.flush();

        ASSERT_EQ(3, result.size());
        EXPECT_EQ(239, std::to_integer<std::uint8_t>(result.at(0)));
        EXPECT_EQ(55, std::to_integer<std::uint8_t>(result.at(1)));
        EXPECT_EQ(34, std::to_integer<std::uint8_t>(result.at(2)));
    }

    {
        auto result = ot::Space{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(19, 17533928416540072849u);
        writer.flush();
        auto intReader = ot::blockchain::internal::BitReader{result};

        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 0);
        EXPECT_EQ(intReader.read(19u), 498577u);
    }
}

TEST_F(Test_Filters, golomb_coding)
{
    const auto elements = std::pmr::vector<std::uint64_t>{2, 3, 5, 8, 13};
    const auto N = static_cast<std::uint32_t>(elements.size());
    const auto P = std::uint8_t{19};
    const auto encoded = ot::gcs::GolombEncode(P, elements);

    ASSERT_GT(encoded.size(), 0);

    const auto decoded = ot::gcs::GolombDecode(N, P, encoded);

    ASSERT_EQ(elements.size(), decoded.size());

    for (auto i = std::size_t{0}; i < decoded.size(); ++i) {
        EXPECT_EQ(elements.at(i), decoded.at(i));
    }
}

TEST_F(Test_Filters, gcs)
{
    const auto s1 = std::string{"blah"};
    const auto s2 = std::string{"foo"};
    const auto s3 = std::string{"justus"};
    const auto s4 = std::string{"fellowtraveler"};
    const auto s5 = std::string{"islajames"};
    const auto s6 = std::string{"timewaitsfornoman"};
    const auto object1(ot::Data::Factory(s1.data(), s1.length()));
    const auto object2(ot::Data::Factory(s2.data(), s2.length()));
    const auto object3(ot::Data::Factory(s3.data(), s3.length()));
    const auto object4(ot::Data::Factory(s4.data(), s4.length()));
    const auto object5(ot::Data::Factory(s5.data(), s5.length()));
    const auto object6(ot::Data::Factory(s6.data(), s6.length()));

    auto includedElements =
        std::pmr::vector<ot::OTData>{object1, object2, object3, object4};
    auto excludedElements = std::pmr::vector<ot::OTData>{object5, object6};
    auto key = std::string{"0123456789abcdef"};
    auto pGcs = ot::factory::GCS(
        api_, params_.first, params_.second, key, includedElements);

    ASSERT_TRUE(pGcs);

    const auto& gcs = *pGcs;

    EXPECT_TRUE(gcs.Test(object1));
    EXPECT_TRUE(gcs.Test(object2));
    EXPECT_TRUE(gcs.Test(object3));
    EXPECT_TRUE(gcs.Test(object4));
    EXPECT_FALSE(gcs.Test(object5));
    EXPECT_FALSE(gcs.Test(object6));
    EXPECT_TRUE(gcs.Test(includedElements));
    EXPECT_FALSE(gcs.Test(excludedElements));

    const auto partial = std::pmr::vector<ot::ReadView>{
        object1->Bytes(), object4->Bytes(), object5->Bytes(), object6->Bytes()};
    const auto matches = gcs.Match(partial);

    EXPECT_TRUE(2 == matches.size());

    const auto good1 = object1->str();
    const auto good2 = object4->str();

    for (const auto& match : matches) {
        EXPECT_TRUE((good1 == *match) || (good2 == *match));
    }
}

TEST_F(Test_Filters, bip158_case_0) { EXPECT_TRUE(TestGCSBlock(0)); }

TEST_F(Test_Filters, bip158_case_49291) { EXPECT_TRUE(TestGCSBlock(49291)); }

// filter for block
// 0486d358f515ee448ee77f8dadb24e3c49e17e8208bbbcff2241c89900000000 should match
// output 0 of:
// https://live.blockcypher.com/btc-testnet/tx/507449133487c4c288a007b12ae6489204cf7d2316028433e289ef089a66fb8a/
TEST_F(Test_Filters, bip158_case_1665877)
{
    const auto block = api_.Factory().Data(
        "0486d358f515ee448ee77f8dadb24e3c49e17e8208bbbcff2241c89900000000",
        ot::StringStyle::Hex);
    const auto script = api_.Factory().Data(
        "76a914abcfcc53dcae0b49d45a4830a667dfb3aadf185b88ac",
        ot::StringStyle::Hex);
    const auto encodedGCS = api_.Factory().Data(
        "8f1ee69134da43687f3cc23fcba34efcce9b25c6ba1b296a1c456a96d2d0f362800795"
        "dd0746a5264514ad1d50e190590c69a312ff1f3d66426292978c5f14751c1691705f17"
        "a260a23a92dedce351ce983ef4c09a17591e212c7cf34836da468b23f462511311dd53"
        "f7c0f6708305851aa96c6568d9a20e35871771e959790b88cdb70a3152ab500837199a"
        "b528f78ee97ecee538657b18a76c3abae2efb9dcfd013d63c4fe79faebe407bfbdbe47"
        "4ad42809a9493cc5b3d3f4668e2f5e2a2e0c631901dd8a8d2472a325349faa05fbf5b2"
        "642044d481712a30588471898fc5cb74e82f8a023449b5ff171f4393863e7dcf9c95da"
        "e8fd381036ab429c457005b2585da398b5a69fda0e8cfe94276423a55b57a9c5aebbfc"
        "18f6a9870e3ca8ca238a450c7c6aaa7c80f7eac71dfc30a57e4dd7a7abd682bb41e32f"
        "c47db5c3fb445394143671552051462c148e0ada789356733fb38d2e76b90a40e8ffd7"
        "c0bc6bbd8159136b4d48fb4e1709c2dc175cc2172d27110d750648ff2a43d2b8133550"
        "b263e994f9a0f4351e852968fba21f41a27628e900",
        ot::StringStyle::Hex);

    auto key = ot::blockchain::internal::BlockHashToFilterKey(block->Bytes());
    const auto pGcs = ot::factory::GCS(
        api_, params_.first, params_.second, key, 154, encodedGCS->Bytes());

    ASSERT_TRUE(pGcs);

    const auto& gcs = *pGcs;

    EXPECT_TRUE(gcs.Test(script));
}

TEST_F(Test_Filters, bip158_headers)
{
    namespace bc = ot::blockchain::internal;

    const auto filter_0 =
        api_.Factory().Data("0x019dfca8", ot::StringStyle::Hex);
    const auto filter_1 =
        api_.Factory().Data("0x015d5000", ot::StringStyle::Hex);
    const auto filter_2 =
        api_.Factory().Data("0x0174a170", ot::StringStyle::Hex);
    const auto filter_3 =
        api_.Factory().Data("0x016cf7a0", ot::StringStyle::Hex);
    const auto blank = api_.Factory().Data(
        "0x0000000000000000000000000000000000000000000000000000000000000000",
        ot::StringStyle::Hex);
    const auto expected_0 = api_.Factory().Data(
        "0x50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
        ot::StringStyle::Hex);
    const auto expected_1 = api_.Factory().Data(
        "0xe14fc288fdbf3c8d84f31bfc45892e44a0f152e82c0ddd1a5b749da513acbdd7",
        ot::StringStyle::Hex);
    const auto expected_2 = api_.Factory().Data(
        "0xf06c381b7d46b1f8df603de51f25fda128dff8cbe8f204357e5e2bef11fd6a18",
        ot::StringStyle::Hex);
    const auto expected_3 = api_.Factory().Data(
        "0x2a9d721212af044cec24f188631cff7b516fb1576a31d2b67c25b75adfaa638d",
        ot::StringStyle::Hex);
    auto previous = blank->Bytes();
    auto hash = bc::FilterToHash(api_, filter_0->Bytes());
    auto calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    auto calculated_b = bc::FilterToHeader(api_, filter_0->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_0.get());

    previous = expected_0->Bytes();
    hash = bc::FilterToHash(api_, filter_1->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_1->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_1.get());

    previous = expected_1->Bytes();
    hash = bc::FilterToHash(api_, filter_2->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_2->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_2.get());

    previous = expected_2->Bytes();
    hash = bc::FilterToHash(api_, filter_3->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_3->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_3.get());
}

TEST_F(Test_Filters, hash)
{
    namespace bc = ot::blockchain::internal;

    const auto& block_0 = ot::blockchain::node::HeaderOracle::GenesisBlockHash(
        ot::blockchain::Type::Bitcoin_testnet3);
    const auto preimage =
        api_.Factory().Data("0x019dfca8", ot::StringStyle::Hex);
    const auto filter_0 = api_.Factory().Data("0x9dfca8", ot::StringStyle::Hex);
    auto pGcs = ot::factory::GCS(
        api_,
        params_.first,
        params_.second,
        bc::BlockHashToFilterKey(block_0.Bytes()),
        1,
        filter_0->Bytes());

    ASSERT_TRUE(pGcs);

    const auto& gcs = *pGcs;
    const auto hash_a = gcs.Hash();
    const auto hash_b = bc::FilterToHash(api_, preimage->Bytes());

    EXPECT_EQ(hash_a.get(), hash_b.get());
}

TEST_F(Test_Filters, init_array)
{
    constexpr auto count{10000000u};
    stress_test_.reserve(count);

    {
        auto& first = stress_test_.emplace_back(api_.Factory().Data());
        first->SetSize(32);

        ASSERT_TRUE(
            api_.Crypto().Util().RandomizeMemory(first->data(), first->size()));
    }

    while (stress_test_.size() < count) {
        const auto& previous = stress_test_.back();
        auto& next = stress_test_.emplace_back(api_.Factory().Data());

        EXPECT_TRUE(api_.Crypto().Hash().Digest(
            ot::crypto::HashType::Sha256,
            previous->Bytes(),
            next->WriteInto()));
    }
}

TEST_F(Test_Filters, test_set_intersection)
{
    namespace bc = ot::blockchain::internal;

    const auto params = ot::blockchain::internal::GetFilterParams(
        ot::blockchain::filter::Type::ES);
    const auto hash = [&] {
        auto out = api_.Factory().Data();
        out->SetSize(32);
        api_.Crypto().Util().RandomizeMemory(out->data(), out->size());

        return out;
    }();
    const auto pGCS = ot::factory::GCS(
        api_,
        params.first,
        params.second,
        bc::BlockHashToFilterKey(hash->Bytes()),
        stress_test_);

    ASSERT_TRUE(pGCS);

    const auto& gcs = *pGCS;
    const auto subset = [&] {
        constexpr auto count{1000u};
        auto copy{stress_test_};
        auto stop{count};
        auto begin{copy.begin()};
        auto end{copy.end()};
        std::size_t left = std::distance(begin, end);

        while (stop--) {
            auto r = begin;
            std::advance(r, rand() % left);
            std::swap(*begin, *r);
            ++begin;
            --left;
        }

        auto out = decltype(copy){copy.begin(), std::next(copy.begin(), count)};

        return out;
    }();

    const auto targets = [&] {
        auto out = ot::blockchain::GCS::Targets{};
        out.reserve(subset.size());
        std::transform(
            subset.begin(),
            subset.end(),
            std::back_inserter(out),
            [](const auto& i) { return i->Bytes(); });

        return out;
    }();
    const auto matches = gcs.Match(targets);

    EXPECT_EQ(matches.size(), targets.size());
}
}  // namespace ottest
