// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class ConnectionInfoReply;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(const ConnectionInfoReply& reply, const bool silent) -> bool;
auto CheckProto_2(const ConnectionInfoReply& reply, const bool silent) -> bool;
auto CheckProto_3(const ConnectionInfoReply& reply, const bool silent) -> bool;
auto CheckProto_4(const ConnectionInfoReply& reply, const bool silent) -> bool;
auto CheckProto_5(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_6(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_7(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_8(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_9(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_10(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_11(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_12(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_13(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_14(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_15(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_16(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_17(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_18(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_19(const ConnectionInfoReply&, const bool) -> bool;
auto CheckProto_20(const ConnectionInfoReply&, const bool) -> bool;
}  // namespace opentxs::proto
