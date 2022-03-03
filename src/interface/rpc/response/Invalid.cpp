// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "interface/rpc/response/Invalid.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/rpc/response/Base.hpp"
#include "opentxs/interface/rpc/ResponseCode.hpp"

namespace opentxs::rpc::response
{
Invalid::Invalid(const request::Base& request) noexcept
    : Base(std::make_unique<Imp>(
          this,
          request,
          Responses{{0, ResponseCode::invalid}}))
{
}

Invalid::~Invalid() = default;
}  // namespace opentxs::rpc::response
