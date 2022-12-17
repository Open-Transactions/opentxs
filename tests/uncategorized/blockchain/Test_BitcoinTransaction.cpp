// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstring>
#include <optional>
#include <span>

#include "ottest/data/blockchain/Bip143.hpp"
#include "ottest/fixtures/blockchain/BitcoinTransaction.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(BitcoinTransaction, serialization)
{
    const auto transaction = api_.Factory()
                                 .BlockchainTransaction(
                                     ot::blockchain::Type::Bitcoin,
                                     tx_bytes_.Bytes(),
                                     false,
                                     ot::Clock::now(),
                                     {})
                                 .asBitcoin();

    EXPECT_TRUE(transaction.IsValid());
    EXPECT_EQ(tx_id_, transaction.ID());
    EXPECT_EQ(transaction.Locktime(), 0);
    EXPECT_EQ(transaction.Version(), 1);

    {
        const auto& inputs = transaction.Inputs();

        ASSERT_EQ(3, inputs.size());

        {
            const auto& input1 = inputs[0];

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
            const auto& input2 = inputs[1];

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
            const auto& input3 = inputs[2];

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
        const auto& outputs = transaction.Outputs();

        ASSERT_EQ(2, outputs.size());

        {
            const auto& output1 = outputs[0];

            EXPECT_EQ(1000000, output1.Value());

            const auto& script4 = output1.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script4.Type());
            EXPECT_EQ(Position::Output, script4.Role());
            EXPECT_TRUE(script4.PubkeyHash().has_value());
        }

        {
            const auto& output2 = outputs[1];

            EXPECT_EQ(485000000, output2.Value());

            const auto& script5 = output2.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script5.Type());
            EXPECT_EQ(Position::Output, script5.Role());
            EXPECT_TRUE(script5.PubkeyHash().has_value());
        }
    }

    const auto bytes = Serialize(transaction);

    EXPECT_EQ(tx_bytes_.size(), bytes.size());
    EXPECT_EQ(tx_bytes_, bytes);

    auto transaction2 = api_.Factory()
                            .BlockchainTransaction(
                                ot::blockchain::Type::UnitTest,
                                bytes.Bytes(),
                                false,
                                ot::Clock::now(),
                                {})
                            .asBitcoin();

    EXPECT_TRUE(transaction2.IsValid());
    EXPECT_EQ(transaction2.Locktime(), 0);
    EXPECT_EQ(transaction2.Version(), 1);

    {
        const auto& inputs = transaction2.Inputs();

        ASSERT_EQ(3, inputs.size());

        {
            const auto& input1 = inputs[0];

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
            const auto& input2 = inputs[1];

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
            const auto& input3 = inputs[2];

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
        const auto& outputs = transaction2.Outputs();

        ASSERT_EQ(2, outputs.size());

        {
            const auto& output1 = outputs[0];

            EXPECT_EQ(1000000, output1.Value());

            const auto& script4 = output1.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script4.Type());
            EXPECT_EQ(Position::Output, script4.Role());
            EXPECT_TRUE(script4.PubkeyHash().has_value());
        }

        {
            const auto& output2 = outputs[1];

            EXPECT_EQ(485000000, output2.Value());

            const auto& script5 = output2.Script();

            EXPECT_EQ(Pattern::PayToPubkeyHash, script5.Type());
            EXPECT_EQ(Position::Output, script5.Role());
            EXPECT_TRUE(script5.PubkeyHash().has_value());
        }
    }
}

TEST_F(BitcoinTransaction, normalized_id)
{
    const auto transaction1 = api_.Factory()
                                  .BlockchainTransaction(
                                      ot::blockchain::Type::Bitcoin,
                                      tx_bytes_.Bytes(),
                                      false,
                                      ot::Clock::now(),
                                      {})
                                  .asBitcoin();
    const auto transaction2 = api_.Factory()
                                  .BlockchainTransaction(
                                      ot::blockchain::Type::Bitcoin,
                                      mutated_bytes_.Bytes(),
                                      false,
                                      ot::Clock::now(),
                                      {})
                                  .asBitcoin();

    EXPECT_TRUE(transaction1.IsValid());
    EXPECT_TRUE(transaction2.IsValid());
    EXPECT_EQ(
        IDNormalized(api_, transaction1), IDNormalized(api_, transaction2));

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

TEST_F(BitcoinTransaction, vbytes)
{
    const auto tx = api_.Factory()
                        .BlockchainTransaction(
                            ot::blockchain::Type::Bitcoin,
                            vbyte_test_transaction_.Bytes(),
                            false,
                            ot::Clock::now(),
                            {})
                        .asBitcoin();

    EXPECT_TRUE(tx.IsValid());

    EXPECT_EQ(tx.vBytes(ot::blockchain::Type::Bitcoin), 136u);
    EXPECT_EQ(tx.vBytes(ot::blockchain::Type::BitcoinCash), 218u);
}

TEST_F(BitcoinTransaction, native_p2wpkh)
{
    const auto& hex = GetBip143Vector()[0];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}

TEST_F(BitcoinTransaction, p2sh_p2wpkh)
{
    const auto& hex = GetBip143Vector()[1];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}

TEST_F(BitcoinTransaction, native_p2wsh)
{
    const auto& hex = GetBip143Vector()[2];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}

TEST_F(BitcoinTransaction, native_p2wsh_anyonecanpay)
{
    const auto& hex = GetBip143Vector()[3];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}

TEST_F(BitcoinTransaction, p2sh_p2wpsh)
{
    const auto& hex = GetBip143Vector()[4];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}

TEST_F(BitcoinTransaction, no_find_and_delete)
{
    const auto& hex = GetBip143Vector()[5];
    const auto bytes = api_.Factory().DataFromHex(hex);
    const auto tx = api_.Factory().BlockchainTransaction(
        ot::blockchain::Type::Bitcoin,
        bytes.Bytes(),
        false,
        ot::Clock::now(),
        {});

    EXPECT_TRUE(tx.IsValid());
    // TODO check input, output, and witness sizes
}
}  // namespace ottest
