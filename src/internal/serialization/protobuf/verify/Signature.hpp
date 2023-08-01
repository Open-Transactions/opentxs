// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Enums.pb.h>
#include <cstdint>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class Identifier;
class Signature;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(
    const Signature& signature,
    const bool silent,
    const proto::Identifier& selfID,
    const proto::Identifier& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_1(
    const Signature& signature,
    const bool silent,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_2(
    const Signature& signature,
    const bool silent,
    const proto::Identifier& selfID,
    const proto::Identifier& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_2(
    const Signature& signature,
    const bool silent,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_3(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_3(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_4(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_4(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_5(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_5(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;

auto CheckProto_6(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_7(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_8(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_9(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_10(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_11(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_12(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_13(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_14(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_15(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_16(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_17(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_18(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_19(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_20(
    const Signature&,
    const bool,
    const proto::Identifier&,
    const proto::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;

auto CheckProto_6(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_7(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_8(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_9(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_10(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_11(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_12(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_13(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_14(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_15(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_16(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_17(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_18(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_19(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto CheckProto_20(
    const Signature&,
    const bool,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
}  // namespace opentxs::proto
