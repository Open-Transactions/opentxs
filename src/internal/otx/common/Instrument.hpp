// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Instrument : public OTScriptable
{
public:
    void Release() override;

    void Release_Instrument();
    auto VerifyCurrentDate() -> bool;  // Verify whether the CURRENT date
                                       // is WITHIN the VALID FROM / TO
                                       // dates.
    auto IsExpired() -> bool;          // Verify whether the CURRENT date is
                                       // AFTER the the "VALID TO" date.
    inline auto GetValidFrom() const -> Time { return m_VALID_FROM; }
    inline auto GetValidTo() const -> Time { return m_VALID_TO; }

    inline auto GetInstrumentDefinitionID() const
        -> const identifier::UnitDefinition&
    {
        return m_InstrumentDefinitionID;
    }
    inline auto GetNotaryID() const -> const identifier::Notary&
    {
        return m_NotaryID;
    }
    void InitInstrument();

    ~Instrument() override;

protected:
    OTUnitID m_InstrumentDefinitionID;
    OTNotaryID m_NotaryID;
    // Expiration Date (valid from/to date)
    // The date, in seconds, when the instrument is valid FROM.
    Time m_VALID_FROM;
    // The date, in seconds, when the instrument expires.
    Time m_VALID_TO;

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    inline void SetValidFrom(const Time TIME_FROM) { m_VALID_FROM = TIME_FROM; }
    inline void SetValidTo(const Time TIME_TO) { m_VALID_TO = TIME_TO; }
    inline void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    {
        m_InstrumentDefinitionID = INSTRUMENT_DEFINITION_ID;
    }
    inline void SetNotaryID(const identifier::Notary& NOTARY_ID)
    {
        m_NotaryID = NOTARY_ID;
    }

    Instrument(const api::Session& api);
    Instrument(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);

private:
    Instrument() = delete;
};
}  // namespace opentxs
