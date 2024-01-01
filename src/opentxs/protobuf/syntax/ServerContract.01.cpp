// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ServerContract.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ServerContract.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ListenAddress.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Nym.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ServerContract& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(id, ServerContractAllowedIdentifier());
    CHECK_SUBOBJECT(nymid, ServerContractAllowedIdentifier());
    CHECK_NAME(name);
    OPTIONAL_SUBOBJECT(publicnym, ServerContractAllowedNym());
    CHECK_HAVE(address);
    CHECK_SUBOBJECTS(address, ServerContractAllowedListenAddress());
    CHECK_KEY(transportkey);
    CHECK_SUBOBJECT(signature, ServerContractAllowedSignature());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
