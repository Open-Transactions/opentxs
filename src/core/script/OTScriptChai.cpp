// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTScriptChai.hpp"

#if OT_SCRIPT_CHAI
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScript.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <chaiscript/chaiscript.hpp>
#ifdef OT_USE_CHAI_STDLIB
#include <chaiscript/chaiscript_stdlib.hpp>
#endif

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>

#define OT_METHOD "opentxs::OTScriptChai::"

namespace opentxs
{

bool OTScriptChai::ExecuteScript(OTVariable* pReturnVar)
{
    using namespace chaiscript;

    OT_ASSERT(nullptr != chai_);

    if (m_str_script.size() > 0) {

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

        for (auto& it : m_mapParties) {
            OTParty* pParty = it.second;
            OT_ASSERT(nullptr != pParty);

            std::string party_name = pParty->GetPartyName();

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

        for (auto& it : m_mapAccounts) {
            OTPartyAccount* pAcct = it.second;
            OT_ASSERT(nullptr != pAcct);

            std::string acct_name = pAcct->GetName().Get();

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

         OTVariable_Access      GetAccess() const { return m_Access; }

         std::int64_t& GetValueLong() { return m_lValue; }
         bool& GetValueBool() { return m_bValue; }
         std::string& GetValueString() { return m_str_Value; }
         */

        for (auto& it : m_mapVariables) {
            const std::string var_name = it.first;
            OTVariable* pVar = it.second;
            OT_ASSERT((nullptr != pVar) && (var_name.size() > 0));

            switch (pVar->GetType()) {
                case OTVariable::Var_Integer: {
                    std::int32_t& nValue = pVar->GetValueInteger();

                    if (OTVariable::Var_Constant ==
                        pVar->GetAccess())  // no pointer here, since it's
                                            // constant.
                        chai_->add_global_const(
                            const_var(pVar->CopyValueInteger()),
                            var_name.c_str());
                    else
                        chai_->add(
                            var(&nValue),  // passing ptr here so the
                                           // script can modify this
                                           // variable if it wants.
                            var_name.c_str());
                } break;

                case OTVariable::Var_Bool: {
                    bool& bValue = pVar->GetValueBool();

                    if (OTVariable::Var_Constant ==
                        pVar->GetAccess())  // no pointer here, since it's
                                            // constant.
                        chai_->add_global_const(
                            const_var(pVar->CopyValueBool()), var_name.c_str());
                    else
                        chai_->add(
                            var(&bValue),  // passing ptr here so the
                                           // script can modify this
                                           // variable if it wants.
                            var_name.c_str());
                } break;

                case OTVariable::Var_String: {
                    std::string& str_Value = pVar->GetValueString();

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

                default:
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure: Unknown "
                        "variable type for variable: ")(var_name)(".")
                        .Flush();
                    return false;
            }
        }

        // Here we add the mapOfParties user-defined type to the chaiscript
        // engine.
        //      chai_->add(user_type<mapOfParties>(), "mapOfParties");

        // Here we add the m_mapParties member variable itself
        //      chai_->add_global_const(const_var(m_mapParties),
        // "Parties");

        try {
            if (nullptr == pReturnVar)  // Nothing to return.
                chai_->eval(
                    m_str_script.c_str(),
                    exception_specification<const std::exception&>(),
                    m_str_display_filename);

            else  // There's a return variable.
            {
                switch (pReturnVar->GetType()) {
                    case OTVariable::Var_Integer: {
                        std::int32_t nResult = chai_->eval<int32_t>(
                            m_str_script.c_str(),
                            exception_specification<const std::exception&>(),
                            m_str_display_filename);
                        pReturnVar->SetValue(nResult);
                    } break;

                    case OTVariable::Var_Bool: {
                        bool bResult = chai_->eval<bool>(
                            m_str_script.c_str(),
                            exception_specification<const std::exception&>(),
                            m_str_display_filename);
                        pReturnVar->SetValue(bResult);
                    } break;

                    case OTVariable::Var_String: {
                        std::string str_Result = chai_->eval<std::string>(
                            m_str_script.c_str(),
                            exception_specification<const std::exception&>(),
                            m_str_display_filename);
                        pReturnVar->SetValue(str_Result);
                    } break;

                    default:
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Unknown return "
                            "type passed in, "
                            "unable to service it.")
                            .Flush();
                        return false;
                }  // switch
            }      // else return variable.
        }          // try
        catch (const chaiscript::exception::eval_error& ee) {
            // Error in script parsing / execution
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Caught "
                "chaiscript::exception::eval_error: ")(ee.reason)(". File: ")(
                ee.filename)(". Start position, line: ")(
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

                //                const std::string text;
                //                boost::shared_ptr<const std::string> filename;

                for (size_t j = 1; j < ee.call_stack.size(); ++j) {
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
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Caught "
                "chaiscript::exception::bad_boxed_cast : ")(
                (e.what() != nullptr) ? e.what()
                                      : "e.what() returned null, sorry.")
                .Flush();
            return false;
        } catch (const std::exception& e) {
            // Error explicitly thrown from script
            LogOutput(OT_METHOD)(__FUNCTION__)(": Caught std::exception "
                                               "exception: ")(
                (e.what() != nullptr) ? e.what()
                                      : "e.what() returned null, sorry.")
                .Flush();
            return false;
        }
        //      catch (chaiscript::Boxed_Value bv)
        catch (...) {
            //          std::int32_t i = chaiscript::boxed_cast<int32_t>(bv);
            LogOutput(OT_METHOD)(__FUNCTION__)(": Caught exception.").Flush();
            return false;
        }
    }

    return true;
}

#if !defined(OT_USE_CHAI_STDLIB)

OTScriptChai::OTScriptChai()
    : OTScript()
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const OTString& strValue)
    : OTScript(strValue)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const char* new_string)
    : OTScript(new_string)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const char* new_string, size_t sizeLength)
    : OTScript(new_string, sizeLength)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const std::string& new_string)
    : OTScript(new_string)
    , chai_(new chaiscript::ChaiScript)
{
}

#else  // OT_USE_CHAI_STDLIB *is* defined...

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

OTScriptChai::OTScriptChai(const char* new_string, size_t sizeLength)
    : OTScript(new_string, sizeLength)
    , chai_(new chaiscript::ChaiScript)
{
}

OTScriptChai::OTScriptChai(const std::string& new_string)
    : OTScript(new_string)
    , chai_(new chaiscript::ChaiScript)
{
}

#endif  // defined(OT_USE_CHAI_STDLIB)

OTScriptChai::~OTScriptChai()
{
    if (nullptr != chai_) delete chai_;

    // chai = nullptr;  (It's const).
}
}  // namespace opentxs
#endif  // OT_SCRIPT_CHAI
