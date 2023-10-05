// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#pragma once

#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/regtest/PaymentCode.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct ScanListener;
struct TXOs;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
class OPENTXS_EXPORT Regtest_multiple_payment_code : public Regtest_payment_code
{
protected:
    static const User chris_;
    static const User daniel_;
    static TXOs txos_chris_;
    static TXOs txos_daniel_;

    ScanListener& listener_chris_;
    ScanListener& listener_daniel_;

    auto ChrisHD() const noexcept -> const ot::blockchain::crypto::HD&;
    auto ChrisPC() const noexcept -> const ot::blockchain::crypto::PaymentCode&;
    auto DanielHD() const noexcept -> const ot::blockchain::crypto::HD&;
    auto DanielPC() const noexcept
        -> const ot::blockchain::crypto::PaymentCode&;

    auto Shutdown() noexcept -> void final;

    Regtest_multiple_payment_code();

private:
    static bool init_multiple_;
    static std::unique_ptr<ScanListener> listener_chris_p_;
    static std::unique_ptr<ScanListener> listener_daniel_p_;
};
}  // namespace ottest
