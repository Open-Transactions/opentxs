// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/RPCCommand.hpp"  // IWYU pragma: associated

#include <RPCCommand.pb.h>
#include <RPCEnums.pb.h>

#include "internal/serialization/protobuf/verify/APIArgument.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/AcceptPendingPayment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/AddClaim.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/AddContact.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/CreateInstrumentDefinition.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/CreateNym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/GetWorkflow.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/HDSeed.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/MoveFunds.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/SendMessage.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/SendPayment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ServerContract.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationItem.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyClaim.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/serialization/protobuf/verify/VerifyRPC.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const RPCCommand& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(cookie);
    CHECK_EXISTS(type);

    switch (input.type()) {
        case RPCCOMMAND_ADDCLIENTSESSION: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
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
        case RPCCOMMAND_ADDSERVERSESSION: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
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
        case RPCCOMMAND_LISTCLIENTSESSIONS: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_LISTSERVERSESSIONS: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_IMPORTHDSEED: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_SUBOBJECT(hdseed, RPCCommandAllowedHDSeed());
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
        case RPCCOMMAND_LISTHDSEEDS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_GETHDSEED: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_CREATENYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_SUBOBJECT(createnym, RPCCommandAllowedCreateNym());
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
        case RPCCOMMAND_LISTNYMS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_GETNYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_ADDCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_SUBOBJECTS(claim, RPCCommandAllowedAddClaim());
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
        case RPCCOMMAND_DELETECLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_IMPORTSERVERCONTRACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_HAVE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_SUBOBJECTS(server, RPCCommandAllowedServerContract());
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
        case RPCCOMMAND_LISTSERVERCONTRACTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_REGISTERNYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_CREATEUNITDEFINITION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_SUBOBJECT(
                createunit, RPCCommandAllowedCreateInstrumentDefinition());
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
        case RPCCOMMAND_LISTUNITDEFINITIONS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_ISSUEUNITDEFINITION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_IDENTIFIER(unit);
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
        case RPCCOMMAND_CREATEACCOUNT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_IDENTIFIER(unit);
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
        case RPCCOMMAND_LISTACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
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
        case RPCCOMMAND_GETACCOUNTBALANCE: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_GETACCOUNTACTIVITY: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_SENDPAYMENT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_SUBOBJECT(sendpayment, RPCCommandAllowedSendPayment());
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
        case RPCCOMMAND_MOVEFUNDS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_SUBOBJECT(movefunds, RPCCommandAllowedSendPayment());
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ADDCONTACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_SUBOBJECTS(addcontact, RPCCommandAllowedAddContact());
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTCONTACTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_GETCONTACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_ADDCONTACTCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_SUBOBJECTS(claim, RPCCommandAllowedAddClaim());
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
        case RPCCOMMAND_DELETECONTACTCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_VERIFYCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
            CHECK_SUBOBJECTS(verifyclaim, RPCCommandAllowedVerifyClaim());
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ACCEPTVERIFICATION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
            CHECK_SUBOBJECTS_VA(
                acceptverification,
                RPCCommandAllowedVerificationItem(),
                VerificationType::Indexed);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_SENDCONTACTMESSAGE: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
            CHECK_SUBOBJECTS(sendmessage, RPCCommandAllowedSendMessage());
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETCONTACTACTIVITY: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_GETSERVERCONTRACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_GETPENDINGPAYMENTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_HAVE(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
        case RPCCOMMAND_ACCEPTPENDINGPAYMENTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
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
            CHECK_SUBOBJECTS(
                acceptpendingpayment, RPCCommandAllowedAcceptPendingPayment());
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_CREATECOMPATIBLEACCOUNT:
        case RPCCOMMAND_GETCOMPATIBLEACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_HAVE(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
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
        case RPCCOMMAND_GETWORKFLOW: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
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
            CHECK_SUBOBJECTS(getworkflow, RPCCommandAllowedGetWorkflow());
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETSERVERPASSWORD:
        case RPCCOMMAND_GETADMINNYM:
        case RPCCOMMAND_GETUNITDEFINITION:
        case RPCCOMMAND_GETTRANSACTIONDATA:
        case RPCCOMMAND_LOOKUPACCOUNTID:
        case RPCCOMMAND_RENAMEACCOUNT:
        case RPCCOMMAND_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::proto
