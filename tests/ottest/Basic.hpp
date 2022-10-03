// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Options;
}  // namespace opentxs

class QObject;
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;
namespace fs = std::filesystem;

namespace ottest
{
auto Args(bool lowlevel = false, int argc = 0, char** argv = nullptr) noexcept
    -> const ot::Options&;
auto GetQT() noexcept -> QObject*;
auto Home() noexcept -> const fs::path&;
auto StartQT(bool lowlevel = false) noexcept -> void;
auto StopQT() noexcept -> void;
auto WipeHome() noexcept -> void;
}  // namespace ottest
