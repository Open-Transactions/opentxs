/************************************************************
*
*  PerformanceLogger.hpp
*
*  Created on: Sep 10, 2014
*      Author: barry
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

#ifndef __OPENTXS_PERFORMANCE_LOGGER_HPP__
#define __OPENTXS_PERFORMANCE_LOGGER_HPP__

#include <cstdio>
#include <cstdint>
#include <syslog.h>
#include "ServerSettings.hpp"

namespace perfLogger
{
// The types of message errors we log. Extend this enum if
// we add logging for additional types of message errors.
enum class MsgErrorType { ReceiveError, MalformedMsgError, SendError };

// The available values for the perlog_level parameter in the
// server.cfg server configuration file
enum class PerfLogLevel {
    DisablePerfLogging = 0, // disables collection of all performance metrics
    LogError,               // log error messages
    LogMsgProcessingTime,   // log message round, and message processing times
    LogTxProcessingTime,    // log transaction processing times
    LogMsgSize,             // log message sizes
    LogEverything           // log everything above
};

// Specifies the event type.
// The performance metrics reporting system filters performance
// data based on these values. These values are also used to
// determine the format of the metrics data.
enum class PerfProbeType {
    MsgProcessingTime = 1,
    MsgRoundTimeRemaining,
    TxProcessingTime,
    MsgSize,
    FileSize,
    MemorySize,
    CPUUtilization,
    ErrorStats
};

enum class MsgRoundEvents {
    MsgRoundStart = 1, // The time we started message round processing
    MsgRoundEnd        // Time we finished message round processing
};

enum class TimingEvents {
    MsgStart = 3,               // 3 The time we started processing a message
    MsgEnd,                     // 4 The time we finished processing a message
    MsgRcvStart,                // 5 The time we started receiving a message
    MsgRcvEnd,                  // 6 The time we finished receiving a message
    MsgSendStart,               // 7 The time we started sending a message
    MsgSendEnd,                 // 8 The time we finished sending a message
    MsgDearmorStart,            // 9 The time we started de-armoring a message
    MsgDearmorEnd,              // 10 The time we finished de-armoring a message
    MsgEnvDecryptionStart,      // 11 The time we started decrypting a message
    MsgEnvDecryptionEnd,        // 12 The time we finished decrypting a message
    ContractInstantiationStart, // 13 The time we started to de-armor the
                                // contract data, parse it, and populate the
                                // associated OTContract instance.
    ContractInstantiationEnd,   // 14 The time we finished de-armoring the
                                // contract data, parsing it, and populating the
                                // associated OTContract instance.
    ContractDecodingStart, // 15 The time we started base64 contract decoding.
    ContractDecodingEnd,   // 16 The time we finished base64 contract decoding.
    ContractParsingStart,  // 17 The time we started to Parse the contract
    ContractParsingEnd,    // 18 The time we finished parsing the contract
    UserCmdStart,          // 19 The time we started processing a user command
    UserCmdEnd,            // 20 The time we finished processing a user command
    ContractSigningStart,  // 21 The time we started signing the contract
    ContractSigningEnd,    // 22 The time we finished signing the contract
    ContractSavingStart,   // 23 The time we started writing the contract to its
                           // m_strRawFile member
    ContractSavingEnd, // 24 The time we finished writing the contract to its
                       // m_strRawFile member
    MsgEnvEncryptionStart, // 25 The time we started encrypting the reply
                           // message envelope
    MsgEnvEncryptionEnd, // 26 The time we finished encrypting the reply message
                         // envelope
    MsgReplyArmoringStart, // 27 The time we started base64 encoding the
                           // encrypted client reply envelope
    MsgReplyArmoringEnd,   // 28 The time we finished base64 encoding the
                           // encrypted client reply envelope
    MsgCheckServerIdStart, // 29 The time we started processing a CheckServerID
                           // message
    MsgCheckServerIdEnd,   // 30 The time we finished processing a CheckServerID
                           // message
    MsgCreateUserAccountStart, // 31 The time we started processing a
                               // CreateUserAccount message
    MsgCreateUserAccountEnd,   // 32 The time we finished processing a
                               // CreateUserAccount message
    MsgGetRequestStart,        // 33 The time we started processing a GetRequest
                               // message
    MsgGetRequestEnd, // 34 The time we finished processing a GetRequest message
    MsgGetTransactionNumStart, // 35 The time we started processing a
                               // GetTransactionNum message
    MsgGetTransactionNumEnd,   // 36 The time we finished processing a
                               // GetTransactionNum message
    MsgCheckUserStart, // 37 The time we started processing a CheckUser message
    MsgCheckUserEnd,   // 38 The time we finished processing a CheckUser message
    MsgSendUserMessageStart,    // 39 The time we started processing a
                                // SendUserMessage message
    MsgSendUserMessageEnd,      // 40 The time we finished processing a
                                // SendUserMessage message
    MsgSendUserInstrumentStart, // 41 The time we started processing a
                                // SendUserInstrument message
    MsgSendUserInstrumentEnd,   // 42 The time we finished processing a
                                // SendUserIntrument message
    MsgDeleteUserAccountStart,  // 43 The time we started processing a
                                // DeleteUserAccount message
    MsgDeleteUserAccountEnd,    // 44 The time we finished processing a
                                // DeleteUserAccount message
    MsgDeleteAssetAccountStart, // 45 The time we started processing a
                                // DeleteAssetAccount message
    MsgDeleteAssetAccountEnd,   // 46 The time we finished processing a
                                // DeleteAssetAccount message
    MsgCreateAccountStart, // 47 The time we started processing a CreateAccount
                           // message
    MsgCreateAccountEnd,   // 48 The time we finished processing a CreateAccount
                           // message
    MsgIssueAssetTypeStart, // 49 The time we started processing an
                            // IssueAssetType message
    MsgIssueAssetTypeEnd,   // 50 The time we finished processing an
                            // IssueAssetType message
    MsgIssueBasketStart,    // 51 The time we started processing an IssueBasket
                            // message
    MsgIssueBasketEnd,      // 52 The time we finished processing an IssueBasket
                            // message
    MsgNotarizeTransactionsStart, // 53 The time we started processing a
                                  // NotarizeTransactions message
    MsgNotarizeTransactionsEnd,   // 54 The time we finished processing a
                                  // NotarizeTransactions message
    MsgGetNymboxStart, // 55 The time we started processing a GetNymbox message
    MsgGetNymboxEnd,   // 56 The time we finished processing a GetNymbox message
    MsgGetBoxReceiptStart, // 57 The time we started processing a GetBoxReceipt
                           // message
    MsgGetBoxReceiptEnd,   // 58 The time we finished processing a GetBoxReceipt
                           // message
    MsgGetInboxStart,  // 59 The time we started processing a GetInbox message
    MsgGetInboxEnd,    // 60 The time we finished processing a GetInbox message
    MsgGetOutboxStart, // 61 The time we started processing a GetOutbox message
    MsgGetOutboxEnd,   // 62 The time we finished processing a GetOutbox message
    MsgGetAccountStart, // 63 The time we started processing a GetAccount
                        // message
    MsgGetAccountEnd, // 64 The time we finished processing a GetAccount message
    MsgGetAccountFilesStart, // 65 The time we started processing a
                             // GetAccountFiles message
    MsgGetAccountFilesEnd,   // 66 The time we finished processing a
                             // GetAccountFiles message
    MsgProcessNymboxStart, // 67 The time we started processing a ProcessNymbox
                           // message
    MsgProcessNymboxEnd,   // 68 The time we finished processing a ProcessNymbox
                           // message
    MsgProcessInboxStart,  // 69 The time we started processing a ProcessInbox
                           // message
    MsgProcessInboxEnd,    // 70 The time we finished processing a ProcessInbox
                           // message
    MsgQueryAssetTypesStart, // 71 The time we started processing a
                             // QueryAssetTypes message
    MsgQueryAssetTypesEnd,   // 72 The time we finished processing a
                             // QueryAssetTypes message
    MsgGetContractStart,     // 73 The time we started processing a GetContract
                             // message
    MsgGetContractEnd,       // 74 The time we finished processing a GetContract
                             // message
    MsgGetMintStart, // 75 The time we started processing a GetMint message
    MsgGetMintEnd,   // 76 The time we finished processing a GetMint message
    MsgGetMarketListStart, // 77 The time we started processing a GetMarketList
                           // message
    MsgGetMarketListEnd,   // 78 The time we finished processing a GetMarketList
                           // message
    MsgGetMarketOffersStart,       // 79 The time we started processing a
                                   // GetMarketOffers message
    MsgGetMarketOffersEnd,         // 80 The time we finished processing a
                                   // GetMarketOffers message
    MsgGetMarketRecentTradesStart, // 81 The time we started processing a
                                   // GetMarketRecentTrades message
    MsgGetMarketRecentTradesEnd,   // 82 The time we finished processing a
                                   // GetMarketRecentTrades message
    MsgGetNymMarketOffersStart,    // 83 The time we started processing a
                                   // GetNym_MarketOffers message
    MsgGetNymMarketOffersEnd,      // 84 The time we finished processing a
                                   // GetNym_MarketOffers message
    MsgTriggerClauseStart, // 85 The time we started processing a TriggerClause
                           // message
    MsgTriggerClauseEnd,   // 86 The time we finished processing a TriggerClause
                           // message
    MsgUsageCreditsStart,  // 87 The time we started processing a UsageCredits
                           // message
    MsgUsageCreditsEnd,    // 88 The time we finished processing a UsageCredits
                           // message
    MsgNotarizeTransactionStart, // 89 The time we started processing a
                                 // NotarizeTransaction message
    MsgNotarizeTransactionEnd,   // 90 The time we finished processing a
                                 // NotarizeTransaction message
    NotarizeTxValidationStart,   // 91 the time we started processing the
                                 // transaction type independent transaction
                                 // security validation logic.
    NotarizeTxValidationEnd,     // 92 The time we finished processing the
                                 // transaction type independent transaction
                                 // security validation logic.
};

enum class TxTimingEvents {
    TxTransferStart =
        100,       // The time we started processing a Transfer transaction
    TxTransferEnd, // The time we finished processing a Transfer transaction
    TxProcessInboxStart, // The time we started processing a ProcessInbox
                         // transaction
    TxProcessInboxEnd,   // The time we finished processing a ProcessInbox
                         // transaction
    TxWithdrawalStart,   // The time we started processing a Withdrawal
                         // transaction
    TxWithdrawalEnd, // The time we finished processing a Withdrawal transaction
    TxDepositStart,  // The time we started processing a Deposit transaction
    TxDepositEnd,    // The time we finished processing a Deposit transaction
    TxPayDividendStart,    // The time we started processing a PayDividend
                           // transaction
    TxPayDividendEnd,      // The time we finished processing a PayDividend
                           // transaction
    TxMarketOfferStart,    // The time we started processing a MarketOffer
                           // transaction
    TxMarketOfferEnd,      // The time we finished processing a MarketOffer
                           // transaction
    TxPaymentPlanStart,    // The time we started processing a PaymentPlan
                           // transaction
    TxPaymentPlanEnd,      // The time we finished processing a PaymentPlan
                           // transaction
    TxSmartContractStart,  // The time we started processing a SmartContract
                           // transaction
    TxSmartContractEnd,    // The time we finished processing a SmartContract
                           // transaction
    TxCancelCronItemStart, // The time we started processing a CancelCronItem
                           // transaction
    TxCancelCronItemEnd,   // The time we finished processing a CancelCronItem
                           // transaction
    TxExchangeBasketStart, // The time we started processing an ExchangeBasket
                           // transaction
    TxExchangeBasketEnd    // The time we finished processing an ExchangeBasket
                           // transaction
};

enum class ValueEvents { MsgRoundTimeRemaining = 1 };

enum class MsgSizeEvents { SizeRecvd = 1, SizeReply };

// Log entry: "opentxs-server
// <version>,PID,eventTimeinMicroseconds,PerfProbeType::ErrorStats,MsgErrorType"
void ProbeError(const MsgErrorType errorType);

// Log entry: "opentxs-server
// <version>,PID,eventTimeinMicroseconds,PerfProbeType::MsgProcessingTime,MsgRoundEvents::msgRoundEventId"
void ProbeMsgRoundTiming(MsgRoundEvents msgRoundEventId);

// Log entry: "opentxs-server <version>,PID,eventTimeinMicroseconds,
//             PerfProbeType::MsgRoundTimeRemaining,ValueEvents::MsgRoundTimeRemaining,msRemaining"
void ProbeMsgRoundTimeRemaining(const int32_t msRemaining);

// Log entry: "opentxs-server <version>,PID,eventTimeinMicroseconds,
//             PerfProbeType::MsgProcessingTime,TimingEvents::eventID"
void ProbeMsgTiming(const TimingEvents eventID);

// Log entry: "opentxs-server <version>,PID,eventTimeinMicroseconds,
//             PerfProbeType::TxProcessingTime,TxTimingEvents::txEventID"
void ProbeTxTiming(const TxTimingEvents txEventID);

// Log entry: "opentxs-server <version>,PID,eventTimeinMicroseconds,
//             PerfProbeType::MsgSize,MsgSizeEvents::msgSizeEventID,msgSize"
void ProbeMsgSize(const MsgSizeEvents msgSizeEventID, const uint32_t msgSize);

void testProbes();

} // namespace perfLogger

#endif // __OPENTXS_PERFORMANCE_LOGGER_HPP__
