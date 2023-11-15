// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Signals.hpp"  // IWYU pragma: associated

#include <csignal>

#include "internal/util/Flag.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Signals::Block() -> void
{
    sigset_t allSignals;
    sigfillset(&allSignals);
    pthread_sigmask(SIG_SETMASK, &allSignals, nullptr);
}

auto Signals::handle() -> void
{
    sigset_t allSignals;
    sigfillset(&allSignals);

    while (running_) {
        int sig{0};

        if (0 == sigwait(&allSignals, &sig)) {
            auto shouldBreak = process(sig);

            if (shouldBreak) { break; }
        } else {
            LogError()()("ERROR: Invalid signal received.").Flush();
        }
    }
}
}  // namespace opentxs
