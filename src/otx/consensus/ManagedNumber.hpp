// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/otx/consensus/ManagedNumber.hpp"

#include "internal/util/Flag.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs::otx::context
{
class Server;

class ManagedNumber::Imp final
{
public:
    auto operator<(const ManagedNumber&) const noexcept -> bool;
    void SetSuccess(const bool value = true) const;
    auto Valid() const -> bool;
    auto Value() const -> TransactionNumber;

    Imp(const TransactionNumber, otx::context::Server&);
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    virtual ~Imp();

private:
    otx::context::Server& context_;
    const TransactionNumber number_;
    mutable OTFlag success_;
    bool managed_{true};
};
}  // namespace opentxs::otx::context
