// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/blockchain/Cashtoken.hpp"  // IWYU pragma: associated

namespace ottest
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#include <bch_vmb_tests_before_chip_cashtokens_nonstandard_json.h>
#include <bch_vmb_tests_before_chip_cashtokens_standard_json.h>
#include <bch_vmb_tests_chip_cashtokens_nonstandard_json.h>
#include <bch_vmb_tests_chip_cashtokens_standard_json.h>

#pragma GCC diagnostic pop
}  // namespace ottest

namespace ottest
{
auto bch_vmb_tests_before_chip_cashtokens_nonstandard_json() noexcept
    -> std::string_view
{
    return {
        reinterpret_cast<const char*>(
            ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_before_chip_cashtokens_nonstandard_json),
        ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_before_chip_cashtokens_nonstandard_json_len};
}

auto bch_vmb_tests_before_chip_cashtokens_standard_json() noexcept
    -> std::string_view
{
    return {
        reinterpret_cast<const char*>(
            ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_before_chip_cashtokens_standard_json),
        ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_before_chip_cashtokens_standard_json_len};
}

auto bch_vmb_tests_chip_cashtokens_nonstandard_json() noexcept
    -> std::string_view
{
    return {
        reinterpret_cast<const char*>(
            ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_chip_cashtokens_nonstandard_json),
        ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_chip_cashtokens_nonstandard_json_len};
}

auto bch_vmb_tests_chip_cashtokens_standard_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(
            ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_chip_cashtokens_standard_json),
        ______tests_ottest_data_blockchain_cashtoken_test_vectors_vmb_tests_bch_vmb_tests_chip_cashtokens_standard_json_len};
}
}  // namespace ottest
