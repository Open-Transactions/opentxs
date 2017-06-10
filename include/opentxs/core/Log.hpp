/************************************************************
 *
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
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_OTLOG_HPP
#define OPENTXS_CORE_OTLOG_HPP

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

#include <chrono>
#include <cstdint>
#include <deque>
#include <iostream>
#include <thread>

#if defined(unix) || defined(__unix__) || defined(__unix) ||                   \
    defined(__APPLE__) || defined(linux) || defined(__linux) ||                \
    defined(__linux__)
#define PREDEF_PLATFORM_UNIX 1
#endif

#if defined(debug) || defined(_DEBUG) || defined(DEBUG)
#define PREDEF_MODE_DEBUG 1
#endif

namespace opentxs
{

typedef std::deque<String*> dequeOfStrings;

class OTLogStream;

#ifdef _WIN32
#ifdef OTLOG_IMPORT
#undef OTLOG_IMPORT
#define OTLOG_IMPORT __declspec(dllexport)
#else
#define OTLOG_IMPORT __declspec(dllimport)
#endif
#else
#ifndef OTLOG_IMPORT
#define OTLOG_IMPORT
#endif
#endif

OTLOG_IMPORT extern OTLogStream otErr;   // logs using OTLog::vError()
OTLOG_IMPORT extern OTLogStream otInfo;  // logs using OTLog::vOutput(2)
OTLOG_IMPORT extern OTLogStream otOut;   // logs using OTLog::vOutput(0)
OTLOG_IMPORT extern OTLogStream otWarn;  // logs using OTLog::vOutput(1)
OTLOG_IMPORT extern OTLogStream otLog3;  // logs using OTLog::vOutput(3)
OTLOG_IMPORT extern OTLogStream otLog4;  // logs using OTLog::vOutput(4)
OTLOG_IMPORT extern OTLogStream otLog5;  // logs using OTLog::vOutput(5)

class OTLogStream : public std::ostream, std::streambuf
{
private:
    int logLevel{0};
    int next{0};
    char* pBuffer{nullptr};

public:
    explicit OTLogStream(int _logLevel);
    ~OTLogStream();

    virtual int overflow(int c) override;
};

// cppcheck-suppress noConstructor
class Log
{
private:
    static Log* pLogger;

    static const String m_strVersion;
    static const String m_strPathSeparator;

    dequeOfStrings logDeque;

    String m_strThreadContext;
    String m_strLogFileName;
    String m_strLogFilePath;

    int32_t m_nLogLevel{0};

    bool m_bInitialized{false};

    /** For things that represent internal inconsistency in the code. Normally
     * should NEVER happen even with bad input from user. (Don't call this
     * directly. Use the above #defined macro instead.) */
    static Assert::fpt_Assert_sz_n_sz(logAssert);

    static bool CheckLogger(Log* pLogger);

public:
    /** now the logger checks the global config file itself for the
     * log-filename. */
    EXPORT static bool Init(
        const String& strThreadContext = "",
        const int32_t& nLogLevel = 0);

    EXPORT static bool IsInitialized();

    EXPORT static bool Cleanup();

    // OTLog Constants.
    //

    // Compiled into OTLog:

    EXPORT static const char* Version();
    EXPORT static const String& GetVersion();

    EXPORT static const char* PathSeparator();
    EXPORT static const String& GetPathSeparator();

    // Set in constructor:

    EXPORT static const String& GetThreadContext();

    EXPORT static const char* LogFilePath();
    EXPORT static const String& GetLogFilePath();

    EXPORT static int32_t LogLevel();
    EXPORT static bool SetLogLevel(const int32_t& nLogLevel);

    // OTLog Functions:
    //

    EXPORT static bool LogToFile(const String& strOutput);

    /** We keep 1024 logs in memory, to make them available via the API. */
    EXPORT static int32_t GetMemlogSize();
    EXPORT static String GetMemlogAtIndex(int32_t nIndex);
    EXPORT static String PeekMemlogFront();
    EXPORT static String PeekMemlogBack();
    EXPORT static bool PopMemlogFront();
    EXPORT static bool PopMemlogBack();
    EXPORT static bool PushMemlogFront(const String& strLog);
    EXPORT static bool Sleep(const std::chrono::microseconds us);

    /** Output() logs normal output, which carries a verbosity level. If
     * nVerbosity of a message is 0, the message will ALWAYS log. (ALL output
     * levels are higher or equal to 0.) If nVerbosity is 1, the message will
     * run only if __CurrentLogLevel is 1 or higher. If nVerbosity if 2, the
     * message will run only if __CurrentLogLevel is 2 or higher. Etc.
     * THEREFORE: The higher the verbosity level for a message, the more verbose
     * the software must be configured in order to display that message. Default
     * verbosity level for the software is 0, and output that MUST appear on the
     * screen should be set at level 0. For output that you don't want to see as
     * often, set it up to 1. Set it up even higher for the really verbose stuff
     * (e.g. only if you really want to see EVERYTHING.) */
    EXPORT static void Output(
        int32_t nVerbosity,
        const char* szOutput);  // stdout
    EXPORT static void vOutput(int32_t nVerbosity, const char* szOutput, ...)
        ATTR_PRINTF(2, 3);

    /** This logs an error condition, which usually means bad input from the
     * user, or a file wouldn't open, or something like that. This contrasted
     * with Assert() which should NEVER actually happen. The software expects
     * bad user input from time to time. But it never expects a loaded mint to
     * have a nullptr pointer. The bad input would log with Error(), whereas the
     * nullptr pointer would log with Assert(); */
    EXPORT static void Error(const char* szError);       // stderr
    EXPORT static void vError(const char* szError, ...)  // stderr
        ATTR_PRINTF(1, 2);

    /** This method will print out errno and its associated string. Optionally
     * you can pass the location you are calling it from, which will be
     * prepended to the log. */
    EXPORT static void Errno(const char* szLocation = nullptr);  // stderr

    // String Helpers
    EXPORT static bool StringFill(
        String& out_strString,
        const char* szString,
        int32_t iLength,
        const char* szAppend = nullptr);

    /** OPTIONAL. Therefore I will call it in xmlrpcxx_client.cpp just above
     * OT_Init. */
    EXPORT static void SetupSignalHandler();
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTLOG_HPP
