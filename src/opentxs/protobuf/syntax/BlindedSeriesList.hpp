// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class BlindedSeriesList;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_2(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_3(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_4(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_5(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_6(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_7(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_8(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_9(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_10(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_11(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_12(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_13(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_14(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_15(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_16(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_17(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_18(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_19(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
auto version_20(
    const BlindedSeriesList& input,
    const Log& log,
    const UnallocatedCString& notary) -> bool;
}  // namespace opentxs::protobuf::inline syntax
