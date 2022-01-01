// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "crypto/Bip32Vectors.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace ot = opentxs;

namespace
{
struct AddressData {
    std::pmr::vector<ot::Time> times_{};
    std::pmr::vector<ot::OTIdentifier> txids_{};
};

bool init_{false};
const ot::contact::ClaimType individual_{ot::contact::ClaimType::Individual};
const ot::blockchain::Type btc_chain_{ot::blockchain::Type::Bitcoin};
const ot::blockchain::Type bch_chain_{ot::blockchain::Type::BitcoinCash};
const ot::blockchain::Type ltc_chain_{ot::blockchain::Type::Litecoin};
std::unique_ptr<ot::OTPasswordPrompt> reason_p_{nullptr};
std::unique_ptr<ot::OTNymID> invalid_nym_p_{nullptr};
std::unique_ptr<ot::OTNymID> nym_not_in_wallet_p_{nullptr};
std::unique_ptr<ot::OTNymID> alex_p_{nullptr};
std::unique_ptr<ot::OTNymID> bob_p_{nullptr};
std::unique_ptr<ot::OTNymID> chris_p_{nullptr};
std::unique_ptr<ot::OTNymID> daniel_p_{nullptr};
std::unique_ptr<ot::OTData> address_1_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> empty_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_alex_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_bob_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_chris_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_daniel_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_1_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_2_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_3_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_4_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_5_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_6_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_7_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_8_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_9_id_p_{nullptr};
AddressData address_data_{};
const std::string fingerprint_a_{};
const std::string fingerprint_b_{};
const std::string fingerprint_c_{};
const std::string dummy_script_{"00000000000000000000000000000000"};
const std::string txid_0_{"00000000000000000000000000000000"};
const std::string txid_1_{"11111111111111111111111111111111"};
const std::string txid_2_{"22222222222222222222222222222222"};
const std::string txid_3_{"33333333333333333333333333333333"};
const std::string txid_4_{"44444444444444444444444444444444"};
const std::string memo_1_{"memo 1"};
const std::string memo_2_{"memo 2"};
const std::string memo_3_{"memo 3"};
const std::string memo_4_{"memo 4"};
const std::string empty_string_{};
constexpr auto zero_time_ = ot::Time{};
const std::pmr::vector<std::string> alex_external_{
    "1K9teXNg8iKYwUPregT8QTmMepb376oTuX",
    "1GgpoMuPBfaa4ZT6ZeKaTY8NH9Ldx4Q89t",
    "1FXb97adaza32zYQ5U29nxHZS4FmiCfXAJ",
    "1Dx4k7daUS1VNNeoDtZe1ujpt99YeW7Yz",
    "19KhniSVj1CovZWg1P5JvoM199nQR3gkhp",
    "1CBnxZdo58Vu3upwEt96uTMZLAxVx4Xeg9",
    "12vm2SqQ7RhhYPi6bJqqQzyJomV6H3j4AX",
    "1D2fNJYjyWL1jn5qRhJZL6EbGzeyBjHuP3",
    "19w4gVEse89JjE7TroavXZ9pyfJ78h4arG",
    "1DVYvYAmTNtvML7vBrhBBhyePaEDVCCNaw",
};
const std::pmr::vector<std::string> alex_internal_{
    "179XLYWcaHiPMnPUsSdrPiAwNcybx2vpaa",
    "1FPoX1BUe9a6ugobnQkzFyn1Uycyns4Ejp",
    "17jfyBx8ZHJ3DT9G2WehYEPKwT7Zv3kcLs",
    "15zErgibP264JkEMqihXQDp4Kb7vpvDpd5",
    "1KvRA5nngc4aA8y57A6TuS83Gud4xR5oPK",
    "14wC1Ph9z6S82QJA6yTaDaSZQjng9kDihT",
    "1FjW1pENbM6g5PAUpCdjQQykBYH6bzs5hU",
    "1Bt6BP3bXfRJbKUEFS15BrWa6Hca8G9W1L",
    "197TU7ptMMnhufMLFrY1o2Sgi5zcw2e3qv",
    "176aRLv3W94vyWPZDPY9csUrLNrqDFrzCs",
};
const std::pmr::vector<std::string> bob_external_{
    "1AngXb5xQoQ4nT8Bn6dDdr6AFS4yMZU2y",
    "1FQMy3HkD5C3gGZZHeeH9rjHgyqurxC44q",
    "1APXZ5bCTbj2ZRV3ZHyAa59CmsXRP4HkTh",
    "1M966pvtChYbceTsou73eB2hutwoZ7QtVv",
    "1HcN6BWFZKLNEdBo15oUPQGXpDJ26SVKQE",
    "1NcaLRLFr4edY4hUcR81aNMpveHaRqzxPR",
    "1CT86ZmqRFZW57aztRscjWuzkhJjgHjiMS",
    "1CXT6sU5s4mxP4UattFA6fGN7yW4dkkARn",
    "12hwhKpxTyfiSGDdQw63SWVzefRuRxrFqb",
    "18SRAzD6bZ2GsTK4J4RohhYneEyZAUvyqp",
};
const std::pmr::vector<std::string> bob_internal_{
    "1GXj4LrpYKugu4ps7BvYHkUgJLErjBcZc",
    "18yFFsUUe7ATjku2NfKizdnNfZGx99LmLJ",
    "19hDov3sMJdXkgrinhfD2seaKhcb6FiDKL",
    "1W9fEcakg5ZshPuAt5j2vTYkV6txNoiwq",
    "1EPTv3qdCJTbgqUZw83nUbjoKBmy4sHbhd",
    "17mcj9bmcuBfSZqc2mQnjLiT1mtPxGD1yu",
    "1LT2ZEnj1kmpgDbBQodiXVrAj6nRBmWUcH",
    "1HZmwsMWU87WFJxYDNQbnCW52KqUoLiCqZ",
    "16SdtUXrRey55j49Ae84YwVVNZXwGL2tLU",
    "1N2Y3mM828N4JQGLzDfxNjU2WK9CMMekVg",
};
const std::pmr::vector<std::string> chris_btc_external_{
    "1MWZN5PtYjfHA7WC1czB43HK9NjTKig1rA",
    "16Ach28pUQbWDpVhe75AjwoCJws144Nd25",
};
const std::pmr::vector<std::string> chris_btc_internal_{
    "1PsjtCRUQ32t5F18W2K8Zzpn1aVmuRmTdB",
    "15xi7Z3kVPg88ZYA82V8zPyodnQnamSZvN",
};
const std::pmr::vector<std::string> chris_bch_external_{
    "14Et9A6QnwpnUH2Ym9kZ4Zz1FN2GixG9qS",
    "17u11yKTfr13Xkm4k7h4bx3o3ssz4HSwGJ",
};
const std::pmr::vector<std::string> chris_bch_internal_{
    "1FkAAgJWW1YWSqa5ByvHFe8dQvfNLT2rQN",
    "1HyweNdaw2QoRU1YfuJQWcZKUAVqMXyJsj",
};
const std::pmr::vector<std::string> chris_ltc_external_{
    "LWDn8duKKwbP9hhCWpmX9o8BxywgCSTg41",
    "LSyrWGpCUm457F9TaXWAhvZs7Vu5g7a4Do",
};
const std::pmr::vector<std::string> chris_ltc_internal_{
    "LX3FAVopX2moW5h2ZwAKcrCKTChTyWqWze",
    "LMoZuWNnoTEJ1FjxQ4NXTcNbMK3croGpaF",
};

class Test_BlockchainAPI : public ::testing::Test
{
public:
    using AddressStyle = ot::blockchain::crypto::AddressStyle;
    using Subchain = ot::blockchain::crypto::Subchain;
    using ThreadData = std::tuple<std::string, std::uint64_t, std::string>;
    using ThreadVectors = std::pmr::map<int, std::pmr::vector<ThreadData>>;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt& reason_;
    const ot::identifier::Nym& invalid_nym_;
    const ot::identifier::Nym& nym_not_in_wallet_;
    const ot::identifier::Nym& alex_;
    const ot::identifier::Nym& bob_;
    const ot::identifier::Nym& chris_;
    const ot::identifier::Nym& daniel_;
    const ot::Data& address_1_;
    const ot::Identifier& empty_id_;
    const ot::Identifier& contact_alex_;
    const ot::Identifier& contact_bob_;
    const ot::Identifier& contact_chris_;
    const ot::Identifier& contact_daniel_;
    ot::Identifier& account_1_id_;  // alex, btc, bip32 *
    ot::Identifier& account_2_id_;  // daniel, btc, bip32
    ot::Identifier& account_3_id_;  // chris, btc, bip32
    ot::Identifier& account_4_id_;  // chris, btc, bip44
    ot::Identifier& account_5_id_;  // chris, bch, bip44 *
    ot::Identifier& account_6_id_;  // bob, btc, bip32 *
    ot::Identifier& account_7_id_;  // chris, ltc, bip44
    ot::Identifier& account_8_id_;  // alex, btc, bip44
    ot::Identifier& account_9_id_;  // alex, bch, bip44
    const ThreadVectors threads_;

    static const ot::api::session::Client& init()
    {
        const auto& api = ot::Context().StartClientSession(0);

        if (false == init_) {
            reason_p_.reset(new ot::OTPasswordPrompt{
                api.Factory().PasswordPrompt(__func__)});
            invalid_nym_p_.reset(new ot::OTNymID{api.Factory().NymID("junk")});
            nym_not_in_wallet_p_.reset(new ot::OTNymID{api.Factory().NymID(
                "ot2xuVYn8io5LpjK7itnUT7ujx8n5Rt3GKs5xXeh9nfZja2SwB5jEq6")});
            const_cast<std::string&>(fingerprint_a_) =
                api.InternalClient().Exec().Wallet_ImportSeed(
                    "response seminar brave tip suit recall often sound stick "
                    "owner lottery motion",
                    "");
            const_cast<std::string&>(fingerprint_b_) =
                api.InternalClient().Exec().Wallet_ImportSeed(
                    "reward upper indicate eight swift arch injury crystal "
                    "super wrestle already dentist",
                    "");
            const_cast<std::string&>(fingerprint_c_) =
                api.InternalClient().Exec().Wallet_ImportSeed(
                    "predict cinnamon gauge spoon media food nurse improve "
                    "employ similar own kid genius seed ghost",
                    "");
            alex_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym({fingerprint_a_, 0}, individual_, *reason_p_, "Alex")
                    ->ID()});
            bob_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym({fingerprint_b_, 0}, individual_, *reason_p_, "Bob")
                    ->ID()});
            chris_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym({fingerprint_c_, 0}, individual_, *reason_p_, "Chris")
                    ->ID()});
            daniel_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym({fingerprint_a_, 1}, individual_, *reason_p_, "Daniel")
                    ->ID()});

            address_1_p_.reset(new ot::OTData{api.Factory().Data(
                "0xf54a5851e9372b87810a8e60cdd2e7cfd80b6e31",
                ot::StringStyle::Hex)});
            empty_p_.reset(new ot::OTIdentifier{api.Factory().Identifier()});
            contact_alex_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*alex_p_)});
            contact_bob_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*bob_p_)});
            contact_chris_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*chris_p_)});
            contact_daniel_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*daniel_p_)});
            account_1_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_2_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_3_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_4_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_5_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_6_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_7_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_8_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_9_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            init_ = true;
        }

        return api;
    }

    auto check_deterministic_account(
        const ot::identifier::Nym& nym,
        const ot::blockchain::Type chain,
        const ot::Identifier& accountID,
        const ot::Identifier& contactID,
        const std::pmr::vector<std::string>& external,
        const std::pmr::vector<std::string>& internal,
        const Subchain subchain1,
        const Subchain subchain2,
        const std::string label1,
        const std::string label2) const noexcept -> bool
    {
        auto output = true;
        const auto& account = api_.Crypto()
                                  .Blockchain()
                                  .Account(nym, chain)
                                  .GetHD()
                                  .at(accountID);

        EXPECT_EQ(account.ID(), accountID);

        output &= (account.ID() == accountID);
        output &= check_initial_state(account, subchain1);
        output &= check_initial_state(account, subchain2);

        for (ot::Bip32Index i{0}; i < external.size(); ++i) {
            const auto label{label1 + std::to_string(i)};
            output &= check_hd_index(
                accountID, contactID, external, account, subchain1, i, label);
        }

        auto floor = account.Floor(subchain1);

        EXPECT_EQ(floor.value_or(1), 0);

        output &= (floor.value_or(1) == 0);

        for (ot::Bip32Index i{0}; i < internal.size(); ++i) {
            const auto label{label2 + std::to_string(i)};
            output &= check_hd_index(
                accountID, contactID, internal, account, subchain2, i, label);
        }

        floor = account.Floor(subchain2);

        EXPECT_EQ(floor.value_or(1), 0);

        output &= (floor.value_or(1) == 0);

        return output;
    }
    auto check_hd_account(
        const ot::identifier::Nym& nym,
        const ot::blockchain::Type chain,
        const ot::Identifier& accountID,
        const ot::Identifier& contactID,
        const std::pmr::vector<std::string>& external,
        const std::pmr::vector<std::string>& internal) const noexcept -> bool
    {
        return check_deterministic_account(
            nym,
            chain,
            accountID,
            contactID,
            external,
            internal,
            Subchain::External,
            Subchain::Internal,
            "receive ",
            "change ");
    }
    auto check_hd_index(
        const ot::Identifier& accountID,
        const ot::Identifier& contactID,
        const std::pmr::vector<std::string>& expected,
        const ot::blockchain::crypto::HD& account,
        const Subchain subchain,
        const ot::Bip32Index i,
        const std::string& label) const noexcept -> bool
    {
        auto output = true;
        auto index = account.Reserve(subchain, reason_, contactID, label);
        auto generated = account.LastGenerated(subchain);

        EXPECT_TRUE(index);

        if (!index) { return false; }

        EXPECT_EQ(i, index.value());
        EXPECT_TRUE(generated);

        output &= (i == index.value());

        if (!generated) { return false; }

        const auto& target = expected.at(i);
        const auto [bytes, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(target);

        EXPECT_GT(chains.size(), 0u);

        if (chains.size() == 0u) { return false; }

        const auto& chain = *chains.cbegin();
        const auto encoded =
            api_.Crypto().Blockchain().EncodeAddress(style, chain, bytes);

        EXPECT_EQ(target, encoded);

        output &= (target == encoded);
        const auto locator =
            ot::blockchain::crypto::Key{accountID.str(), subchain, i};
        const auto& element = account.BalanceElement(subchain, i);

        EXPECT_EQ(element.Address(AddressStyle::P2PKH), target);
        EXPECT_EQ(element.Confirmed().size(), 0);
        EXPECT_EQ(element.Index(), i);
        EXPECT_TRUE(element.Key());
        EXPECT_EQ(element.KeyID(), locator);
        EXPECT_EQ(element.Label(), label);
        EXPECT_NE(element.LastActivity(), zero_time_);
        EXPECT_TRUE(element.PrivateKey(reason_));
        EXPECT_EQ(element.PubkeyHash(), bytes);
        EXPECT_EQ(element.Subchain(), subchain);
        EXPECT_EQ(element.Unconfirmed().size(), 0);

        output &= (element.Address(AddressStyle::P2PKH) == target);
        output &= (element.Confirmed().size() == 0);
        output &= (element.Index() == i);
        output &= bool(element.Key());
        output &= (element.KeyID() == locator);
        output &= (element.Label() == label);
        output &= (element.LastActivity() != zero_time_);
        output &= bool(element.PrivateKey(reason_));
        output &= (element.PubkeyHash() == bytes);
        output &= (element.Subchain() == subchain);
        output &= (element.Unconfirmed().size() == 0);

        if (Subchain::Internal != subchain) {
            EXPECT_EQ(element.Contact()->str(), contactID.str());

            output &= (element.Contact() == contactID);
        }

        return output;
    }
    auto check_initial_state(
        const ot::blockchain::crypto::Deterministic& account,
        const Subchain subchain) const noexcept -> bool
    {
        auto output = true;
        const auto expected = account.Lookahead() - 1u;
        const auto gen = account.LastGenerated(subchain).value_or(0u);
        const auto floor = account.Floor(subchain);

        EXPECT_EQ(gen, expected);
        EXPECT_TRUE(floor);
        EXPECT_EQ(floor.value_or(1), 0);

        output &= (gen == expected);
        output &= bool(floor);
        output &= (floor.value_or(1) == 0);

        for (auto i = ot::Bip32Index{0u}; i < gen; ++i) {
            const auto& element = account.BalanceElement(subchain, i);

            EXPECT_EQ(element.LastActivity(), zero_time_);

            output &= (element.LastActivity() == zero_time_);
        }

        return output;
    }
    auto random() const noexcept -> ot::OTIdentifier
    {
        auto out = api_.Factory().Identifier();
        out->Randomize();

        return out;
    }

    Test_BlockchainAPI()
        : api_(init())
        , reason_(reason_p_->get())
        , invalid_nym_(invalid_nym_p_->get())
        , nym_not_in_wallet_(nym_not_in_wallet_p_->get())
        , alex_(alex_p_->get())
        , bob_(bob_p_->get())
        , chris_(chris_p_->get())
        , daniel_(daniel_p_->get())
        , address_1_(address_1_p_->get())
        , empty_id_(empty_p_->get())
        , contact_alex_(contact_alex_p_->get())
        , contact_bob_(contact_bob_p_->get())
        , contact_chris_(contact_chris_p_->get())
        , contact_daniel_(contact_daniel_p_->get())
        , account_1_id_(account_1_id_p_->get())
        , account_2_id_(account_2_id_p_->get())
        , account_3_id_(account_3_id_p_->get())
        , account_4_id_(account_4_id_p_->get())
        , account_5_id_(account_5_id_p_->get())
        , account_6_id_(account_6_id_p_->get())
        , account_7_id_(account_7_id_p_->get())
        , account_8_id_(account_8_id_p_->get())
        , account_9_id_(account_9_id_p_->get())
        , threads_({
              {0,
               {
                   {txid_3_, 0, ""},
               }},
              {1,
               {
                   {txid_2_, 0, ""},
               }},
              {2,
               {
                   {txid_1_, 0, ""},
               }},
              {3,
               {
                   {txid_2_, 0, ""},
               }},
              {4,
               {
                   {txid_4_, 1, ""},
               }},
              {5,
               {
                   {txid_4_, 0, ""},
               }},
              {6,
               {
                   {txid_2_, 0, ""},
                   {txid_4_, 1, ""},
               }},
          })
    {
    }
};

TEST_F(Test_BlockchainAPI, init)
{
    EXPECT_TRUE(invalid_nym_.empty());
    EXPECT_FALSE(nym_not_in_wallet_.empty());
    EXPECT_FALSE(alex_.empty());
    EXPECT_FALSE(bob_.empty());
    EXPECT_FALSE(chris_.empty());
    EXPECT_FALSE(daniel_.empty());
    EXPECT_TRUE(empty_id_.empty());
    EXPECT_FALSE(contact_alex_.empty());
    EXPECT_FALSE(contact_bob_.empty());
    EXPECT_FALSE(contact_chris_.empty());
    EXPECT_FALSE(contact_daniel_.empty());
    EXPECT_TRUE(account_1_id_.empty());
    EXPECT_TRUE(account_2_id_.empty());
    EXPECT_TRUE(account_3_id_.empty());
    EXPECT_TRUE(account_4_id_.empty());
    EXPECT_TRUE(account_5_id_.empty());
    EXPECT_TRUE(account_6_id_.empty());
    EXPECT_TRUE(account_7_id_.empty());
    EXPECT_FALSE(fingerprint_a_.empty());
    EXPECT_FALSE(fingerprint_b_.empty());
    EXPECT_FALSE(fingerprint_c_.empty());
}

TEST_F(Test_BlockchainAPI, invalid_nym)
{
    bool loaded(false);

    try {
        api_.Crypto().Blockchain().Account(invalid_nym_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        invalid_nym_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_TRUE(accountID->empty());

    auto list =
        api_.Crypto().Blockchain().SubaccountList(invalid_nym_, btc_chain_);

    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.count(accountID), 0);

    loaded = false;

    try {
        api_.Crypto().Blockchain().Account(nym_not_in_wallet_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nym_not_in_wallet_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_TRUE(accountID->empty());

    list = api_.Crypto().Blockchain().SubaccountList(
        nym_not_in_wallet_, btc_chain_);

    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.count(accountID), 0);
}

// Test: when you create a nym with seed A, then the root of every HDPath for a
// blockchain account associated with that nym should also be A.
TEST_F(Test_BlockchainAPI, TestSeedRoot)
{
    account_1_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        alex_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));
    account_2_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        daniel_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));

    EXPECT_FALSE(account_1_id_.empty());
    EXPECT_FALSE(account_2_id_.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(alex_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_1_id_), 1);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_2_id_), 1);

    // Test difference in index on BIP32 implies a different account
    EXPECT_NE(account_1_id_, account_2_id_);

    try {
        const auto& account1 =
            api_.Crypto().Blockchain().HDSubaccount(alex_, account_1_id_);

        EXPECT_EQ(account1.PathRoot(), fingerprint_a_);
    } catch (const std::exception& e) {
        std::cout << __LINE__ << ": " << e.what() << '\n';
        EXPECT_TRUE(false);
    }

    try {
        const auto& account2 =
            api_.Crypto().Blockchain().HDSubaccount(daniel_, account_2_id_);

        EXPECT_EQ(account2.PathRoot(), fingerprint_a_);
    } catch (const std::exception& e) {
        std::cout << __LINE__ << ": " << e.what() << '\n';
        EXPECT_TRUE(false);
    }

    EXPECT_EQ(alex_, api_.Crypto().Blockchain().Owner(account_1_id_));
    EXPECT_EQ(daniel_, api_.Crypto().Blockchain().Owner(account_2_id_));
}

// Test that one nym creates the same account for the same chain (BIP32 or
// BIP44).
TEST_F(Test_BlockchainAPI, TestNym_AccountIdempotence)
{
    account_3_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));
    account_4_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_));

    EXPECT_FALSE(account_3_id_.empty());
    EXPECT_FALSE(account_4_id_.empty());
    EXPECT_NE(account_3_id_, account_4_id_);

    const auto& before = api_.Crypto()
                             .Blockchain()
                             .Account(chris_, btc_chain_)
                             .GetHD()
                             .at(account_4_id_);

    EXPECT_EQ(before.ID(), account_4_id_);

    const auto duplicate = api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_EQ(account_4_id_, duplicate);

    const auto& after = api_.Crypto()
                            .Blockchain()
                            .Account(chris_, btc_chain_)
                            .GetHD()
                            .at(account_4_id_);

    EXPECT_EQ(after.ID(), account_4_id_);

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, btc_chain_);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.count(account_3_id_), 1);
    EXPECT_EQ(list.count(account_4_id_), 1);
}

// Test that the same nym creates different accounts for two chains
TEST_F(Test_BlockchainAPI, TestChainDiff)
{
    account_5_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        bch_chain_,
        reason_));

    EXPECT_NE(account_5_id_, account_4_id_);

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, bch_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_5_id_), 1);
}

TEST_F(Test_BlockchainAPI, TestBip32_standard_1)
{
    const auto& test = bip32_test_cases_.at(0);
    const std::string empty{};
    auto bytes = api_.Factory().Data(test.seed_, ot::StringStyle::Hex);
    auto seed = api_.Factory().SecretFromBytes(bytes->Bytes());
    const auto fingerprint = api_.Crypto().Seed().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID =
        api_.Wallet()
            .Nym({fingerprint, 0}, individual_, reason_, "John Doe")
            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nymID, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account =
        api_.Crypto().Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto pRoot = account.RootNode(reason_);

    ASSERT_TRUE(pRoot);

    const auto& root = *pRoot;
    const auto& expected = test.children_.at(1);

    EXPECT_EQ(expected.xpub_, root.Xpub(reason_));
    EXPECT_EQ(expected.xprv_, root.Xprv(reason_));
}

// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-3
TEST_F(Test_BlockchainAPI, TestBip32_standard_3)
{
    const auto& test = bip32_test_cases_.at(2);
    const std::string empty{};
    auto bytes = api_.Factory().Data(test.seed_, ot::StringStyle::Hex);
    auto seed = api_.Factory().SecretFromBytes(bytes->Bytes());
    const auto fingerprint = api_.Crypto().Seed().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID =
        api_.Wallet()
            .Nym({fingerprint, 0}, individual_, reason_, "John Doe")
            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nymID, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account =
        api_.Crypto().Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto pRoot = account.RootNode(reason_);

    ASSERT_TRUE(pRoot);

    const auto& root = *pRoot;
    const auto& expected = test.children_.at(1);

    EXPECT_EQ(expected.xpub_, root.Xpub(reason_));
    EXPECT_EQ(expected.xprv_, root.Xprv(reason_));
}

TEST_F(Test_BlockchainAPI, testBip32_SeedA)
{
    EXPECT_TRUE(check_hd_account(
        alex_,
        btc_chain_,
        account_1_id_,
        contact_bob_,
        alex_external_,
        alex_internal_));
}

TEST_F(Test_BlockchainAPI, testBip32_SeedB)
{
    account_6_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        bob_, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_));

    ASSERT_FALSE(account_6_id_.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(bob_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_6_id_), 1);
    EXPECT_TRUE(check_hd_account(
        bob_,
        btc_chain_,
        account_6_id_,
        contact_alex_,
        bob_external_,
        bob_internal_));
}

TEST_F(Test_BlockchainAPI, testBip44_btc)
{
    EXPECT_TRUE(check_hd_account(
        chris_,
        btc_chain_,
        account_4_id_,
        contact_daniel_,
        chris_btc_external_,
        chris_btc_internal_));
}

TEST_F(Test_BlockchainAPI, testBip44_bch)
{
    EXPECT_TRUE(check_hd_account(
        chris_,
        bch_chain_,
        account_5_id_,
        empty_id_,
        chris_bch_external_,
        chris_bch_internal_));
}

TEST_F(Test_BlockchainAPI, testBip44_ltc)
{
    account_7_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        ltc_chain_,
        reason_));

    ASSERT_FALSE(account_7_id_.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, ltc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_7_id_), 1);
    EXPECT_TRUE(check_hd_account(
        chris_,
        ltc_chain_,
        account_7_id_,
        contact_alex_,
        chris_ltc_external_,
        chris_ltc_internal_));
}

TEST_F(Test_BlockchainAPI, AccountList)
{
    auto list = api_.Crypto().Blockchain().SubaccountList(alex_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(alex_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(bob_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(bob_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);
}

TEST_F(Test_BlockchainAPI, reserve_addresses)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    account_8_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        nym, ot::blockchain::crypto::HDProtocol::BIP_44, chain, reason_));
    const auto& accountID = account_8_id_;
    const auto subchain = Subchain::External;

    ASSERT_FALSE(accountID.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(nym, chain);

    EXPECT_EQ(list.count(accountID), 1);

    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);

    ASSERT_EQ(account.ID(), accountID);
    ASSERT_EQ(account.Lookahead(), 20u);

    auto& times = address_data_.times_;
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 8});
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 3});
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 1});

    auto counter{-1};

    for (const auto& time : times) {
        {  // contact only matches, no transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, no transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata mismatch, no transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // no metadata, no transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // no metadata, unconfirmed transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata mismatch, unconfirmed transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, unconfirmed transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, confirmed transactions
            const auto index =
                account.Reserve(subchain, reason_, random(), "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
    }
}

TEST_F(Test_BlockchainAPI, set_metadata)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_;
    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;
    const auto setContact =
        std::pmr::vector<ot::Bip32Index>{0, 1, 6, 8, 9, 14, 16, 17, 22};
    const auto clearContact =
        std::pmr::vector<ot::Bip32Index>{3, 4, 11, 12, 19, 20};
    const auto unconfirmed =
        std::pmr::vector<std::pair<ot::Bip32Index, ot::Bip32Index>>{
            {4, 0},
            {5, 0},
            {6, 0},
            {12, 1},
            {13, 1},
            {14, 1},
            {20, 2},
            {21, 2},
            {22, 2},
        };
    const auto confirmed = std::pmr::vector<ot::Bip32Index>{7, 15, 23};

    for (const auto& index : setContact) {
        EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
            nym, accountID, subchain, index, contact_bob_));

        const auto& element = account.BalanceElement(subchain, index);

        EXPECT_EQ(element.Contact(), contact_bob_);
    }

    for (const auto& index : clearContact) {
        EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
            nym, accountID, subchain, index, empty_id_));

        const auto& element = account.BalanceElement(subchain, index);

        EXPECT_EQ(element.Contact(), empty_id_);
    }

    for (const auto& [index, time] : unconfirmed) {
        const auto& txid = address_data_.txids_.emplace_back(random());

        EXPECT_TRUE(api_.Crypto().Blockchain().Unconfirm(
            {accountID.str(), subchain, index},
            txid,
            address_data_.times_.at(time)));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Unconfirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
    }

    ASSERT_GT(address_data_.txids_.size(), 0);

    auto tIndex = address_data_.txids_.size() - 1u;

    for (const auto& index : confirmed) {
        const auto& txid = address_data_.txids_.emplace_back(random());

        EXPECT_TRUE(api_.Crypto().Blockchain().Unconfirm(
            {accountID.str(), subchain, index}, txid));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Unconfirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
    }

    for (const auto& index : confirmed) {
        const auto& txid = address_data_.txids_.at(++tIndex);

        EXPECT_TRUE(api_.Crypto().Blockchain().Confirm(
            {accountID.str(), subchain, index}, txid));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Confirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
        EXPECT_EQ(element.Unconfirmed().size(), 0);
    }
}

TEST_F(Test_BlockchainAPI, reserve)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_;
    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 3);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 11);
    }

    for (auto i = 24u; i < 44u; ++i) {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), i);
    }

    {
        auto index = account.Reserve(subchain, reason_, contact_bob_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 1);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 4);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 44);
    }
    {
        auto index = account.Reserve(subchain, reason_, contact_bob_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 6);
    }
    {
        auto index = account.Reserve(subchain, reason_, contact_bob_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 9);
    }
    {
        auto index = account.Reserve(subchain, reason_, contact_bob_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 45);
    }
}

TEST_F(Test_BlockchainAPI, release)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_;
    const auto& bc = api_.Crypto().Blockchain();
    const auto& account = bc.Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;

    EXPECT_FALSE(bc.Release({accountID.str(), subchain, 7}));
    EXPECT_FALSE(bc.Release({accountID.str(), subchain, 15}));
    EXPECT_FALSE(bc.Release({accountID.str(), subchain, 23}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 0}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 2}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 5}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 8}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 10}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 12}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 13}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 14}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 16}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 17}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 18}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 19}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 20}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 21}));
    EXPECT_TRUE(bc.Release({accountID.str(), subchain, 22}));

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 0);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 2);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 8);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 10);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 16);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 17);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 18);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 19);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 5);
    }
}

TEST_F(Test_BlockchainAPI, floor)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_;
    const auto& bc = api_.Crypto().Blockchain();
    const auto& account = bc.Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;
    auto& txids = address_data_.txids_;

    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 6}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 5}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 4}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 3}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 2}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 1}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 0}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 7).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID.str(), subchain, 7},
        account.BalanceElement(subchain, 7).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 7).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 7);
    EXPECT_TRUE(bc.Confirm({accountID.str(), subchain, 7}, txids.at(0)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    EXPECT_TRUE(bc.Confirm({accountID.str(), subchain, 7}, txids.at(1)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    EXPECT_TRUE(bc.Unconfirm({accountID.str(), subchain, 7}, txids.at(1)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 15).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID.str(), subchain, 15},
        account.BalanceElement(subchain, 15).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 15).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 0).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID.str(), subchain, 0},
        account.BalanceElement(subchain, 0).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 0).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(bc.Confirm(
        {accountID.str(), subchain, 0}, txids.emplace_back(random())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 12);
    }
}

TEST_F(Test_BlockchainAPI, paymentcode)
{
    const auto& nym = alex_;
    const auto& chain = btc_chain_;
    const auto pNym = api_.Wallet().Nym(nym);
    auto bytes = ot::Space{};
    const auto accountID = api_.Crypto().Blockchain().NewPaymentCodeSubaccount(
        nym,
        api_.Factory().PaymentCode(pNym->PaymentCode()),
        api_.Factory().PaymentCode(std::string{
            "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu"}),
        [&] {
            pNym->PaymentCodePath(ot::writer(bytes));
            return ot::reader(bytes);
        }(),
        chain,
        reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account = api_.Crypto()
                              .Blockchain()
                              .Account(nym, chain)
                              .GetPaymentCode()
                              .at(accountID);

    EXPECT_EQ(account.ID(), accountID);
    EXPECT_TRUE(check_initial_state(account, Subchain::Outgoing));
    EXPECT_TRUE(check_initial_state(account, Subchain::Incoming));

    const auto index = account.Reserve(Subchain::Outgoing, reason_);

    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(index.value(), 0u);
}

TEST_F(Test_BlockchainAPI, batch)
{
    const auto& nym = alex_;
    const auto chain = bch_chain_;
    account_9_id_.Assign(api_.Crypto().Blockchain().NewHDSubaccount(
        nym, ot::blockchain::crypto::HDProtocol::BIP_44, chain, reason_));
    const auto& accountID = account_9_id_;

    ASSERT_FALSE(accountID.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(nym, chain);

    EXPECT_EQ(list.count(accountID), 1);

    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);

    ASSERT_EQ(account.ID(), accountID);

    constexpr auto count{1000u};
    constexpr auto subchain{Subchain::External};
    const auto indices = account.Reserve(subchain, count, reason_);
    const auto gen = account.LastGenerated(subchain);

    ASSERT_TRUE(gen.has_value());
    EXPECT_EQ(gen.value(), count - 1u);
    ASSERT_TRUE(indices.size() == count);

    for (auto i{0u}; i < count; ++i) {
        const auto index = indices.at(i);

        EXPECT_EQ(index, i);

        try {
            account.BalanceElement(subchain, index);

            EXPECT_TRUE(true);
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
}
}  // namespace
