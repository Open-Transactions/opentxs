// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/Qt.hpp>
#include <opentxs/opentxs.hpp>
#include <QString>
#include <QValidator>
#include <future>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/Pair.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/Client.hpp"
#include "ottest/fixtures/common/Notary.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

class OPENTXS_EXPORT AmountValidator : public Notary_fixture,
                                       public Client_fixture
{
public:
    struct ChainAmounts {
        ot::UnitType unittype_;
        std::vector<std::pair<std::string_view, std::string_view>> amounts_;
        std::vector<
            std::tuple<std::string_view, std::string_view, std::string_view>>
            invalid_amounts_;
    };

    AmountValidator() noexcept = default;
};
}  // namespace ottest
