// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class OTScript;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto OTScript(const UnallocatedCString& script_type = "")
    -> std::shared_ptr<opentxs::OTScript>;
auto OTScript(
    const UnallocatedCString& script_type,
    const UnallocatedCString& script_contents)
    -> std::shared_ptr<opentxs::OTScript>;
auto OTScriptChai() -> std::shared_ptr<opentxs::OTScript>;
auto OTScriptChai(const UnallocatedCString& script_contents)
    -> std::shared_ptr<opentxs::OTScript>;
}  // namespace opentxs::factory
