// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/Invalid.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/response/MessagePrivate.hpp"

namespace opentxs::rpc::response
{
Invalid::Invalid(const request::Message& request) noexcept
    : Message(std::make_unique<Imp>(
          this,
          request,
          Responses{{0, ResponseCode::invalid}}))
{
}

Invalid::~Invalid() = default;
}  // namespace opentxs::rpc::response
