// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <memory>
#include <thread>

#include "opentxs/util/Container.hpp"

namespace opentxs
{
class Flag;

class Signals
{
public:
    static void Block();

    Signals(const Flag& running);
    Signals() = delete;
    Signals(const Signals&) = delete;
    Signals(Signals&&) = delete;
    auto operator=(const Signals&) -> Signals& = delete;
    auto operator=(Signals&&) -> Signals& = delete;

    ~Signals();

private:
    static const UnallocatedMap<int, std::function<bool()>> handler_;

    const Flag& running_;
    std::unique_ptr<std::thread> thread_{nullptr};

    /** SIGHUP */
    static auto handle_1() -> bool { return ignore(); }
    /** SIGINT */
    static auto handle_2() -> bool { return shutdown(); }
    /** SIGQUIT */
    static auto handle_3() -> bool { return shutdown(); }
    /** SIGILL */
    static auto handle_4() -> bool { return shutdown(); }
    /** SIGTRAP */
    static auto handle_5() -> bool { return ignore(); }
    /** SIGABRT, SIGIOT */
    static auto handle_6() -> bool { return ignore(); }
    /** SIGBUS */
    static auto handle_7() -> bool { return ignore(); }
    /** SIGFPE */
    static auto handle_8() -> bool { return ignore(); }
    /** SIGKILL */
    static auto handle_9() -> bool { return shutdown(); }
    /** SIGUSR1 */
    static auto handle_10() -> bool { return ignore(); }
    /** SIGSEGV */
    static auto handle_11() -> bool { return ignore(); }
    /** SIGUSR2 */
    static auto handle_12() -> bool { return ignore(); }
    /** SIGPIPE */
    static auto handle_13() -> bool { return ignore(); }
    /** SIGALRM */
    static auto handle_14() -> bool { return ignore(); }
    /** SIGTERM */
    static auto handle_15() -> bool { return shutdown(); }
    /** SIGSTKFLT */
    static auto handle_16() -> bool { return ignore(); }
    /** SIGCLD, SIGCHLD */
    static auto handle_17() -> bool { return ignore(); }
    /** SIGCONT */
    static auto handle_18() -> bool { return ignore(); }
    /** SIGSTOP */
    static auto handle_19() -> bool { return shutdown(); }
    /** SIGTSTP */
    static auto handle_20() -> bool { return ignore(); }
    /** SIGTTIN */
    static auto handle_21() -> bool { return ignore(); }
    /** SIGTTOU */
    static auto handle_22() -> bool { return ignore(); }
    /** SIGURG */
    static auto handle_23() -> bool { return ignore(); }
    /** SIGXCPU */
    static auto handle_24() -> bool { return ignore(); }
    /** SIGXFSZ */
    static auto handle_25() -> bool { return ignore(); }
    /** SIGVTALRM */
    static auto handle_26() -> bool { return ignore(); }
    /** SIGPROF */
    static auto handle_27() -> bool { return ignore(); }
    /** SIGWINCH */
    static auto handle_28() -> bool { return ignore(); }
    /** SIGPOLL, SIGIO */
    static auto handle_29() -> bool { return ignore(); }
    /** SIGPWR */
    static auto handle_30() -> bool { return ignore(); }
    /** SIGSYS, SIGUNUSED */
    static auto handle_31() -> bool { return shutdown(); }
    static auto ignore() -> bool { return false; }
    static auto shutdown() -> bool;

    void handle();
    auto process(const int signal) -> bool;
};
}  // namespace opentxs
