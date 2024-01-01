// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/RPCCommand.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCCommand.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>

#include "opentxs/protobuf/syntax/APIArgument.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/AcceptPendingPayment.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/AddClaim.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/AddContact.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/CreateInstrumentDefinition.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/CreateNym.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/GetWorkflow.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/HDSeed.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/ModifyAccount.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/syntax/MoveFunds.hpp"         // IWYU pragma: keep
#include "opentxs/protobuf/syntax/SendMessage.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/syntax/SendPayment.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ServerContract.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerificationItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyClaim.hpp"       // IWYU pragma: keep

namespace opentxs::protobuf::inline syntax
{
auto version_3(const RPCCommand& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(cookie);
    CHECK_EXISTS(type);

    switch (input.type()) {
        case RPCCOMMAND_LISTACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            OPTIONAL_IDENTIFIER(owner);
            OPTIONAL_IDENTIFIER(notary);
            OPTIONAL_IDENTIFIER(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        default: {
            return version_2(input, log);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
