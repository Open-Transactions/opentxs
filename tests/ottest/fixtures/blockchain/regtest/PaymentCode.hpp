// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/blockchain/block/Types.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct ScanListener;
struct Server;
struct TXOs;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
class OPENTXS_EXPORT Regtest_payment_code : public Regtest_fixture_normal
{
protected:
    static constexpr auto message_text_{
        "I have come here to chew bubblegum and kick ass...and I'm all out of "
        "bubblegum."};

    static const User alice_;
    static const User bob_;
    static Server server_1_;
    static TXOs txos_alice_;
    static TXOs txos_bob_;
    static std::unique_ptr<ScanListener> listener_alice_p_;
    static std::unique_ptr<ScanListener> listener_bob_p_;

    const ot::api::session::Notary& api_server_1_;
    const ot::identifier::Notary& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const ot::UnallocatedCString expected_display_unit_;
    const ot::UnallocatedCString expected_account_name_;
    const ot::UnallocatedCString expected_notary_name_;
    const ot::UnallocatedCString memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::UnitType expected_unit_type_;
    const Generator mine_to_alice_;
    ScanListener& listener_alice_;
    ScanListener& listener_bob_;

    static auto ExtractElements(
        const ot::blockchain::bitcoin::block::Block& block) noexcept
        -> ot::blockchain::block::Elements;

    auto CheckContactID(
        const User& local,
        const User& remote,
        const ot::UnallocatedCString& paymentcode) const noexcept -> bool;
    auto CheckTXODBAlice() const noexcept -> bool;
    auto CheckTXODBBob() const noexcept -> bool;

    auto ReceiveHD() const noexcept -> const ot::blockchain::crypto::HD&;
    auto ReceivePC() const noexcept
        -> const ot::blockchain::crypto::PaymentCode&;
    auto SendHD() const noexcept -> const ot::blockchain::crypto::HD&;
    auto SendPC() const noexcept -> const ot::blockchain::crypto::PaymentCode&;

    auto Shutdown() noexcept -> void final;

    Regtest_payment_code();
};
}  // namespace ottest
