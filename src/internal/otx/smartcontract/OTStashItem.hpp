// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/core/String.hpp"

namespace opentxs
{
class Identifier;

class OTStashItem
{
    OTString m_strInstrumentDefinitionID;
    std::int64_t m_lAmount;

public:
    auto GetAmount() const -> std::int64_t { return m_lAmount; }
    void SetAmount(std::int64_t lAmount) { m_lAmount = lAmount; }
    auto CreditStash(const std::int64_t& lAmount) -> bool;
    auto DebitStash(const std::int64_t& lAmount) -> bool;
    auto GetInstrumentDefinitionID() -> const String&
    {
        return m_strInstrumentDefinitionID;
    }
    OTStashItem();
    OTStashItem(
        const String& strInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    OTStashItem(
        const Identifier& theInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    virtual ~OTStashItem();
};

}  // namespace opentxs
