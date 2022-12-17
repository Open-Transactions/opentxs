// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/script/OTScriptable.hpp"
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
    inline auto GetValidFrom() const -> Time { return valid_from_; }
    inline auto GetValidTo() const -> Time { return valid_to_; }

    inline auto GetInstrumentDefinitionID() const
        -> const identifier::UnitDefinition&
    {
        return instrument_definition_id_;
    }
    inline auto GetNotaryID() const -> const identifier::Notary&
    {
        return notary_id_;
    }
    void InitInstrument();

    Instrument() = delete;

    ~Instrument() override;

protected:
    identifier::UnitDefinition instrument_definition_id_;
    identifier::Notary notary_id_;
    // Expiration Date (valid from/to date)
    // The date, in seconds, when the instrument is valid FROM.
    Time valid_from_;
    // The date, in seconds, when the instrument expires.
    Time valid_to_;

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    inline void SetValidFrom(const Time TIME_FROM) { valid_from_ = TIME_FROM; }
    inline void SetValidTo(const Time TIME_TO) { valid_to_ = TIME_TO; }
    inline void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    {
        instrument_definition_id_ = INSTRUMENT_DEFINITION_ID;
    }
    inline void SetNotaryID(const identifier::Notary& NOTARY_ID)
    {
        notary_id_ = NOTARY_ID;
    }

    Instrument(const api::Session& api);
    Instrument(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
};
}  // namespace opentxs
