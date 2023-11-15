// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

extern "C" {
#include <unistd.h>
}

namespace opentxs::storage::driver::filesystem
{
auto Common::sync(DescriptorType::handle_type fd) noexcept -> bool
{
    return 0 == ::fsync(fd);
}
}  // namespace opentxs::storage::driver::filesystem
