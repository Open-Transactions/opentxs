/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#if OT_CRYPTO_USING_LIBBITCOIN

#include <gtest/gtest.h>
#include <bitcoin/bitcoin/chain/script.hpp>
#include <bitcoin/bitcoin/chain/input.hpp>
#include <bitcoin/bitcoin/chain/output.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>
#include <bitcoin/bitcoin/wallet/ec_private.hpp>
#include <bitcoin/bitcoin/wallet/ec_public.hpp>

#include "opentxs/core/crypto/Bip47.hpp"
#include "opentxs/core/Data.hpp"

// Some randomly generated private keys that we use to make test scripts. 
std::vector<libbitcoin::wallet::ec_private> pks = {
    libbitcoin::wallet::ec_private(std::string("92jzWxyfpnFWWQSuGL2mTAizDg5BFpqrC8K6StMzoXxEdfXHqz5")),
    libbitcoin::wallet::ec_private(std::string("924REJHf9dqNWT28bhxhKj5fF37hHY1WepBwwhgAX1KkKeRfQCu")), 
    libbitcoin::wallet::ec_private(std::string("92C1QQ2bx9x69CVsYG1WzGsVVMkpB3CY7GNGBKMtG8vqauV2TNo"))};

std::vector<libbitcoin::wallet::ec_private> extra_pks = {
    libbitcoin::wallet::ec_private(std::string("L4P4wKfq2uhtFvx9p58SxJmLKDQA6BAxCZGdvUkF79JZtve9Qwgg")),
    libbitcoin::wallet::ec_private(std::string("L1D4M3eHGXBmiScJp8utABLXX2vhn7t1FP34Z5iZd65KvfTj73Us")), 
    libbitcoin::wallet::ec_private(std::string("L3Qk2VeASwyYmJEAJmynYhYwREBHJbnEsmE7GSVt26zPf996gtVP")), 
    libbitcoin::wallet::ec_private(std::string("KwY3d3uNTDNjZmi5KbH1ZZVawszUNsRRsDon6opqhBaiCJ7DP88F")), 
    libbitcoin::wallet::ec_private(std::string("KwdEDagQ5LuDpa8bgvzWDitEHsjYDxipSyrWQfZe7CJiLEvYMVDa")), 
    libbitcoin::wallet::ec_private(std::string("L4T85i7xeUTFmU2pEbkyq5Mduo2qPkkJANpwtswN21ezq11L88w1"))};

libbitcoin::wallet::ec_public test_key();

libbitcoin::wallet::ec_public test_key() {
    static int i = 0;
    i++;
    if (i == extra_pks.size()) i = 0;
    return extra_pks[i].to_public();
}

libbitcoin::machine::operation push(libbitcoin::data_chunk chunk) {
    return libbitcoin::machine::operation(chunk);
}

libbitcoin::machine::operation push(std::string str) {
    libbitcoin::data_chunk in;
    libbitcoin::decode_base16(in, str);
    uint8_t size = in.size();
    if (size < 1 || size > 75) {
        // error state
    }
    in.insert(in.begin(), &size, &size + 1);
    return push(in);
}

namespace opentxs
{

struct test_script {
    libbitcoin::machine::operation::list input_script, output_script;
    
    // the pay to script hash version of this script. 
    test_script p2sh() {
        libbitcoin::machine::operation::list new_input = input_script;
        new_input.insert(new_input.end(), output_script.begin(), output_script.end());
        return {new_input, 
            libbitcoin::chain::script::to_pay_script_hash_pattern(libbitcoin::wallet::payment_address(new_input))};
    };
    
    // test this script to see that the value read matches the given expected value.
    bool test(libbitcoin::wallet::ec_public expected,
              unsigned int p, // The index of the redeemed tx in the unsorted list of previous txs.
              unsigned int q  // The index of the output which is redeemed by the notification tx.
             ) {
        std::vector<libbitcoin::chain::output> prev_outputs(q + 1);
        for (int i = 0; i < q; i ++) {
            libbitcoin::data_chunk r;
            libbitcoin::decode_base16(r, "random stuff");
            prev_outputs[i] = libbitcoin::chain::output(1, libbitcoin::chain::script::to_null_data_pattern(r));
        }
        prev_outputs[q] = libbitcoin::chain::output(1, output_script);
        
        // Construct a test transaction for the previous output that will be redeemed to 
        // make the notification tx. 
        const libbitcoin::chain::transaction prev(0, 0, std::vector<libbitcoin::chain::input>(), prev_outputs);
        
        if (!prev.is_valid()) return false;
        
        const libbitcoin::hash_digest prev_hash = prev.hash();
        
        // The keys that will be used 
        std::vector<libbitcoin::wallet::ec_public> prev_keys(p);
        for (int i = 0; i < p; i ++) {
            prev_keys[i] = test_key();
        }
        
        // The set of input txs as a list of data. The correct one is always last.
        std::vector<Data> prev_list(p + 1);
        prev_list[p] = Data(prev.to_data());
        
        // The list of input txs that are not the main one we're looking for. 
        std::vector<libbitcoin::chain::transaction> prev_txs(p);
        std::vector<libbitcoin::hash_digest> prev_hashes(p);
        for (int i = 0; i < p; i++) {
            prev_txs[i] = libbitcoin::chain::transaction(0, 0,
                std::vector<libbitcoin::chain::input>(),
                {libbitcoin::chain::output(1,
                    libbitcoin::chain::script::to_pay_key_hash_pattern(libbitcoin::wallet::payment_address(prev_keys[i], libbitcoin::wallet::payment_address::testnet_p2kh)))});
            
            if (!prev_txs[i].is_valid()) return false;
            
            prev_hashes[i] = prev_txs[i].hash();
            prev_list[i] = Data(prev_txs[i].to_data());
        }
        
        // Create the inputs to the test tx. The correct one is always the first one. 
        std::vector<libbitcoin::chain::input> inputs(p + 1);
        inputs[0] = libbitcoin::chain::input(libbitcoin::chain::output_point(prev_hash, q), input_script, 0);
        for (int i = 0; i < p; i ++) {
            libbitcoin::data_chunk pk;
            prev_keys[i].to_data(pk);
            libbitcoin::machine::operation::list alt_input_script = {push("abdcef"), push(pk)};
            inputs[i + 1] = libbitcoin::chain::input(libbitcoin::chain::output_point(prev_hashes[i], 0), 
                alt_input_script, 0);
        }
        
        // Construct a test transaction for the inputs. 
        const libbitcoin::chain::transaction tx(0, 0, 
            inputs, std::vector<libbitcoin::chain::output>());
        
        if (!tx.is_valid()) return false;
        
        std::unique_ptr<Data> extracted = DesignatedPubkey(Data(tx.to_data()), prev_list);
        
        return extracted != nullptr && expected.encoded() == 
            libbitcoin::wallet::ec_public(bip47::ReadData(*extracted)).encoded();
    }
    
    // test this script to see that the value read matches the given expected value.
    bool test(libbitcoin::wallet::ec_public expected) {
        return test(expected, 0, 0);
    }

    // Generate standard test scripts. 
    static test_script p2pk(libbitcoin::wallet::ec_public key, std::string signature) {
        libbitcoin::data_chunk pk;
        key.to_data(pk);
        return {{push(signature)}, libbitcoin::chain::script::to_pay_public_key_pattern(pk)};
    }

    static test_script p2pkh(libbitcoin::wallet::ec_public key, std::string signature) {
        libbitcoin::data_chunk pk;
        key.to_data(pk);
        return {{push(signature), push(pk)}, libbitcoin::chain::script::to_pay_key_hash_pattern(libbitcoin::wallet::payment_address(key, libbitcoin::wallet::payment_address::testnet_p2kh))};
    }

    static test_script p2multi(std::queue<std::string> signatures, int minimum, std::queue<libbitcoin::ec_compressed> pubkeys) {
        std::vector<libbitcoin::ec_compressed> pubs(pubkeys.size());
        int i = 0;
        while(!pubkeys.empty()) {
            pubs[i] = pubkeys.front();
            pubkeys.pop();
            i++;
        }
        
        auto multisig = libbitcoin::chain::script::to_pay_multisig_pattern(minimum, pubs);
        
        test_script z = {{libbitcoin::machine::operation(libbitcoin::machine::opcode::push_size_0)}, multisig};
        while (!signatures.empty()) {
            z.input_script.push_back(push(signatures.front()));
            signatures.pop();
        }
        return z;
    }
};

struct test_case {
    libbitcoin::wallet::ec_public expected;
    test_script script;
    
    bool test() {
        return script.test(expected);
    }
    
    bool test_p2sh() {
        return script.p2sh().test(expected);
    }
    
    std::string to_string() {
        return "{" + libbitcoin::chain::script(script.input_script).to_string(1) + ", " + libbitcoin::chain::script(script.output_script).to_string(1) + "}";
    }
};

// Generate a set of multisig test cases. 
std::queue<test_case> multisig_test_cases() {
    std::vector<std::string> signatures = {"abcd", "0123", "4567"};
    std::queue<test_case> test_cases;
    
    std::vector<libbitcoin::ec_compressed> pub(pks.size());
    
    for (int i = 0; i < pks.size(); i++) {
        pub[i] = pks[i].to_public();
    }
    
    for (int size = 2; size <= 3; size++) {
        for (int of = 2; of <= size; of++) {
            std::queue<std::string> sig;
                
            for (int i = 0; i < of; i ++) {
                sig.push(signatures.at(i));
            }
                
            for(int order = 0; order < pks.size(); order ++) {
                std::queue<libbitcoin::ec_compressed> keys;
            
                libbitcoin::ec_compressed expected = pub.at(order);
                keys.push(expected);
                
                for (int i = 1; i < size; i++) {
                    keys.push(pub.at((order + i)%pub.size()));
                }
                
                test_cases.push({
                    expected,
                    test_script::p2multi(sig, of, keys)});
            }
        }
    }
    
    return test_cases;
}

class raw_test_case {
public:
    opentxs::Data tx;
    std::vector<opentxs::Data> prev;
    
    raw_test_case(std::string hex, std::vector<std::string> prev_hex):tx(Data::fromHex(hex)),prev(prev_hex.size()) {
        auto rtx = libbitcoin::chain::transaction::factory(bip47::ReadData(tx));
        if (!rtx.is_valid()) {
            throw "not valid";
        }
        
        for (int i = 0; i < prev_hex.size(); i ++) {
            prev[i] = Data::fromHex(prev_hex[i]);
            
            auto ztx = libbitcoin::chain::transaction::factory(bip47::ReadData(prev[i]));
            if (!ztx.is_valid()) {
                throw "not valid";
            }
        }
    }
};
    
// Raw txs that we use as test cases. 
    std::vector<raw_test_case> raw_test_cases = {raw_test_case( std::string("010000000134d3a0b2a82327028c0ef4de18e08b947b20efde8fa62ef8cf6c5acdfd3371b3000000006a473044022045cf89a6b050746c1178a4a4872f09c6e742cfd7f137a9c928dcb43ed86815c5022070e7f68c4948bfa0d51811ecdac68926f263ddcae3c80bec960b36ca8e3f6b20012102a0a11c882e89165e4965924754b28ae77c02c96feec96fe9372e9a977a331b7cffffffff030000000000000000536a4c5001000318bdb4978d27893924f950659527b35acbb9c8f61a758bcee1605e768035e391c8d631d02b337be070c9c4131c6df0d37646ebee474c877bc55d0a073b32e20e000000000000000000000000002a41bf07000000001976a9141b7f45dd59c8bb33bc0af532f8aefbfe2196b19288acaa0a0000000000001976a91477c40baad31493ac9d6f087831aaffa92617f90988ac00000000"), 
        {std::string("010000000138ee971d51266752189aea453867a9a4017e8fc494bbc53946032a40490ce8f60100000017160014c069d3bcd52b811ccf677cf850e919075b8d649dffffffff0280a4bf07000000001976a914785babbf897aafb2fd1129d03c6663e44e3ffcc488ac141f9f507400000017a914b192e9c9465fa4b2e67250bcf9adbe5cf45578378700000000")})};

// Test that a basic pay to pubkey tx is recognized and has the correct pubkey returned. 
TEST(bip47_extract_designated_pubkey, pay_to_pubkey)
{
   for (std::vector<libbitcoin::wallet::ec_private>::iterator it = pks.begin(); it != pks.end(); ++it) {
       auto pub = it->to_public();
       ASSERT_TRUE(test_script::p2pk(pub, "abcd").test(pub));
   }
}

TEST(bip47_extract_designated_pubkey, out_of_order)
{
    auto key = pks[0].to_public();
    for (int i = 1; i < 3; i++) {
        for (int j = 1; j < 3; j++) {
            ASSERT_NO_FATAL_FAILURE(test_script::p2pk(key, "abcd").test(key, i, j));
        }
    }
}

TEST(bip47_extract_designated_pubkey, pay_to_pubkey_hash)
{
   for (std::vector<libbitcoin::wallet::ec_private>::iterator it = pks.begin(); it != pks.end(); ++it) {
       auto pub = it->to_public();
       ASSERT_TRUE(test_script::p2pkh(pub, "abcd").test(pub));
   }
}

TEST(bip47_extract_designated_pubkey, pay_to_pubkey_p2sh)
{
   for (std::vector<libbitcoin::wallet::ec_private>::iterator it = pks.begin(); it != pks.end(); ++it) {
       auto pub = it->to_public();
       ASSERT_TRUE(test_script::p2pk(pub, "abcd").p2sh().test(pub));
   }
}

TEST(bip47_extract_designated_pubkey, pay_to_pubkey_hash_p2sh)
{
   for (std::vector<libbitcoin::wallet::ec_private>::iterator it = pks.begin(); it != pks.end(); ++it) {
       auto pub = it->to_public();
       ASSERT_TRUE(test_script::p2pkh(pub, "abcd").p2sh().test(pub));
   }
}

TEST(bip47_extract_designated_pubkey, multisig)
{
    std::queue<test_case> test_cases;
    ASSERT_NO_THROW(test_cases = multisig_test_cases());
    int i = 0;
    while (!test_cases.empty()) {
        ASSERT_TRUE(test_cases.front().test());
        test_cases.pop();
        i++;
    }
}

TEST(bip47_extract_designated_pubkey, multisig_p2sh)
{
    std::queue<test_case> test_cases;
    ASSERT_NO_THROW(test_cases = multisig_test_cases());
    int i = 0;
    while (!test_cases.empty()) {
        ASSERT_TRUE(test_cases.front().test_p2sh());
        test_cases.pop();
        i++;
    }
}

// Tests with raw txs. The raw test cases are all valid and we simply tests that nullptr is not returned. 
// The other tests should have sufficiently demonstrated that the correct pubkey is returned. 
TEST(bip47_extract_designated_pubkey, raw)
{
    for (raw_test_case c : raw_test_cases) {
        std::unique_ptr<Data> z;
        ASSERT_NO_THROW(z = DesignatedPubkey(c.tx, c.prev));
        ASSERT_TRUE(z != nullptr);
    } 
}

} // opentxs

#endif  // OT_CRYPTO_USING_LIBBITCOIN
