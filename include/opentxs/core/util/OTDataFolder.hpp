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

#ifndef OPENTXS_CORE_OTDATAFOLDER_HPP
#define OPENTXS_CORE_OTDATAFOLDER_HPP

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{

// cppcheck-suppress noConstructor
class OTDataFolder
{
private:
    static OTDataFolder* pDataFolder;

    bool m_bInitialized;

    String m_strDataFolderPath;
    String m_strDataConfigFilePath;

public:
    EXPORT static bool Init(const String& strThreadContext);

    EXPORT static bool IsInitialized();

    EXPORT static bool Cleanup();

    EXPORT static String Get();
    EXPORT static bool Get(String& strDataFolder);

    EXPORT static bool GetConfigFilePath(String& strConfigFilePath);
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTDATAFOLDER_HPP
