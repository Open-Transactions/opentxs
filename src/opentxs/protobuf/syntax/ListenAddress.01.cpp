// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ListenAddress.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContractEnums.pb.h>
#include <opentxs/protobuf/ListenAddress.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ListenAddress& input, const Log& log) -> bool
{
    CHECK_EXISTS(type);

    if ((ADDRESSTYPE_IPV4 > input.type()) || (ADDRESSTYPE_EEP < input.type())) {
        FAIL_1("invalid type");
    }

    if (!input.has_protocol()) { FAIL_1("missing protocol"); }

    if ((PROTOCOLVERSION_ERROR == input.protocol()) ||
        (PROTOCOLVERSION_NOTIFY < input.protocol())) {
        FAIL_1("invalid protocol");
    }

    CHECK_EXISTS(host);
    CHECK_EXISTS(port);

    if (MAX_VALID_PORT < input.port()) { FAIL_1("invalid port"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
