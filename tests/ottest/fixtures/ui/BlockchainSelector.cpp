// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/BlockchainSelector.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <atomic>

#include "internal/api/session/UI.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"

namespace ottest
{
namespace ot = opentxs;

Counter BlockchainSelector::counter_full_{};
Counter BlockchainSelector::counter_main_{};
Counter BlockchainSelector::counter_test_{};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"  // NOLINT
BlockchainSelector::BlockchainSelector()
    : client_(OTTestEnvironment::GetOT().StartClientSession(0))
    , full_([&]() -> auto& {
        static std::atomic_bool init{true};
        static auto cb = make_cb(counter_full_, "Blockchain selector (full)");

        if (init) {
            counter_full_.expected_ = 14;
            init = false;
        }

        return client_.UI().Internal().BlockchainSelection(
            ot::ui::Blockchains::All, cb);
    }())
    , main_([&]() -> auto& {
        static std::atomic_bool init{true};
        static auto cb = make_cb(counter_main_, "Blockchain selector (main)");

        if (init) {
            counter_main_.expected_ = 7;
            init = false;
        }

        return client_.UI().Internal().BlockchainSelection(
            ot::ui::Blockchains::Main, cb);
    }())
    , test_([&]() -> auto& {
        static std::atomic_bool init{true};
        static auto cb = make_cb(counter_test_, "Blockchain selector (test)");

        if (init) {
            counter_test_.expected_ = 7;
            init = false;
        }

        return client_.UI().Internal().BlockchainSelection(
            ot::ui::Blockchains::Test, cb);
    }())
{
}
#pragma GCC diagnostic pop
}  // namespace ottest
