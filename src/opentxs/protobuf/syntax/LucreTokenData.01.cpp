// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/LucreTokenData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CashEnums.pb.h>
#include <opentxs/protobuf/LucreTokenData.pb.h>

#include "opentxs/protobuf/syntax/Ciphertext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCash.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const LucreTokenData& input,
    const Log& log,
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
