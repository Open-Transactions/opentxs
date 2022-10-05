// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/RPCResponse.hpp"  // IWYU pragma: associated

#include <RPCEnums.pb.h>
#include <RPCResponse.pb.h>
#include <RPCStatus.pb.h>
#include <string>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/AccountData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/AccountEvent.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Contact.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ContactEvent.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/HDSeed.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Nym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PaymentWorkflow.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/RPCStatus.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/RPCTask.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ServerContract.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/SessionData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/TransactionData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyRPC.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_2(const RPCResponse& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(cookie);

    bool atLeastOne = false;
    for (auto status : input.status()) {
        if (RPCRESPONSE_SUCCESS == status.code()) {
            atLeastOne = true;
            break;
        }
    }

    switch (input.type()) {
        case RPCCOMMAND_ADDCLIENTSESSION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ADDSERVERSESSION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTCLIENTSESSIONS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_SUBOBJECTS(sessions, RPCResponseAllowedSessionData());
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTSERVERSESSIONS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_SUBOBJECTS(sessions, RPCResponseAllowedSessionData());
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_IMPORTHDSEED: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_SIZE(identifier, 1);
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTHDSEEDS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETHDSEED: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(seed, RPCResponseAllowedHDSeed());
            } else {
                CHECK_NONE(seed);
            }

            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_CREATENYM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_SIZE(identifier, 1);
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTNYMS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETNYM: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(nym, RPCResponseAllowedNym());
            } else {
                CHECK_NONE(nym);
            }

            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ADDCLAIM: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_DELETECLAIM: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_IMPORTSERVERCONTRACT: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTSERVERCONTRACTS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_REGISTERNYM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_SUBOBJECTS(task, RPCResponseAllowedRPCTask());
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_CREATEUNITDEFINITION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_SIZE(identifier, 1);
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTUNITDEFINITIONS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ISSUEUNITDEFINITION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_SUBOBJECTS(task, RPCResponseAllowedRPCTask());
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_CREATECOMPATIBLEACCOUNT:
        case RPCCOMMAND_CREATEACCOUNT: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_SIZE(identifier, 1);
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_SUBOBJECTS(task, RPCResponseAllowedRPCTask());
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTACCOUNTS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETACCOUNTBALANCE: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(balance, RPCResponseAllowedAccountData());
            } else {
                CHECK_NONE(balance);
            }

            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETACCOUNTACTIVITY: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            OPTIONAL_SUBOBJECTS(accountevent, RPCResponseAllowedAccountEvent());
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_SENDPAYMENT: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_SUBOBJECTS(task, RPCResponseAllowedRPCTask());
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_MOVEFUNDS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ADDCONTACT: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_LISTCONTACTS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETCONTACT: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(contact, RPCResponseAllowedContact());
            } else {
                CHECK_NONE(contact);
            }

            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ADDCONTACTCLAIM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_DELETECONTACTCLAIM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_VERIFYCLAIM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ACCEPTVERIFICATION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_SENDCONTACTMESSAGE: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETCONTACTACTIVITY: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            OPTIONAL_SUBOBJECTS(contactevent, RPCResponseAllowedContactEvent());
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETSERVERCONTRACT: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(notary, RPCResponseAllowedServerContract());
            } else {
                CHECK_NONE(notary);
            }

            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETPENDINGPAYMENTS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(
                    accountevent, RPCResponseAllowedAccountEvent());
            } else {
                CHECK_NONE(accountevent);
            }

            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ACCEPTPENDINGPAYMENTS: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_SUBOBJECTS(task, RPCResponseAllowedRPCTask());
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETCOMPATIBLEACCOUNTS: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETWORKFLOW: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);

            if (atLeastOne) {
                CHECK_SUBOBJECTS(workflow, RPCResponseAllowedWorkflow());
            } else {
                CHECK_NONE(workflow);
            }

            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETSERVERPASSWORD: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETADMINNYM: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETUNITDEFINITION: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(unit, RPCResponseAllowedUnitDefinition());
            } else {
                CHECK_NONE(unit);
            }

            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_GETTRANSACTIONDATA: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);

            if (atLeastOne) {
                OPTIONAL_SUBOBJECTS(
                    transactiondata, RPCResponseAllowedTransactionData());
            } else {
                CHECK_NONE(transactiondata);
            }
        } break;
        case RPCCOMMAND_LOOKUPACCOUNTID: {
            CHECK_SIZE(status, 1);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);

            if (atLeastOne) {
                CHECK_IDENTIFIERS(identifier);
            } else {
                CHECK_NONE(identifier);
            }

            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_RENAMEACCOUNT: {
            CHECK_HAVE(status);
            CHECK_SUBOBJECTS(status, RPCResponseAllowedRPCStatus());
            CHECK_NONE(sessions);
            CHECK_NONE(identifier);
            CHECK_NONE(seed);
            CHECK_NONE(nym);
            CHECK_NONE(balance);
            CHECK_NONE(contact);
            CHECK_NONE(accountevent);
            CHECK_NONE(contactevent);
            CHECK_NONE(task);
            CHECK_NONE(notary);
            CHECK_NONE(workflow);
            CHECK_NONE(unit);
            CHECK_NONE(transactiondata);
        } break;
        case RPCCOMMAND_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}

auto CheckProto_3(const RPCResponse& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_4(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const RPCResponse& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
