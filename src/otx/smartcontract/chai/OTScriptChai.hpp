// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string>

#include "internal/otx/smartcontract/OTScript.hpp"
#include "opentxs/Version.hpp"

namespace chaiscript
{
class ChaiScript;
}  // namespace chaiscript

namespace opentxs
{
class OTScriptable;
class OTSmartContract;
class OTVariable;
class String;
}  // namespace opentxs

namespace opentxs
{
class OTScriptChai final : public OTScript
{
public:
    OTScriptChai();
    OTScriptChai(const String& strValue);
    OTScriptChai(const char* new_string);
    OTScriptChai(const char* new_string, size_t sizeLength);
    OTScriptChai(const std::string& new_string);

    ~OTScriptChai() final;

    auto ExecuteScript(OTVariable* pReturnVar = nullptr) -> bool final;
    auto RegisterNativeScriptableCalls(OTScriptable& parent) noexcept
        -> void final;
    auto RegisterNativeSmartContractCalls(OTSmartContract& parent) noexcept
        -> void final;
    chaiscript::ChaiScript* const chai_{nullptr};

private:
    OTScriptChai(const OTScriptChai&) = delete;
    OTScriptChai(OTScriptChai&&) = delete;
    auto operator=(const OTScriptChai&) -> OTScriptChai& = delete;
    auto operator=(OTScriptChai&&) -> OTScriptChai& = delete;
};
}  // namespace opentxs