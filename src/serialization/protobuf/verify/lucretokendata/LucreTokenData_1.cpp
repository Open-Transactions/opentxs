// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/LucreTokenData.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/Ciphertext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCash.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/CashEnums.pb.h"
#include "serialization/protobuf/LucreTokenData.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const LucreTokenData& input,
    const bool silent,
    const TokenState state) -> bool
{
    switch (state) {
        case TokenState::TOKENSTATE_BLINDED: {
            CHECK_SUBOBJECT_VA(
                privateprototoken, LucreTokenDataAllowedCiphertext(), true);
            CHECK_SUBOBJECT_VA(
                publicprototoken, LucreTokenDataAllowedCiphertext(), true);
            CHECK_EXCLUDED(signature);
            CHECK_EXCLUDED(spendable);
        } break;
        case TokenState::TOKENSTATE_SIGNED: {
            CHECK_SUBOBJECT_VA(
                privateprototoken, LucreTokenDataAllowedCiphertext(), true);
            CHECK_SUBOBJECT_VA(
                publicprototoken, LucreTokenDataAllowedCiphertext(), true);
            CHECK_EXISTS(signature);
            CHECK_EXCLUDED(spendable);
        } break;
        case TokenState::TOKENSTATE_READY: {
            CHECK_EXCLUDED(privateprototoken);
            CHECK_EXCLUDED(publicprototoken);
            CHECK_EXCLUDED(signature);
            CHECK_SUBOBJECT_VA(
                spendable, LucreTokenDataAllowedCiphertext(), true);
        } break;
        case TokenState::TOKENSTATE_SPENT: {
            CHECK_EXCLUDED(privateprototoken);
            CHECK_EXCLUDED(publicprototoken);
            CHECK_EXCLUDED(signature);
            CHECK_SUBOBJECT_VA(
                spendable, LucreTokenDataAllowedCiphertext(), true);
        } break;
        case TokenState::TOKENSTATE_EXPIRED: {
            OPTIONAL_SUBOBJECT_VA(
                privateprototoken, LucreTokenDataAllowedCiphertext(), true);
            OPTIONAL_SUBOBJECT_VA(
                publicprototoken, LucreTokenDataAllowedCiphertext(), true);
            OPTIONAL_SUBOBJECT_VA(
                spendable, LucreTokenDataAllowedCiphertext(), true);
        } break;
        case TokenState::TOKENSTATE_ERROR:
        default: {
            FAIL_2("Invalid state", std::to_string(state));
        }
    }

    return true;
}
auto CheckProto_2(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const LucreTokenData& input,
    const bool silent,
    const TokenState) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
