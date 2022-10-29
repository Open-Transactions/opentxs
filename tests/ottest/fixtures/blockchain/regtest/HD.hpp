// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/blockchain/regtest/Base.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"
#include "ottest/fixtures/common/User.hpp"

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
class OPENTXS_EXPORT Regtest_fixture_hd : public Regtest_fixture_normal
{
protected:
    static const User alice_;
    static TXOs txos_;
    static std::unique_ptr<ScanListener> listener_p_;

    const ot::identifier::Notary& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const ot::UnallocatedCString expected_display_unit_;
    const ot::UnallocatedCString expected_account_name_;
    const ot::UnallocatedCString expected_notary_name_;
    const ot::UnallocatedCString memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::UnitType expected_unit_type_;
    const Generator hd_generator_;
    ScanListener& listener_;

    auto CheckTXODB() const noexcept -> bool;
    auto SendHD() const noexcept -> const ot::blockchain::crypto::HD&;

    auto Shutdown() noexcept -> void final;

    Regtest_fixture_hd();
};
}  // namespace ottest
