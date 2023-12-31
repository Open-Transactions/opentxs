// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Enums.pb.h>
#include <cstdint>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Identifier;
class Signature;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Signature& signature,
    const Log& log,
    const protobuf::Identifier& selfID,
    const protobuf::Identifier& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_1(
    const Signature& signature,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_2(
    const Signature& signature,
    const Log& log,
    const protobuf::Identifier& selfID,
    const protobuf::Identifier& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_2(
    const Signature& signature,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_3(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_3(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_4(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_4(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_5(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_5(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;

auto version_6(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_7(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_8(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_9(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_10(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_11(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_12(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_13(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_14(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_15(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_16(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_17(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_18(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_19(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_20(
    const Signature&,
    const Log& log,
    const protobuf::Identifier&,
    const protobuf::Identifier&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole role = SIGROLE_ERROR) -> bool;

auto version_6(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_7(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_8(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_9(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_10(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_11(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_12(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_13(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_14(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_15(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_16(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_17(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_18(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_19(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
auto version_20(
    const Signature&,
    const Log& log,
    const SignatureRole role = SIGROLE_ERROR) -> bool;
}  // namespace opentxs::protobuf::inline syntax
