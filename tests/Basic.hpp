// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace ot = opentxs;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QObject;

namespace ottest
{
auto Args(bool lowlevel = false, int argc = 0, char** argv = nullptr) noexcept
    -> const ot::Options&;
auto GetQT() noexcept -> QObject*;
auto Home() noexcept -> const ot::UnallocatedCString&;
auto StartQT(bool lowlevel = false) noexcept -> void;
auto StopQT() noexcept -> void;
auto WipeHome() noexcept -> void;
}  // namespace ottest
