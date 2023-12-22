// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/smartcontract/chai/OTScriptChai.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "ChaiScript.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/smartcontract/Factory.hpp"
#include "internal/otx/smartcontract/OTParty.hpp"
#include "internal/otx/smartcontract/OTPartyAccount.hpp"
#include "internal/otx/smartcontract/OTScript.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/otx/smartcontract/OTVariable.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto OTScriptChai() -> std::shared_ptr<opentxs::OTScript>
{
    return std::make_shared<opentxs::OTScriptChai>();
}

auto OTScriptChai(const UnallocatedCString& script_contents)
    -> std::shared_ptr<opentxs::OTScript>
{
    return std::make_shared<opentxs::OTScriptChai>(script_contents);
}
}  // namespace opentxs::factory

namespace opentxs
{
auto OTScriptChai::ExecuteScript(OTVariable* pReturnVar) -> bool
{
    using namespace chaiscript;

    assert_false(nullptr == chai_);

    if (script_.size() > 0) {

        /*
        chai_->add(user_type<OTParty>(), "OTParty");
        chai_->add(constructor<OTParty()>(), "OTParty");
        chai_->add(constructor<OTParty(const OTParty&)>(), "OTParty");
        chai_->add(fun<OTParty&(OTParty::*)(const
        OTParty&)>(&OTParty::operator=), "=");

        chai_->add(fun(&OTParty::GetPartyName), "GetPartyName");
        chai_->add(fun(&OTParty::GetNymID), "GetNymID");
        chai_->add(fun(&OTParty::GetEntityID), "GetEntityID");
        chai_->add(fun(&OTParty::GetPartyID), "GetPartyID");
        chai_->add(fun(&OTParty::HasActiveAgent), "HasActiveAgent");
        */

        // etc

        //      chai_->add(m); // Here we add the OTParty class to the
        //      chaiscript engine.

        for (auto& it : parties_) {
            OTParty* pParty = it.second;
            assert_false(nullptr == pParty);

            const UnallocatedCString party_name = pParty->GetPartyName();

            //          std::cerr << " TESTING PARTY: " << party_name <<
            //            std::endl;
            //            chai_->add(chaiscript::var(&d), "d");

            // Currently I don't make the entire party available -- just his ID.
            //
            // Update: The client side uses constant variables only (a block or
            // two down
            // from here) and it stores the user's ID, and acct IDs, directly in
            // those
            // variables based on name.  Whereas the server side passes in
            // Parties and
            // PartyAccounts, and only the names are made available inside the
            // scripts.
            // This way the scripts must entirely rely on the server-side API,
            // functions
            // such as move_funds(from_name, to_name), which expect a name, and
            // translate
            // only internally to resolve the ID.
            // (Contrast this with client-side scripts, which actually have the
            // real ID
            // available inside the script, and which can call any OT API
            // function that
            // exists...)
            //
            chai_->add_global_const(
                const_var(party_name),
                party_name.c_str());  // Why name and not
                                      // ID? See comment
                                      // just above.
        }

        for (auto& it : accounts_) {
            OTPartyAccount* pAcct = it.second;
            assert_false(nullptr == pAcct);

            const UnallocatedCString acct_name = pAcct->GetName().Get();

            //          std::cerr << " TESTING ACCOUNT: " << acct_name <<
            //            std::endl;
            //            chai_->add(chaiscript::var(&d), "d");

            // Currently I don't make the entire account available -- just his
            // ID.
            //
            chai_->add_global_const(
                const_var(acct_name),
                acct_name.c_str());  // See comment in
                                     // above block for
                                     // party name.
        }

        /*
         enum OTVariable_Access
         {
             Var_Constant,        // Constant -- you cannot change this value.
             Var_Persistent,    // Persistent -- changing value doesn't require
         notice to parties.
             Var_Important,        // Important -- changing value requires
         notice to parties.
             Var_Error_Access    // should never happen.
         };

         OTVariable_Access      GetAccess() const { return access_; }

         std::int64_t& GetValueLong() { return value_; }
         bool& GetValueBool() { return bool_; }
         UnallocatedCString& GetValueString() { return string_; }
         */

        for (auto& it : variables_) {
            const UnallocatedCString var_name = it.first;
            OTVariable* pVar = it.second;
            assert_true((nullptr != pVar) && (var_name.size() > 0));

            switch (pVar->GetType()) {
                case OTVariable::Var_Integer: {
                    std::int32_t& nValue = pVar->GetValueInteger();

                    if (OTVariable::Var_Constant ==
                        pVar->GetAccess()) {  // no pointer here, since it's
                                              // constant.
                        chai_->add_global_const(
                            const_var(pVar->CopyValueInteger()),
                            var_name.c_str());
                    } else {
                        chai_->add(
                            var(&nValue),  // passing ptr here so the
                                           // script can modify this
                                           // variable if it wants.
                            var_name.c_str());
                    }
                } break;
                case OTVariable::Var_Bool: {
                    bool& bValue = pVar->GetValueBool();

                    if (OTVariable::Var_Constant ==
                        pVar->GetAccess()) {  // no pointer here, since it's
                                              // constant.
                        chai_->add_global_const(
                            const_var(pVar->CopyValueBool()), var_name.c_str());
                    } else {
                        chai_->add(
                            var(&bValue),  // passing ptr here so the
                                           // script can modify this
                                           // variable if it wants.
                            var_name.c_str());
                    }
                } break;
                case OTVariable::Var_String: {
                    UnallocatedCString& str_Value = pVar->GetValueString();

                    if (OTVariable::Var_Constant ==
                        pVar->GetAccess())  // no pointer here, since it's
                                            // constant.
                    {
                        chai_->add_global_const(
                            const_var(pVar->CopyValueString()),
                            var_name.c_str());

                        //                      otErr << "\n\n\nOTSCRIPT
                        //                      DEBUGGING
                        // (const var added to script): %s\n\n\n",
                        // str_Value.c_str());
                    } else {
                        chai_->add(
                            var(&str_Value),  // passing ptr here so the
                                              // script can modify this
                                              // variable if it wants.
                            var_name.c_str());

                        //                      otErr << "\n\n\nOTSCRIPT
                        //                      DEBUGGING
                        // var added to script: %s \n\n\n", str_Value.c_str());
                    }
                } break;
                case OTVariable::Var_Error_Type:
                default:
                    LogError()()("Failure: Unknown "
                                 "variable type for variable: ")(var_name)(".")
                        .Flush();
                    return false;
            }
        }

        // Here we add the mapOfParties user-defined type to the chaiscript
        // engine.
        //      chai_->add(user_type<mapOfParties>(), "mapOfParties");

        // Here we add the parties_ member variable itself
        //      chai_->add_global_const(const_var(parties_),
        // "Parties");

        try {
            if (nullptr == pReturnVar) {  // Nothing to return.
                chai_->eval(
                    script_.c_str(),
                    exception_specification<const std::exception&>(),
                    display_filename_);

            } else  // There's a return variable.
            {
                switch (pReturnVar->GetType()) {
                    case OTVariable::Var_Integer: {
                        auto nResult = chai_->eval<int32_t>(
                            script_.c_str(),
                            exception_specification<const std::exception&>(),
                            display_filename_);
                        pReturnVar->SetValue(nResult);
                    } break;
                    case OTVariable::Var_Bool: {
                        const bool bResult = chai_->eval<bool>(
                            script_.c_str(),
                            exception_specification<const std::exception&>(),
                            display_filename_);
                        pReturnVar->SetValue(bResult);
                    } break;
                    case OTVariable::Var_String: {
                        auto str_Result = chai_->eval<UnallocatedCString>(
                            script_.c_str(),
                            exception_specification<const std::exception&>(),
                            display_filename_);
                        pReturnVar->SetValue(str_Result);
                    } break;
                    case OTVariable::Var_Error_Type:
                    default:
                        LogError()()("Unknown return "
                                     "type passed in, "
                                     "unable to service it.")
                            .Flush();
                        return false;
                }  // switch
            }      // else return variable.
        }          // try
        catch (const chaiscript::exception::eval_error& ee) {
            // Error in script parsing / execution
            LogError()()("Caught "
                         "chaiscript::exception::eval_error: ")(ee.reason)(
                ". File: ")(ee.filename)(". Start position, line: ")(
                ee.start_position.line)(". Column: ")(ee.start_position.column)(
                ".")
                .Flush();
            //                  << "\n"
            //                     "   End position,   line: " <<
            //                     ee.end_position.line
            //                  << " column: " << ee.end_position.column

            std::cout << ee.what();
            if (ee.call_stack.size() > 0) {
                std::cout << "during evaluation at ("
                          << ee.call_stack[0].start().line << ", "
                          << ee.call_stack[0].start().column << ")";
            }
            std::cout << std::endl;
            std::cout << std::endl;

            //          std::cout << ee.what();
            if (ee.call_stack.size() > 0) {
                //                std::cout << "during evaluation at (" <<
                // *(ee.call_stack[0]->filename) << " " <<
                // ee.call_stack[0]->start().line << ", " <<
                // ee.call_stack[0]->start().column << ")";

                //                const UnallocatedCString text;
                //                std::shared_ptr<const UnallocatedCString>
                //                filename;

                for (auto j = 1_uz; j < ee.call_stack.size(); ++j) {
                    if (ee.call_stack[j].identifier !=
                            chaiscript::AST_Node_Type::Block &&
                        ee.call_stack[j].identifier !=
                            chaiscript::AST_Node_Type::File) {
                        std::cout << std::endl;
                        std::cout << "  from " << ee.call_stack[j].filename()
                                  << " (" << ee.call_stack[j].start().line
                                  << ", " << ee.call_stack[j].start().column
                                  << ") : ";
                        std::cout << ee.call_stack[j].text << std::endl;
                    }
                }
            }
            std::cout << std::endl;

            return false;
        } catch (const chaiscript::exception::bad_boxed_cast& e) {
            // Error unboxing return value
            LogError()()("Caught "
                         "chaiscript::exception::bad_boxed_cast : ")(
                (e.what() != nullptr) ? e.what()
                                      : "e.what() returned null, sorry.")
                .Flush();
            return false;
        } catch (const std::exception& e) {
            // Error explicitly thrown from script
            LogError()()("Caught std::exception "
                         "exception: ")(
                (e.what() != nullptr) ? e.what()
                                      : "e.what() returned null, sorry.")
                .Flush();
            return false;
        }
        //      catch (chaiscript::Boxed_Value bv)
        catch (...) {
            //          std::int32_t i = chaiscript::boxed_cast<int32_t>(bv);
            LogError()()("Caught exception.").Flush();
            return false;
        }
    }

    return true;
}

auto OTScriptChai::RegisterNativeScriptableCalls(OTScriptable& parent) noexcept
    -> void
{
    using namespace chaiscript;

    assert_false(nullptr == chai_);

    chai_->add(fun(&OTScriptable::GetTime), "get_time");
    chai_->add(
        fun(&OTScriptable::CanExecuteClause, &parent),
        "party_may_execute_clause");
}

auto OTScriptChai::RegisterNativeSmartContractCalls(
    OTSmartContract& parent) noexcept -> void
{
    using OT_SM_RetBool_ThrStr = bool (OTSmartContract::*)(
        UnallocatedCString, UnallocatedCString, UnallocatedCString);

    using namespace chaiscript;

    assert_false(nullptr == chai_);

    // OT NATIVE FUNCTIONS
    // (These functions can be called from INSIDE the scripted clauses.)
    //                                                                                        //
    // Parameters must match as described below. Return value will be as
    // described below.
    //
    //      chai_->add(base_class<OTScriptable,
    //      OTSmartContract>());

    chai_->add(
        fun<OT_SM_RetBool_ThrStr>(&OTSmartContract::MoveAcctFundsStr, &parent),
        "move_funds");

    chai_->add(fun(&OTSmartContract::StashAcctFunds, &parent), "stash_funds");
    chai_->add(
        fun(&OTSmartContract::UnstashAcctFunds, &parent), "unstash_funds");
    chai_->add(
        fun(&OTSmartContract::GetAcctBalance, &parent), "get_acct_balance");
    chai_->add(
        fun(&OTSmartContract::GetUnitTypeIDofAcct, &parent),
        "get_acct_instrument_definition_id");
    chai_->add(
        fun(&OTSmartContract::GetStashBalance, &parent), "get_stash_balance");
    chai_->add(
        fun(&OTSmartContract::SendNoticeToParty, &parent), "send_notice");
    chai_->add(
        fun(&OTSmartContract::SendANoticeToAllParties, &parent),
        "send_notice_to_parties");
    chai_->add(
        fun(&OTSmartContract::SetRemainingTimer, &parent),
        "set_seconds_until_timer");
    chai_->add(
        fun(&OTSmartContract::GetRemainingTimer, &parent),
        "get_remaining_timer");

    chai_->add(
        fun(&OTSmartContract::DeactivateSmartContract, &parent),
        "deactivate_contract");

    // CALLBACKS
    // (Called by OT at key moments) todo security: What if these are
    // recursive? Need to lock down, put the smack down, on these smart
    // contracts.
    //
    // FYI:    chai_->add(fun(&(OTScriptable::CanExecuteClause),
    // (*this)), "party_may_execute_clause");    // From OTScriptable (FYI)
    // param_party_name and param_clause_name will be available inside
    // script. Script must return bool.
    // FYI:    #define SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
    // "callback_party_may_execute_clause"   <=== THE CALLBACK WITH THIS
    // NAME must be connected to a script clause, and then the clause will
    // trigger when the callback is needed.

    chai_->add(
        fun(&OTSmartContract::CanCancelContract, &parent),
        "party_may_cancel_contract");  // param_party_name
                                       // will be available
                                       // inside script.
                                       // Script must return
                                       // bool.
    // FYI:    #define SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL
    // "callback_party_may_cancel_contract"  <=== THE CALLBACK WITH THIS
    // NAME must be connected to a script clause, and then the clause will
    // trigger when the callback is needed.

    // Callback USAGE:    Your clause, in your smart contract, may have
    // whatever name you want. (Within limits.)
    //                    There must be a callback entry in the smart
    // contract, linking your clause the the appropriate callback.
    //                    The CALLBACK ENTRY uses the names
    // "callback_party_may_execute_clause" and
    // "callback_party_may_cancel_contract".
    //                    If you want to call these from INSIDE YOUR SCRIPT,
    // then use the names "party_may_execute_clause" and
    // "party_may_cancel_contract".

    // HOOKS:
    //
    // Hooks are not native calls needing to be registered with the script.
    // (Like the above functions are.)
    // Rather, hooks are SCRIPT CLAUSES, that you have a CHOICE to provide
    // inside your SMART CONTRACT.
    // *IF* you have provided those clauses, then OT *WILL* call them, at
    // the appropriate times. (When
    // specific events occur.) Specifically, Hook entries must be in your
    // smartcontract, linking the below
    // standard hooks to your clauses.
    //
    // FYI:    #define SMARTCONTRACT_HOOK_ON_PROCESS        "cron_process"
    // // Called regularly in OTSmartContract::ProcessCron() based on
    // SMART_CONTRACT_PROCESS_INTERVAL.
    // FYI:    #define SMARTCONTRACT_HOOK_ON_ACTIVATE        "cron_activate"
    // // Done. This is called when the contract is first activated.
}

OTScriptChai::OTScriptChai()
    : OTScript()
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const String& strValue)
    : OTScript(strValue)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const char* new_string)
    : OTScript(new_string)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const char* new_string, std::size_t sizeLength)
    : OTScript(new_string, sizeLength)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const UnallocatedCString& new_string)
    : OTScript(new_string)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::~OTScriptChai()
{
    if (nullptr != chai_) { delete chai_; }

    // chai = nullptr;  (It's const).
}
}  // namespace opentxs
