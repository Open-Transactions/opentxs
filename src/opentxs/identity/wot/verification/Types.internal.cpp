// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/verification/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>
#include <utility>

#include "opentxs/identity/wot/verification/VerificationType.hpp"  // IWYU pragma: keep

namespace opentxs::identity::wot::verification
{
using enum proto::VerificationKind;
using enum identity::wot::verification::Type;
static constexpr auto proto_to_ot_ = frozen::make_unordered_map<
    proto::VerificationKind,
    identity::wot::verification::Type>({
    {VERIFICATIONKIND_ERROR, invalid},
    {VERIFICATIONKIND_AFFIRM, affirm},
    {VERIFICATIONKIND_NEUTRAL, neutral},
    {VERIFICATIONKIND_REFUTE, refute},
});
}  // namespace opentxs::identity::wot::verification

namespace opentxs::identity::wot::verification
{
auto translate(const identity::wot::verification::Type in) noexcept
    -> proto::VerificationKind
{
    using enum proto::VerificationKind;
    static constexpr auto map = frozen::invert_unordered_map(proto_to_ot_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return VERIFICATIONKIND_ERROR;
    }
}
}  // namespace opentxs::identity::wot::verification

namespace opentxs::proto
{
auto translate(const VerificationKind in) noexcept
    -> identity::wot::verification::Type
{
    using enum identity::wot::verification::Type;
    const auto& map = identity::wot::verification::proto_to_ot_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return invalid;
    }
}
}  // namespace opentxs::proto
