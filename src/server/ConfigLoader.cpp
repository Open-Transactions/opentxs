/************************************************************
 *
 *  ConfigLoader.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
**************************************************************/

#include "ConfigLoader.hpp"
#include "ServerSettings.hpp"
#include <opentxs/core/OTString.hpp>
#include <opentxs/core/OTDataFolder.hpp>
#include <opentxs/core/OTSettings.hpp>
#include <opentxs/core/cron/OTCron.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <cstdint>

#define SERVER_WALLET_FILENAME "notaryServer.xml"
#define SERVER_MASTER_KEY_TIMEOUT_DEFAULT -1
#define SERVER_USE_SYSTEM_KEYRING false

namespace opentxs
{

bool ConfigLoader::load(OTString& walletFilename)
{
    const char* szFunc = "ConfigLoader::load()";

    // Setup Config File
    OTString strConfigFolder, strConfigFilename;

    if (!OTDataFolder::IsInitialized()) {
        OT_FAIL;
    }

    // Create Config Object (OTSettings)
    OTString strConfigFilePath = "";
    if (!OTDataFolder::GetConfigFilePath(strConfigFilePath)) {
        OT_FAIL;
    }
    OTSettings* p_Config = new OTSettings(strConfigFilePath);

    // First Load, Create new fresh config file if failed loading.
    if (!p_Config->Load()) {
        OTLog::vOutput(
            0, "%s: Note: Unable to Load Config. Creating a new file: %s\n",
            szFunc, strConfigFilename.Get());
        if (!p_Config->Reset()) return false;
        if (!p_Config->Save()) return false;
    }

    if (!p_Config->Reset()) return false;

    // Second Load, Throw Assert if Failed loading.
    if (!p_Config->Load()) {
        OTLog::vError(0, "%s: Error: Unable to load config file: %s It should "
                         "exist, as we just saved it!\n",
                      szFunc, strConfigFilename.Get());
        OT_FAIL;
    }

    // LOG LEVEL
    {
        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("logging", "log_level", 0, lValue, bIsNewKey);
        OTLog::SetLogLevel(static_cast<int32_t>(lValue));
    }

    // WALLET

    // WALLET FILENAME
    //
    // Clean and Set
    {
        bool bIsNewKey;
        OTString strValue;
        p_Config->CheckSet_str("wallet", "wallet_filename",
                               SERVER_WALLET_FILENAME, strValue, bIsNewKey);
        walletFilename.Set(strValue);
        OTLog::vOutput(0, "Using Wallet: %s\n", strValue.Get());
    }

    // CRON
    {
        const char* szComment = ";; CRON  (regular events like market trades "
                                "and smart contract clauses)\n";

        bool b_SectionExist;
        p_Config->CheckSetSection("cron", szComment, b_SectionExist);
    }

    {
        const char* szComment = "; refill_trans_number is the count of "
                                "transaction numbers cron will grab for "
                                "itself,\n"
                                "; whenever its supply is getting low.  If it "
                                "ever drops below 20% of this count\n"
                                "; while in the middle of processing, it will "
                                "put a WARNING into your server log.\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("cron", "refill_trans_number", 500, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronRefillAmount(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; ms_between_cron_beats is the number of "
                                "milliseconds before Cron processes\n"
                                "; (all the trades, all the smart contracts, "
                                "etc every 10 seconds.)\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("cron", "ms_between_cron_beats", 10000, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronMsBetweenProcess(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; max_items_per_nym is the number of cron "
                                "items (such as market offers or payment\n"
                                "; plans) that any given Nym is allowed to "
                                "have live and active at the same time.\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("cron", "max_items_per_nym", 10, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronMaxItemsPerNym(static_cast<int32_t>(lValue));
    }

    // HEARTBEAT

    {
        const char* szComment = ";; HEARTBEAT\n";

        bool bSectionExist;
        p_Config->CheckSetSection("heartbeat", szComment, bSectionExist);
    }

    {
        const char* szComment = "; no_requests is the number of client "
                                "requests the server processes per "
                                "heartbeat.\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("heartbeat", "no_requests", 10, lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetHeartbeatNoRequests(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; ms_between_beats is the number of "
                                "milliseconds between each heartbeat.\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("heartbeat", "ms_between_beats", 100, lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetHeartbeatMsBetweenBeats(
            static_cast<int32_t>(lValue));
    }

    // PERMISSIONS

    {
        const char* szComment = ";; PERMISSIONS\n"
                                ";; You can deactivate server functions here "
                                "by setting them to false.\n"
                                ";; (Even if you do, override_nym_id will "
                                "STILL be able to do those functions.)\n";

        bool bSectionExists;
        p_Config->CheckSetSection("permissions", szComment, bSectionExists);
    }

    {
        OTString strValue;
        const char* szValue;

        std::string stdstrValue = ServerSettings::GetOverrideNymID();
        szValue = stdstrValue.c_str();

        bool bIsNewKey;

        if (nullptr == szValue)
            p_Config->CheckSet_str("permissions", "override_nym_id", nullptr,
                                   strValue, bIsNewKey);
        else
            p_Config->CheckSet_str("permissions", "override_nym_id", szValue,
                                   strValue, bIsNewKey);

        ServerSettings::SetOverrideNymID(strValue.Get());
    }

    // MARKETS

    {
        const char* szComment = "; minimum_scale is the smallest allowed "
                                "power-of-ten for the scale, for any market.\n"
                                "; (1oz, 10oz, 100oz, 1000oz.)\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("markets", "minimum_scale",
                                ServerSettings::GetMinMarketScale(), lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetMinMarketScale(lValue);
    }

    // SECURITY (beginnings of..)

    // Master Key Timeout
    {
        const char* szComment =
            "; master_key_timeout is how int64_t the master key will be in "
            "memory until a thread wipes it out.\n"
            "; 0   : means you have to type your password EVERY time OT uses a "
            "private key. (Even multiple times in a single function.)\n"
            "; 300 : means you only have to type it once per 5 minutes.\n"
            "; -1  : means you only type it once PER RUN (popular for "
            "servers.)\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("security", "master_key_timeout",
                                SERVER_MASTER_KEY_TIMEOUT_DEFAULT, lValue,
                                bIsNewKey, szComment);
        OTCachedKey::It()->SetTimeoutSeconds(static_cast<int32_t>(lValue));
    }

    // Use System Keyring
    {
        bool bIsNewKey;
        bool bValue;
        p_Config->CheckSet_bool("security", "use_system_keyring",
                                SERVER_USE_SYSTEM_KEYRING, bValue, bIsNewKey);
        OTCachedKey::It()->UseSystemKeyring(bValue);

#if defined(OT_KEYRING_FLATFILE)
        // Is there a password folder? (There shouldn't be, but we allow it...)
        //
        if (bValue) {
            bool bIsNewKey2;
            OTString strValue;
            p_Config->CheckSet_str("security", "password_folder",
                                   SERVER_PASSWORD_FOLDER, strValue,
                                   bIsNewKey2);
            if (strValue.Exists()) {
                OTKeyring::FlatFile_SetPasswordFolder(strValue.Get());
                OTLog::vOutput(0, " Using server password folder: %s\n",
                               strValue.Get());
            }
        }
#endif
    }

    // PERFORMANCE LOGGING

    {
        const char* szComment = ";; PERFORMANCE LOGGING\n";

        bool bSectionExist;
        p_Config->CheckSetSection("perflog", szComment, bSectionExist);
    }

    {
        const char* szComment = "; level 0 = disable performance logging\n"
                                "; level 1 = log errors\n"
                                "; level 2 = log msg processing times\n"
                                "; level 3 = log tx processing times\n"
                                "; level 4 = log msg sizes\n"
                                "; level 5 = log everything\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("perflog", "perflog_level", 0, lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetPerfLogLevel(static_cast<int32_t>(lValue));
    }

    // (#defined right above this function.)
    //

    p_Config->SetOption_bool("permissions", "admin_usage_credits",
                             ServerSettings::__admin_usage_credits);
    p_Config->SetOption_bool("permissions", "admin_server_locked",
                             ServerSettings::__admin_server_locked);
    p_Config->SetOption_bool("permissions", "cmd_usage_credits",
                             ServerSettings::__cmd_usage_credits);
    p_Config->SetOption_bool("permissions", "cmd_issue_asset",
                             ServerSettings::__cmd_issue_asset);
    p_Config->SetOption_bool("permissions", "cmd_get_contract",
                             ServerSettings::__cmd_get_contract);
    p_Config->SetOption_bool("permissions", "cmd_check_server_id",
                             ServerSettings::__cmd_check_server_id);
    p_Config->SetOption_bool("permissions", "cmd_create_user_acct",
                             ServerSettings::__cmd_create_user_acct);
    p_Config->SetOption_bool("permissions", "cmd_del_user_acct",
                             ServerSettings::__cmd_del_user_acct);
    p_Config->SetOption_bool("permissions", "cmd_check_user",
                             ServerSettings::__cmd_check_user);
    p_Config->SetOption_bool("permissions", "cmd_get_request",
                             ServerSettings::__cmd_get_request);
    p_Config->SetOption_bool("permissions", "cmd_get_trans_num",
                             ServerSettings::__cmd_get_trans_num);
    p_Config->SetOption_bool("permissions", "cmd_send_message",
                             ServerSettings::__cmd_send_message);
    p_Config->SetOption_bool("permissions", "cmd_get_nymbox",
                             ServerSettings::__cmd_get_nymbox);
    p_Config->SetOption_bool("permissions", "cmd_process_nymbox",
                             ServerSettings::__cmd_process_nymbox);
    p_Config->SetOption_bool("permissions", "cmd_create_asset_acct",
                             ServerSettings::__cmd_create_asset_acct);
    p_Config->SetOption_bool("permissions", "cmd_del_asset_acct",
                             ServerSettings::__cmd_del_asset_acct);
    p_Config->SetOption_bool("permissions", "cmd_get_acct",
                             ServerSettings::__cmd_get_acct);
    p_Config->SetOption_bool("permissions", "cmd_get_inbox",
                             ServerSettings::__cmd_get_inbox);
    p_Config->SetOption_bool("permissions", "cmd_get_outbox",
                             ServerSettings::__cmd_get_outbox);
    p_Config->SetOption_bool("permissions", "cmd_process_inbox",
                             ServerSettings::__cmd_process_inbox);
    p_Config->SetOption_bool("permissions", "cmd_issue_basket",
                             ServerSettings::__cmd_issue_basket);
    p_Config->SetOption_bool("permissions", "transact_exchange_basket",
                             ServerSettings::__transact_exchange_basket);
    p_Config->SetOption_bool("permissions", "cmd_notarize_transaction",
                             ServerSettings::__cmd_notarize_transaction);
    p_Config->SetOption_bool("permissions", "transact_process_inbox",
                             ServerSettings::__transact_process_inbox);
    p_Config->SetOption_bool("permissions", "transact_transfer",
                             ServerSettings::__transact_transfer);
    p_Config->SetOption_bool("permissions", "transact_withdrawal",
                             ServerSettings::__transact_withdrawal);
    p_Config->SetOption_bool("permissions", "transact_deposit",
                             ServerSettings::__transact_deposit);
    p_Config->SetOption_bool("permissions", "transact_withdraw_voucher",
                             ServerSettings::__transact_withdraw_voucher);
    p_Config->SetOption_bool("permissions", "transact_pay_dividend",
                             ServerSettings::__transact_pay_dividend);
    p_Config->SetOption_bool("permissions", "transact_deposit_cheque",
                             ServerSettings::__transact_deposit_cheque);
    p_Config->SetOption_bool("permissions", "cmd_get_mint",
                             ServerSettings::__cmd_get_mint);
    p_Config->SetOption_bool("permissions", "transact_withdraw_cash",
                             ServerSettings::__transact_withdraw_cash);
    p_Config->SetOption_bool("permissions", "transact_deposit_cash",
                             ServerSettings::__transact_deposit_cash);
    p_Config->SetOption_bool("permissions", "cmd_get_market_list",
                             ServerSettings::__cmd_get_market_list);
    p_Config->SetOption_bool("permissions", "cmd_get_market_offers",
                             ServerSettings::__cmd_get_market_offers);
    p_Config->SetOption_bool("permissions", "cmd_get_market_recent_trades",
                             ServerSettings::__cmd_get_market_recent_trades);
    p_Config->SetOption_bool("permissions", "cmd_get_nym_market_offers",
                             ServerSettings::__cmd_get_nym_market_offers);
    p_Config->SetOption_bool("permissions", "transact_market_offer",
                             ServerSettings::__transact_market_offer);
    p_Config->SetOption_bool("permissions", "transact_payment_plan",
                             ServerSettings::__transact_payment_plan);
    p_Config->SetOption_bool("permissions", "transact_cancel_cron_item",
                             ServerSettings::__transact_cancel_cron_item);
    p_Config->SetOption_bool("permissions", "transact_smart_contract",
                             ServerSettings::__transact_smart_contract);
    p_Config->SetOption_bool("permissions", "cmd_trigger_clause",
                             ServerSettings::__cmd_trigger_clause);

    // Done Loading... Lets save any changes...
    if (!p_Config->Save()) {
        OTLog::vError("%s: Error! Unable to save updated Config!!!\n", szFunc);
        OT_FAIL;
    }

    // Finsihed Saving... now lets cleanup!
    if (!p_Config->Reset()) return false;

    if (nullptr != p_Config) delete p_Config;
    p_Config = nullptr;

    return true;
}

} // namespace opentxs
