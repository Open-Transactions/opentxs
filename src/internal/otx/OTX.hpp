// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/otx/ConsensusType.hpp"
// IWYU pragma: no_include "opentxs/otx/LastReplyStatus.hpp"
// IWYU pragma: no_include "opentxs/otx/OTXPushType.hpp"
// IWYU pragma: no_include "opentxs/otx/ServerReplyType.hpp"
// IWYU pragma: no_include "opentxs/otx/ServerRequestType.hpp"

#pragma once

#include "opentxs/otx/Types.hpp"
#include "serialization/protobuf/ConsensusEnums.pb.h"
#include "serialization/protobuf/OTXEnums.pb.h"

namespace opentxs
{
auto translate(const otx::ConsensusType in) noexcept -> proto::ConsensusType;
auto translate(const otx::LastReplyStatus in) noexcept
    -> proto::LastReplyStatus;
auto translate(const otx::OTXPushType in) noexcept -> proto::OTXPushType;
auto translate(const otx::ServerReplyType in) noexcept
    -> proto::ServerReplyType;
auto translate(const otx::ServerRequestType in) noexcept
    -> proto::ServerRequestType;
auto translate(const proto::ConsensusType in) noexcept -> otx::ConsensusType;
auto translate(const proto::LastReplyStatus in) noexcept
    -> otx::LastReplyStatus;
auto translate(const proto::OTXPushType in) noexcept -> otx::OTXPushType;
auto translate(const proto::ServerReplyType in) noexcept
    -> otx::ServerReplyType;
auto translate(const proto::ServerRequestType in) noexcept
    -> otx::ServerRequestType;
}  // namespace opentxs
