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

#ifndef OPENTXS_CORE_OTINSTRUMENT_HPP
#define OPENTXS_CORE_OTINSTRUMENT_HPP

#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"

#include <stdint.h>

namespace opentxs
{

class Instrument : public OTScriptable
{
public:
    EXPORT Instrument();
    EXPORT Instrument(const Identifier& NOTARY_ID,
                      const Identifier& INSTRUMENT_DEFINITION_ID);
    EXPORT virtual ~Instrument();

    EXPORT void Release() override;

    void Release_Instrument();
    EXPORT bool VerifyCurrentDate(); // Verify whether the CURRENT date is
                                     // WITHIN the VALID FROM / TO dates.
    EXPORT bool IsExpired(); // Verify whether the CURRENT date is AFTER the the
                             // "VALID TO" date.
    inline time64_t GetValidFrom() const
    {
        return m_VALID_FROM;
    }
    inline time64_t GetValidTo() const
    {
        return m_VALID_TO;
    }

    inline const Identifier& GetInstrumentDefinitionID() const
    {
        return m_InstrumentDefinitionID;
    }
    inline const Identifier& GetNotaryID() const
    {
        return m_NotaryID;
    }
    void InitInstrument();

protected:
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    inline void SetValidFrom(time64_t TIME_FROM)
    {
        m_VALID_FROM = TIME_FROM;
    }
    inline void SetValidTo(time64_t TIME_TO)
    {
        m_VALID_TO = TIME_TO;
    }
    inline void SetInstrumentDefinitionID(
        const Identifier& INSTRUMENT_DEFINITION_ID)
    {
        m_InstrumentDefinitionID = INSTRUMENT_DEFINITION_ID;
    }
    inline void SetNotaryID(const Identifier& NOTARY_ID)
    {
        m_NotaryID = NOTARY_ID;
    }

protected:
    Identifier m_InstrumentDefinitionID; // Every cheque or cash note has an
                                         // Asset Type
    Identifier m_NotaryID;               // ...As well as a Notary ID...
    // Expiration Date (valid from/to date)
    time64_t m_VALID_FROM{0}; // The date, in seconds, when the instrument is valid
                           // FROM.
    time64_t m_VALID_TO{0};   // The date, in seconds, when the instrument expires.
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTINSTRUMENT_HPP
