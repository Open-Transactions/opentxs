// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Enums.pb.h>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Authority;
class Identifier;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_2(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_3(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_4(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_5(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_6(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_7(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_8(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_9(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_10(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_11(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_12(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_13(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_14(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_15(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_16(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_17(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_18(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_19(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
auto version_20(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode = AUTHORITYMODE_ERROR) -> bool;
}  // namespace opentxs::protobuf::inline syntax
