// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/core/String.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTStashItem
{
    const api::Session& api_;
    OTString instrument_definition_id_;
    std::int64_t amount_;

public:
    auto GetAmount() const -> std::int64_t { return amount_; }
    void SetAmount(std::int64_t lAmount) { amount_ = lAmount; }
    auto CreditStash(const std::int64_t& lAmount) -> bool;
    auto DebitStash(const std::int64_t& lAmount) -> bool;
    auto GetInstrumentDefinitionID() -> const String&
    {
        return instrument_definition_id_;
    }
    OTStashItem(const api::Session& api);
    OTStashItem(
        const api::Session& api,
        const String& strInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    OTStashItem(
        const api::Session& api,
        const identifier::Generic& theInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    OTStashItem() = delete;

    virtual ~OTStashItem();
};
}  // namespace opentxs
