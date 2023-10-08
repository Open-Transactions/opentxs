// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <string_view>

#include "ottest/fixtures/common/LowLevel.hpp"

namespace ottest
{
class OPENTXS_EXPORT P2PAddress : public LowLevel
{
public:
    static constexpr auto chain_ = opentxs::blockchain::Type::Bitcoin;
    static constexpr auto standard_port_ = 8333;
    static constexpr auto zmq_port_ = 8816;
    static constexpr auto key_z85_ =
        "owom]QR]$mmAC#-}B!qV{:r8Ei>D@4>5g:n^v>bo"sv;
    static constexpr auto endpoint_4_standard_ = "54.39.129.45:8333"sv;
    static constexpr auto endpoint_4_zmq_ = "54.39.129.45:8816"sv;
    static constexpr auto endpoint_6_standard_ = "2607:5300:203:402d:::8333"sv;
    static constexpr auto endpoint_6_zmq_ = "2607:5300:203:402d:::8816"sv;

    static const std::string address_4_;
    static const std::string address_6_;

    const opentxs::ByteArray key_;

    P2PAddress();

    ~P2PAddress() override = default;
};
}  // namespace ottest
