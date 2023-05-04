// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>
#include <tuple>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
template <typename T>
class AsyncConst;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

using namespace std::literals;

struct OPENTXS_EXPORT AddressData {
    ot::UnallocatedVector<ot::Time> times_{};
    ot::UnallocatedVector<ot::blockchain::block::TransactionHash> txids_{};
};

class OPENTXS_EXPORT ApiCryptoBlockchain : public ::testing::Test
{
public:
    using AddressStyle = ot::blockchain::crypto::AddressStyle;
    using Subchain = ot::blockchain::crypto::Subchain;
    using ThreadData =
        std::tuple<std::string_view, std::uint64_t, std::string_view>;
    using ThreadVectors =
        ot::UnallocatedMap<int, ot::UnallocatedVector<ThreadData>>;

    static constexpr auto individual_ = ot::identity::Type::individual;
    static constexpr auto btc_chain_ = ot::blockchain::Type::Bitcoin;
    static constexpr auto bch_chain_ = ot::blockchain::Type::BitcoinCash;
    static constexpr auto ltc_chain_ = ot::blockchain::Type::Litecoin;
    static constexpr auto zero_time_ = ot::Time{};
    static constexpr auto dummy_script_ = "00000000000000000000000000000000"sv;
    static constexpr auto txid_0_ = "00000000000000000000000000000000"sv;
    static constexpr auto txid_1_ = "11111111111111111111111111111111"sv;
    static constexpr auto txid_2_ = "22222222222222222222222222222222"sv;
    static constexpr auto txid_3_ = "33333333333333333333333333333333"sv;
    static constexpr auto txid_4_ = "44444444444444444444444444444444"sv;
    static constexpr auto memo_1_ = "memo 1"sv;
    static constexpr auto memo_2_ = "memo 2"sv;
    static constexpr auto memo_3_ = "memo 3"sv;
    static constexpr auto memo_4_ = "memo 4"sv;

    static const ot::identifier::Generic empty_id_;
    static const ot::UnallocatedCString empty_string_;
    static const ot::UnallocatedVector<ot::UnallocatedCString> alex_external_;
    static const ot::UnallocatedVector<ot::UnallocatedCString> alex_internal_;
    static const ot::UnallocatedVector<ot::UnallocatedCString> bob_external_;
    static const ot::UnallocatedVector<ot::UnallocatedCString> bob_internal_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_btc_external_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_btc_internal_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_bch_external_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_bch_internal_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_ltc_external_;
    static const ot::UnallocatedVector<ot::UnallocatedCString>
        chris_ltc_internal_;

    static bool init_;
    static ot::AsyncConst<ot::PasswordPrompt> reason_p_;
    static ot::AsyncConst<ot::identifier::Nym> invalid_nym_p_;
    static ot::AsyncConst<ot::identifier::Nym> nym_not_in_wallet_p_;
    static ot::AsyncConst<ot::identifier::Nym> alex_p_;
    static ot::AsyncConst<ot::identifier::Nym> bob_p_;
    static ot::AsyncConst<ot::identifier::Nym> chris_p_;
    static ot::AsyncConst<ot::identifier::Nym> daniel_p_;
    static ot::AsyncConst<ot::ByteArray> address_1_p_;
    static ot::AsyncConst<ot::identifier::Generic> contact_alex_p_;
    static ot::AsyncConst<ot::identifier::Generic> contact_bob_p_;
    static ot::AsyncConst<ot::identifier::Generic> contact_chris_p_;
    static ot::AsyncConst<ot::identifier::Generic> contact_daniel_p_;
    static ot::AsyncConst<ot::identifier::Account> account_1_id_;
    static ot::AsyncConst<ot::identifier::Account> account_2_id_;
    static ot::AsyncConst<ot::identifier::Account> account_3_id_;
    static ot::AsyncConst<ot::identifier::Account> account_4_id_;
    static ot::AsyncConst<ot::identifier::Account> account_5_id_;
    static ot::AsyncConst<ot::identifier::Account> account_6_id_;
    static ot::AsyncConst<ot::identifier::Account> account_7_id_;
    static ot::AsyncConst<ot::identifier::Account> account_8_id_;
    static ot::AsyncConst<ot::identifier::Account> account_9_id_;
    static ot::AsyncConst<ot::UnallocatedCString> fingerprint_a_;
    static ot::AsyncConst<ot::UnallocatedCString> fingerprint_b_;
    static ot::AsyncConst<ot::UnallocatedCString> fingerprint_c_;
    static AddressData address_data_;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt& reason_;
    const ot::identifier::Nym& invalid_nym_;
    const ot::identifier::Nym& nym_not_in_wallet_;
    const ot::identifier::Nym& alex_;
    const ot::identifier::Nym& bob_;
    const ot::identifier::Nym& chris_;
    const ot::identifier::Nym& daniel_;
    const ot::Data& address_1_;
    const ot::identifier::Generic& contact_alex_;
    const ot::identifier::Generic& contact_bob_;
    const ot::identifier::Generic& contact_chris_;
    const ot::identifier::Generic& contact_daniel_;
    const ThreadVectors threads_;

    static auto init() -> const ot::api::session::Client&;

    auto check_deterministic_account(
        const ot::identifier::Nym& nym,
        const ot::blockchain::Type chain,
        const ot::identifier::Account& accountID,
        const ot::identifier::Generic& contactID,
        const ot::UnallocatedVector<ot::UnallocatedCString>& external,
        const ot::UnallocatedVector<ot::UnallocatedCString>& internal,
        const Subchain subchain1,
        const Subchain subchain2,
        const ot::UnallocatedCString label1,
        const ot::UnallocatedCString label2) const noexcept -> bool;
    auto check_hd_account(
        const ot::identifier::Nym& nym,
        const ot::blockchain::Type chain,
        const ot::identifier::Account& accountID,
        const ot::identifier::Generic& contactID,
        const ot::UnallocatedVector<ot::UnallocatedCString>& external,
        const ot::UnallocatedVector<ot::UnallocatedCString>& internal)
        const noexcept -> bool;
    auto check_hd_index(
        const ot::identifier::Account& accountID,
        const ot::identifier::Generic& contactID,
        const ot::UnallocatedVector<ot::UnallocatedCString>& expected,
        const ot::blockchain::crypto::HD& account,
        const Subchain subchain,
        const ot::Bip32Index i,
        const ot::UnallocatedCString& label) const noexcept -> bool;
    auto check_initial_state(
        const ot::blockchain::crypto::Deterministic& account,
        const Subchain subchain) const noexcept -> bool;
    auto random_id() const noexcept -> ot::identifier::Generic;
    auto random_tx() const noexcept -> ot::blockchain::block::TransactionHash;

    ApiCryptoBlockchain();
};
}  // namespace ottest
