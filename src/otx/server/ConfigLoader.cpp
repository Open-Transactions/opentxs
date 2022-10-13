// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "otx/server/ConfigLoader.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <exception>

#include "internal/api/Settings.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/server/ServerSettings.hpp"

#define SERVER_WALLET_FILENAME "notaryServer.xml"

namespace opentxs::server
{
auto ConfigLoader::load(
    const api::Session& api,
    const api::Settings& config,
    String& walletFilename) -> bool
{
    try {
        // Setup Config File
        auto strConfigFolder = String::Factory(),
             strConfigFilename = String::Factory();

        // WALLET
        // WALLET FILENAME
        //
        // Clean and Set
        {
            bool bIsNewKey = false;
            auto strValue = String::Factory();
            config.Internal().CheckSet_str(
                String::Factory("wallet"),
                String::Factory("wallet_filename"),
                String::Factory(SERVER_WALLET_FILENAME),
                strValue,
                bIsNewKey);
            walletFilename.Set(strValue);
            {
                LogDetail()(OT_PRETTY_STATIC(ConfigLoader))("Using Wallet: ")(
                    strValue.get())(".")
                    .Flush();
            }
        }
        // CRON
        {
            const char* szComment = ";; CRON  (regular events like market "
                                    "trades and smart contract clauses)\n";

            bool b_SectionExist = false;
            config.Internal().CheckSetSection(
                String::Factory("cron"),
                String::Factory(szComment),
                b_SectionExist);
        }
        {
            const char* szComment =
                "; refill_trans_number is the count of transaction numbers "
                "cron will grab for itself,\n; whenever its supply is getting "
                "low.  If it ever drops below 20% of this count\n; while in "
                "the middle of processing, it will put a WARNING into your "
                "server log.\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("cron"),
                String::Factory("refill_trans_number"),
                500,
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            OTCron::SetCronRefillAmount(static_cast<std::int32_t>(lValue));
        }
        {
            const char* szComment =
                "; ms_between_cron_beats is the number of milliseconds before "
                "Cron processes\n; (all the trades, all the smart contracts, "
                "etc every 10 seconds.)\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("cron"),
                String::Factory("ms_between_cron_beats"),
                10000,
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            OTCron::SetCronMsBetweenProcess(std::chrono::milliseconds(lValue));
        }
        {
            const char* szComment =
                "; max_items_per_nym is the number of cron items (such as "
                "market offers or payment\n; plans) that any given Nym is "
                "allowed to have live and active at the same time.\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("cron"),
                String::Factory("max_items_per_nym"),
                10,
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            OTCron::SetCronMaxItemsPerNym(static_cast<std::int32_t>(lValue));
        }

        // HEARTBEAT
        {
            const char* szComment = ";; HEARTBEAT\n";

            bool bSectionExist = false;
            config.Internal().CheckSetSection(
                String::Factory("heartbeat"),
                String::Factory(szComment),
                bSectionExist);
        }
        {
            const char* szComment =
                "; no_requests is the number of client requests the server "
                "processes per heartbeat.\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("heartbeat"),
                String::Factory("no_requests"),
                10,
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            ServerSettings::SetHeartbeatNoRequests(
                static_cast<std::int32_t>(lValue));
        }
        {
            const char* szComment = "; ms_between_beats is the number of "
                                    "milliseconds between each heartbeat.\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("heartbeat"),
                String::Factory("ms_between_beats"),
                100,
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            ServerSettings::SetHeartbeatMsBetweenBeats(
                static_cast<std::int32_t>(lValue));
        }

        // PERMISSIONS
        {
            const char* szComment =
                ";; PERMISSIONS\n;; You can deactivate server functions here "
                "by setting them to false.\n;; (Even if you do, "
                "override_nym_id will STILL be able to do those functions.)\n";

            bool bSectionExists = false;
            config.Internal().CheckSetSection(
                String::Factory("permissions"),
                String::Factory(szComment),
                bSectionExists);
        }

        {
            auto strValue = String::Factory();
            const char* szValue = nullptr;

            UnallocatedCString stdstrValue = ServerSettings::GetOverrideNymID();
            szValue = stdstrValue.c_str();

            bool bIsNewKey = false;

            if (nullptr == szValue) {
                config.Internal().CheckSet_str(
                    String::Factory("permissions"),
                    String::Factory("override_nym_id"),
                    String::Factory(),
                    strValue,
                    bIsNewKey);
            } else {
                config.Internal().CheckSet_str(
                    String::Factory("permissions"),
                    String::Factory("override_nym_id"),
                    String::Factory(szValue),
                    strValue,
                    bIsNewKey);
            }

            ServerSettings::SetOverrideNymID(strValue->Get());
        }

        // MARKETS
        {
            const char* szComment =
                "; minimum_scale is the smallest allowed power-of-ten for the "
                "scale, for any market.\n; (1oz, 10oz, 100oz, 1000oz.)\n";

            bool bIsNewKey = false;
            std::int64_t lValue = 0;
            config.Internal().CheckSet_long(
                String::Factory("markets"),
                String::Factory("minimum_scale"),
                ServerSettings::GetMinMarketScale(),
                lValue,
                bIsNewKey,
                String::Factory(szComment));
            ServerSettings::SetMinMarketScale(lValue);
        }

        // (#defined right above this function.)
        //
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("admin_usage_credits"),
            ServerSettings::_admin_usage_credits);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("admin_server_locked"),
            ServerSettings::_admin_server_locked);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_usage_credits"),
            ServerSettings::_cmd_usage_credits);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_issue_asset"),
            ServerSettings::_cmd_issue_asset);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_contract"),
            ServerSettings::_cmd_get_contract);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_check_notary_id"),
            ServerSettings::_cmd_check_notary_id);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_create_user_acct"),
            ServerSettings::_cmd_create_user_acct);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_del_user_acct"),
            ServerSettings::_cmd_del_user_acct);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_check_nym"),
            ServerSettings::_cmd_check_nym);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_requestnumber"),
            ServerSettings::_cmd_get_requestnumber);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_trans_nums"),
            ServerSettings::_cmd_get_trans_nums);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_send_message"),
            ServerSettings::_cmd_send_message);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_nymbox"),
            ServerSettings::_cmd_get_nymbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_process_nymbox"),
            ServerSettings::_cmd_process_nymbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_create_asset_acct"),
            ServerSettings::_cmd_create_asset_acct);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_del_asset_acct"),
            ServerSettings::_cmd_del_asset_acct);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_acct"),
            ServerSettings::_cmd_get_acct);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_inbox"),
            ServerSettings::_cmd_get_inbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_outbox"),
            ServerSettings::_cmd_get_outbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_process_inbox"),
            ServerSettings::_cmd_process_inbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_issue_basket"),
            ServerSettings::_cmd_issue_basket);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_exchange_basket"),
            ServerSettings::_transact_exchange_basket);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_notarize_transaction"),
            ServerSettings::_cmd_notarize_transaction);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_process_inbox"),
            ServerSettings::_transact_process_inbox);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_transfer"),
            ServerSettings::_transact_transfer);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_withdrawal"),
            ServerSettings::_transact_withdrawal);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_deposit"),
            ServerSettings::_transact_deposit);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_withdraw_voucher"),
            ServerSettings::_transact_withdraw_voucher);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_pay_dividend"),
            ServerSettings::_transact_pay_dividend);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_deposit_cheque"),
            ServerSettings::_transact_deposit_cheque);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_mint"),
            ServerSettings::_cmd_get_mint);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_withdraw_cash"),
            ServerSettings::_transact_withdraw_cash);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_deposit_cash"),
            ServerSettings::_transact_deposit_cash);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_market_list"),
            ServerSettings::_cmd_get_market_list);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_market_offers"),
            ServerSettings::_cmd_get_market_offers);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_market_recent_trades"),
            ServerSettings::_cmd_get_market_recent_trades);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_get_nym_market_offers"),
            ServerSettings::_cmd_get_nym_market_offers);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_market_offer"),
            ServerSettings::_transact_market_offer);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_payment_plan"),
            ServerSettings::_transact_payment_plan);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_cancel_cron_item"),
            ServerSettings::_transact_cancel_cron_item);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("transact_smart_contract"),
            ServerSettings::_transact_smart_contract);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_trigger_clause"),
            ServerSettings::_cmd_trigger_clause);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_register_contract"),
            ServerSettings::_cmd_register_contract);
        config.Internal().SetOption_bool(
            String::Factory("permissions"),
            String::Factory("cmd_request_admin"),
            ServerSettings::_cmd_request_admin);
        // Done Loading... Lets save any changes...
        if (!config.Internal().Save()) {
            LogError()(OT_PRETTY_STATIC(ConfigLoader))(
                "Error! Unable to save updated Config!!!")
                .Flush();
            OT_FAIL;
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_STATIC(ConfigLoader))(": ")(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::server
