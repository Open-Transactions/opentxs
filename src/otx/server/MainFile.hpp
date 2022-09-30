// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace server
{
class Server;
}  // namespace server

class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class MainFile
{
public:
    auto CreateMainFile(
        const UnallocatedCString& strContract,
        const UnallocatedCString& strNotaryID,
        const UnallocatedCString& strNymID) -> bool;
    auto LoadMainFile(bool readOnly = false) -> bool;
    auto LoadServerUserAndContract() -> bool;
    auto SaveMainFile() -> bool;
    auto SaveMainFileToString(String& filename) -> bool;

    explicit MainFile(Server& server, const PasswordPrompt& reason);
    MainFile() = delete;
    MainFile(const MainFile&) = delete;
    MainFile(MainFile&&) = delete;
    auto operator=(const MainFile&) -> MainFile& = delete;
    auto operator=(MainFile&&) -> MainFile& = delete;

private:
    Server& server_;
    UnallocatedCString version_;
};
}  // namespace opentxs::server
