// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Filters.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/blockchain/Blockchain.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;

const ot::blockchain::internal::FilterParams Filters::params_{
    ot::blockchain::internal::GetFilterParams(
        ot::blockchain::cfilter::Type::Basic_BIP158)};

const Filters::TestMap Filters::gcs_{
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
      "021e8882ef5a0ed932edeebbecfeda1d7ce528ec7b3daa27641acf1189d7b5dc"}}};

Filters::Filters()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
{
}

auto Filters::TestGCSBlock(const ot::blockchain::block::Height height) const
    -> bool
{
    const auto& vector = gcs_.at(height);
    const auto block = api_.Factory().DataFromHex(vector.block_hash_);
    auto elements = ot::Vector<ot::ByteArray>{};

    for (const auto& element : vector.previous_) {
        elements.emplace_back(api_.Factory().DataFromHex(element));
    }

    for (const auto& element : vector.outputs_) {
        elements.emplace_back(api_.Factory().DataFromHex(element));
    }

    const auto gcs = ot::factory::GCS(
        api_,
        params_.first,
        params_.second,
        ot::blockchain::internal::BlockHashToFilterKey(block.Bytes()),
        elements,
        {});

    EXPECT_TRUE(gcs.IsValid());

    if (false == gcs.IsValid()) { return false; }

    const auto encoded = [&] {
        auto out = api_.Factory().Data();
        gcs.Encode(out.WriteInto());

        return out;
    }();

    EXPECT_EQ(vector.filter_, encoded.asHex());

    return true;
}
}  // namespace ottest
