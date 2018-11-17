// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/util/Signals.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/OT.hpp"

extern "C" {
#include <signal.h>
}

#define OT_METHOD "opentxs::Signals::"

namespace opentxs
{

const std::map<int, std::function<bool()>> Signals::handler_{
    {1, &Signals::handle_1},   {2, &Signals::handle_2},
    {3, &Signals::handle_3},   {4, &Signals::handle_4},
    {5, &Signals::handle_5},   {6, &Signals::handle_6},
    {7, &Signals::handle_7},   {8, &Signals::handle_8},
    {9, &Signals::handle_9},   {10, &Signals::handle_10},
    {11, &Signals::handle_11}, {12, &Signals::handle_12},
    {13, &Signals::handle_13}, {14, &Signals::handle_14},
    {15, &Signals::handle_15}, {16, &Signals::handle_16},
    {17, &Signals::handle_17}, {18, &Signals::handle_18},
    {19, &Signals::handle_19}, {20, &Signals::handle_20},
    {21, &Signals::handle_21}, {22, &Signals::handle_22},
    {23, &Signals::handle_23}, {24, &Signals::handle_24},
    {25, &Signals::handle_25}, {26, &Signals::handle_26},
    {27, &Signals::handle_27}, {28, &Signals::handle_28},
    {29, &Signals::handle_29}, {30, &Signals::handle_30},
    {31, &Signals::handle_31},
};

Signals::Signals(const Flag& running)
    : running_(running)
    , thread_(nullptr)
{
    thread_.reset(new std::thread(&Signals::handle, this));
}

void Signals::Block()
{
    sigset_t allSignals;
    sigfillset(&allSignals);
    pthread_sigmask(SIG_SETMASK, &allSignals, nullptr);
}

void Signals::handle()
{
    sigset_t allSignals;
    sigfillset(&allSignals);

    while (running_) {
        int sig{0};

        if (0 == sigwait(&allSignals, &sig)) {
            auto shouldBreak = process(sig);

            if (shouldBreak) { break; }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR: Invalid signal received.")
                .Flush();
        }
    }
}

bool Signals::process(const int signal)
{
    auto handler = handler_.find(signal);

    if (handler_.end() == handler) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled signal ")(
            std::to_string(signal))(" received.")
            .Flush();

        return false;
    }

    return std::get<1>(*handler)();
}

bool Signals::shutdown()
{
    OT::Cleanup();
    return true;
}

Signals::~Signals()
{
    if (thread_) { thread_->detach(); }
}
}  // namespace opentxs
