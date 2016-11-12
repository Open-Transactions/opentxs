/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP
#define OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP

#include "opentxs/core/script/OTScript.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702) // warning C4702: unreachable code
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef OT_USE_SCRIPT_CHAI

// SUBCLASS:  CHAI SCRIPT

namespace chaiscript
{
class ChaiScript;
}

namespace opentxs
{

class OTScriptChai : public OTScript
{
public:
    OTScriptChai();
    OTScriptChai(const String& strValue);
    OTScriptChai(const char* new_string);
    OTScriptChai(const char* new_string, size_t sizeLength);
    OTScriptChai(const std::string& new_string);

    virtual ~OTScriptChai();

    virtual bool ExecuteScript(OTVariable* pReturnVar = nullptr);
    chaiscript::ChaiScript* const chai;
};

    
} // namespace opentxs

#endif // OT_USE_SCRIPT_CHAI

#endif // OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP
