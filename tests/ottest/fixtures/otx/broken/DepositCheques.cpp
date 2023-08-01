// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/otx/broken/DepositCheques.hpp" // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

bool init_{false};

//const bool DepositCheques::have_hd_{
//    ot::api::crypto::HaveHDKeys() &&
//    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)
//};
//
//const ot::UnallocatedCString DepositCheques::SeedA_{""};
//const ot::UnallocatedCString DepositCheques::SeedB_{""};
//const ot::UnallocatedCString DepositCheques::SeedC_{""};
//const ot::identifier::Nym DepositCheques::alice_nym_id_{};
//const ot::identifier::Nym DepositCheques::bob_nym_id_{};
//const ot::identifier::Nym DepositCheques::issuer_nym_id_{};
//ot::identifier::Generic DepositCheques::contact_id_alice_bob_{};
//ot::identifier::Generic DepositCheques::contact_id_alice_issuer_{};
//ot::identifier::Generic DepositCheques::contact_id_bob_alice_{};
//ot::identifier::Generic DepositCheques::contact_id_bob_issuer_{};
//ot::identifier::Generic DepositCheques::contact_id_issuer_alice_{};
//ot::identifier::Generic DepositCheques::contact_id_issuer_bob_{};
//const ot::api::session::Client* DepositCheques::alice_{nullptr};
//const ot::api::session::Client* DepositCheques::bob_{nullptr};
//ot::UnallocatedCString DepositCheques::alice_payment_code_;
//ot::UnallocatedCString DepositCheques::bob_payment_code_;
//ot::UnallocatedCString DepositCheques::issuer_payment_code_;
//ot::identifier::UnitDefinition DepositCheques::unit_id_{};
//ot::identifier::Account DepositCheques::alice_account_id_{};
//ot::identifier::Account DepositCheques::issuer_account_id_{};
//
//DepositCheques::DepositCheques()
//    : alice_client_(OTTestEnvironment::GetOT().StartClientSession(0))
//    , bob_client_(OTTestEnvironment::GetOT().StartClientSession(1))
//    , server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
//    , issuer_client_(OTTestEnvironment::GetOT().StartClientSession(2))
//    , server_contract_(server_1_.Wallet().Internal().Server(server_1_.ID()))
//{
//    if (false == init_) { init(); }
//}
//
//void DepositCheques::import_server_contract(
//    const ot::contract::Server& contract,
//    const ot::api::session::Client& client)
//{
//    auto reason = client.Factory().PasswordPrompt(__func__);
//    auto bytes = ot::Space{};
//    EXPECT_TRUE(server_contract_->Serialize(ot::writer(bytes), true));
//    auto clientVersion =
//        client.Wallet().Internal().Server(ot::reader(bytes));
//
//    client.OTX().SetIntroductionServer(clientVersion);
//}
//
//void DepositCheques::init()
//{
//    const_cast<ot::UnallocatedCString&>(SeedA_) =
//        alice_client_.InternalClient().Exec().Wallet_ImportSeed(
//            "spike nominee miss inquiry fee nothing belt list other "
//            "daughter leave valley twelve gossip paper",
//            "");
//    const_cast<ot::UnallocatedCString&>(SeedB_) =
//        bob_client_.InternalClient().Exec().Wallet_ImportSeed(
//            "trim thunder unveil reduce crop cradle zone inquiry "
//            "anchor skate property fringe obey butter text tank drama "
//            "palm guilt pudding laundry stay axis prosper",
//            "");
//    const_cast<ot::UnallocatedCString&>(SeedC_) =
//        issuer_client_.InternalClient().Exec().Wallet_ImportSeed(
//            "abandon abandon abandon abandon abandon abandon abandon "
//            "abandon abandon abandon abandon about",
//            "");
//    auto reasonA = alice_client_.Factory().PasswordPrompt(__func__);
//    auto reasonB = bob_client_.Factory().PasswordPrompt(__func__);
//    auto reasonI = issuer_client_.Factory().PasswordPrompt(__func__);
//    const_cast<ot::identifier::Nym&>(alice_nym_id_) =
//        alice_client_.Wallet()
//            .Nym({alice_client_.Factory(), SeedA_, 0}, reasonA, ALEX)
//            ->ID();
//    const_cast<ot::identifier::Nym&>(bob_nym_id_) =
//        bob_client_.Wallet()
//            .Nym({bob_client_.Factory(), SeedB_, 0}, reasonB, BOB)
//            ->ID();
//    const_cast<ot::identifier::Nym&>(issuer_nym_id_) =
//        issuer_client_.Wallet()
//            .Nym({issuer_client_.Factory(), SeedC_, 0}, reasonI, ISSUER)
//            ->ID();
//
//    import_server_contract(server_contract_, alice_client_);
//    import_server_contract(server_contract_, bob_client_);
//    import_server_contract(server_contract_, issuer_client_);
//
//    alice_ = &alice_client_;
//    bob_ = &bob_client_;
//
//    init_ = true;
//}

}  // namespace ottest