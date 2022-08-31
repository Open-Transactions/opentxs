// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/api/crypto/Blockchain.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/AsyncConst.hpp"
#include "opentxs/opentxs.hpp"

namespace ottest
{
ApiCryptoBlockchain::ApiCryptoBlockchain()
    : api_(init())
    , reason_(reason_p_.get())
    , invalid_nym_(invalid_nym_p_.get())
    , nym_not_in_wallet_(nym_not_in_wallet_p_.get())
    , alex_(alex_p_.get())
    , bob_(bob_p_.get())
    , chris_(chris_p_.get())
    , daniel_(daniel_p_.get())
    , address_1_(address_1_p_.get())
    , contact_alex_(contact_alex_p_.get())
    , contact_bob_(contact_bob_p_.get())
    , contact_chris_(contact_chris_p_.get())
    , contact_daniel_(contact_daniel_p_.get())
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

auto ApiCryptoBlockchain::check_deterministic_account(
    const ot::identifier::Nym& nym,
    const ot::blockchain::Type chain,
    const ot::identifier::Generic& accountID,
    const ot::identifier::Generic& contactID,
    const ot::UnallocatedVector<ot::UnallocatedCString>& external,
    const ot::UnallocatedVector<ot::UnallocatedCString>& internal,
    const Subchain subchain1,
    const Subchain subchain2,
    const ot::UnallocatedCString label1,
    const ot::UnallocatedCString label2) const noexcept -> bool
{
    auto output = true;
    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);

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

auto ApiCryptoBlockchain::check_hd_account(
    const ot::identifier::Nym& nym,
    const ot::blockchain::Type chain,
    const ot::identifier::Generic& accountID,
    const ot::identifier::Generic& contactID,
    const ot::UnallocatedVector<ot::UnallocatedCString>& external,
    const ot::UnallocatedVector<ot::UnallocatedCString>& internal)
    const noexcept -> bool
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

auto ApiCryptoBlockchain::check_hd_index(
    const ot::identifier::Generic& accountID,
    const ot::identifier::Generic& contactID,
    const ot::UnallocatedVector<ot::UnallocatedCString>& expected,
    const ot::blockchain::crypto::HD& account,
    const Subchain subchain,
    const ot::Bip32Index i,
    const ot::UnallocatedCString& label) const noexcept -> bool
{
    auto output = true;
    auto index = account.Reserve(subchain, contactID, reason_, label);
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
    const auto locator = ot::blockchain::crypto::Key{accountID, subchain, i};
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
        EXPECT_EQ(
            element.Contact().asBase58(api_.Crypto()),
            contactID.asBase58(api_.Crypto()));

        output &= (element.Contact() == contactID);
    }

    return output;
}

auto ApiCryptoBlockchain::check_initial_state(
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

auto ApiCryptoBlockchain::init() -> const ot::api::session::Client&
{
    const auto& api = ot::Context().StartClientSession(0);

    if (false == init_) {
        reason_p_.set_value(api.Factory().PasswordPrompt(__func__));
        const auto& reason = reason_p_.get();
        invalid_nym_p_.set_value(api.Factory().NymIDFromBase58("junk"));
        nym_not_in_wallet_p_.set_value(api.Factory().NymIDFromBase58(
            "ot2xuVYn8io5LpjK7itnUT7ujx8n5Rt3GKs5xXeh9nfZja2SwB5jEq"
            "6"));
        fingerprint_a_.set_value(api.InternalClient().Exec().Wallet_ImportSeed(
            "response seminar brave tip suit recall often sound stick "
            "owner lottery motion",
            ""));
        fingerprint_b_.set_value(api.InternalClient().Exec().Wallet_ImportSeed(
            "reward upper indicate eight swift arch injury crystal "
            "super wrestle already dentist",
            ""));
        fingerprint_c_.set_value(api.InternalClient().Exec().Wallet_ImportSeed(
            "predict cinnamon gauge spoon media food nurse improve "
            "employ similar own kid genius seed ghost",
            ""));
        alex_p_.set_value(
            api.Wallet()
                .Nym({fingerprint_a_.get(), 0}, individual_, reason, "Alex")
                ->ID());
        bob_p_.set_value(
            api.Wallet()
                .Nym({fingerprint_b_.get(), 0}, individual_, reason, "Bob")
                ->ID());
        chris_p_.set_value(
            api.Wallet()
                .Nym({fingerprint_c_.get(), 0}, individual_, reason, "Chris")
                ->ID());
        daniel_p_.set_value(
            api.Wallet()
                .Nym({fingerprint_a_.get(), 1}, individual_, reason, "Daniel")
                ->ID());
        address_1_p_.set_value(api.Factory().DataFromHex(
            "0xf54a5851e9372b87810a8e60cdd2e7cfd80b6e31"));
        contact_alex_p_.set_value(api.Contacts().ContactID(alex_p_.get()));
        contact_bob_p_.set_value(api.Contacts().ContactID(bob_p_.get()));
        contact_chris_p_.set_value(api.Contacts().ContactID(chris_p_.get()));
        contact_daniel_p_.set_value(api.Contacts().ContactID(daniel_p_.get()));
        init_ = true;
    }

    return api;
}

auto ApiCryptoBlockchain::random() const noexcept -> ot::identifier::Generic
{
    return api_.Factory().IdentifierFromRandom();
}
}  // namespace ottest

namespace ottest
{
const ot::identifier::Generic ApiCryptoBlockchain::empty_id_{};
const ot::UnallocatedCString ApiCryptoBlockchain::empty_string_{};
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::alex_external_{
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
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::alex_internal_{
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
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::bob_external_{
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
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::bob_internal_{
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
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_btc_external_{
        "1MWZN5PtYjfHA7WC1czB43HK9NjTKig1rA",
        "16Ach28pUQbWDpVhe75AjwoCJws144Nd25",
    };
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_btc_internal_{
        "1PsjtCRUQ32t5F18W2K8Zzpn1aVmuRmTdB",
        "15xi7Z3kVPg88ZYA82V8zPyodnQnamSZvN",
    };
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_bch_external_{
        "14Et9A6QnwpnUH2Ym9kZ4Zz1FN2GixG9qS",
        "17u11yKTfr13Xkm4k7h4bx3o3ssz4HSwGJ",
    };
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_bch_internal_{
        "1FkAAgJWW1YWSqa5ByvHFe8dQvfNLT2rQN",
        "1HyweNdaw2QoRU1YfuJQWcZKUAVqMXyJsj",
    };
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_ltc_external_{
        "LWDn8duKKwbP9hhCWpmX9o8BxywgCSTg41",
        "LSyrWGpCUm457F9TaXWAhvZs7Vu5g7a4Do",
    };
const ot::UnallocatedVector<ot::UnallocatedCString>
    ApiCryptoBlockchain::chris_ltc_internal_{
        "LX3FAVopX2moW5h2ZwAKcrCKTChTyWqWze",
        "LMoZuWNnoTEJ1FjxQ4NXTcNbMK3croGpaF",
    };
bool ApiCryptoBlockchain::init_{false};
ot::AsyncConst<ot::OTPasswordPrompt> ApiCryptoBlockchain::reason_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::invalid_nym_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::nym_not_in_wallet_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::alex_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::bob_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::chris_p_{};
ot::AsyncConst<ot::identifier::Nym> ApiCryptoBlockchain::daniel_p_{};
ot::AsyncConst<ot::ByteArray> ApiCryptoBlockchain::address_1_p_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::contact_alex_p_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::contact_bob_p_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::contact_chris_p_{};
ot::AsyncConst<ot::identifier::Generic>
    ApiCryptoBlockchain::contact_daniel_p_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_1_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_2_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_3_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_4_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_5_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_6_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_7_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_8_id_{};
ot::AsyncConst<ot::identifier::Generic> ApiCryptoBlockchain::account_9_id_{};
ot::AsyncConst<ot::UnallocatedCString> ApiCryptoBlockchain::fingerprint_a_{};
ot::AsyncConst<ot::UnallocatedCString> ApiCryptoBlockchain::fingerprint_b_{};
ot::AsyncConst<ot::UnallocatedCString> ApiCryptoBlockchain::fingerprint_c_{};
AddressData ApiCryptoBlockchain::address_data_{};
}  // namespace ottest
