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

#include "opentxs/core/Log.hpp"

#include "opentxs/core/String.hpp"
#include "opentxs/core/Version.hpp"
#include "opentxs/core/app/Settings.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/util/stacktrace.h"

#ifndef _WIN32
#include <cerrno>
#endif

#ifdef _WIN32
#include <Shlobj.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef TARGET_OS_MAC
#include <limits.h>
#include <mach-o/dyld.h>
#endif

#include <cxxabi.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <typeinfo>

#define LOG_DEQUE_SIZE 1024

extern "C" {

#ifdef _WIN32

#include <sys/timeb.h>

// For signal handling in Windows.
LONG Win32FaultHandler(struct _EXCEPTION_POINTERS* ExInfo);
void LogStackFrames(void* FaultAdress, char*);

#else // else if NOT _WIN32

// These added for the signal handling:
//
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// This shitty apple section is for struct sigcontext for the signal handling.
#if defined(__APPLE__)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
// Fucking Apple!
struct sigcontext
{
    // cppcheck-suppress unusedStructMember
    int32_t eip;
};
#endif // defined __APPLE__

#if !defined(ANDROID)
#include <execinfo.h>
#include <signal.h>
#include <ucontext.h>
#endif

//#endif

#endif // not _WIN32

} // extern C

#ifdef ANDROID
#include <android/log.h>
#endif

#define LOGFILE_PRE "log-"
#define LOGFILE_EXT ".log"
#define GLOBAL_LOGNAME "init"
#define GLOBAL_LOGFILE "init.log"

//  OTLog Static Variables and Constants.

namespace opentxs
{

Log* Log::pLogger = nullptr;

const String Log::m_strVersion = OPENTXS_VERSION_STRING;
const String Log::m_strPathSeparator = "/";

OTLOG_IMPORT OTLogStream otErr(-1); // logs using otErr << )
OTLOG_IMPORT OTLogStream otInfo(2); // logs using OTLog::vOutput(2)
OTLOG_IMPORT OTLogStream otOut(0);  // logs using OTLog::vOutput(0)
OTLOG_IMPORT OTLogStream otWarn(1); // logs using OTLog::vOutput(1)
OTLOG_IMPORT OTLogStream otLog3(3); // logs using OTLog::vOutput(3)
OTLOG_IMPORT OTLogStream otLog4(4); // logs using OTLog::vOutput(4)
OTLOG_IMPORT OTLogStream otLog5(5); // logs using OTLog::vOutput(5)

OTLogStream::OTLogStream(int _logLevel)
    : std::ostream(this)
    , logLevel(_logLevel)
    , next(0)
    , pBuffer(new char[1024])
{
}

OTLogStream::~OTLogStream()
{
    delete[] pBuffer;
    pBuffer = nullptr;
}

int OTLogStream::overflow(int c)
{
    pBuffer[next++] = c;
    if (c != '\n' && next < 1000) {
        return 0;
    }

    pBuffer[next++] = '\0';
    next = 0;

    if (logLevel < 0) {
        Log::Error(pBuffer);
        return 0;
    }

    Log::Output(logLevel, pBuffer);
    return 0;
}

//  OTLog Init, must run this before using any OTLog function.

// static
bool Log::Init(const String& strThreadContext, const int32_t& nLogLevel)
{
    if (nullptr == pLogger) {
        pLogger = new Log();
        pLogger->m_bInitialized = false;
    }

    if (strThreadContext.Compare(GLOBAL_LOGNAME)) return false;

    if (!pLogger->m_bInitialized) {
        pLogger->logDeque = std::deque<String*>();
        pLogger->m_strThreadContext = strThreadContext;

        pLogger->m_nLogLevel = nLogLevel;

        if (!strThreadContext.Exists() ||
            strThreadContext.Compare("")) // global
        {
            pLogger->m_strLogFileName = GLOBAL_LOGFILE;
        }
        else // not global
        {

            pLogger->m_strLogFileName.Format(
                "%s%s%s", LOGFILE_PRE, strThreadContext.Get(), LOGFILE_EXT);

            Settings config(OTPaths::GlobalConfigFile());

            config.Reset();
            if (!config.Load()) {
                return false;
            }

            bool bIsNew(false);
            if (!config.CheckSet_str("logfile", strThreadContext,
                                     pLogger->m_strLogFileName,
                                     pLogger->m_strLogFileName, bIsNew)) {
                return false;
            }

            if (!config.Save()) {
                return false;
            }
            config.Reset();
        }

#ifdef ANDROID
        if (OTPaths::HomeFolder().Exists())
#endif
            if (!OTPaths::AppendFile(pLogger->m_strLogFilePath,
                                     OTPaths::AppDataFolder(),
                                     pLogger->m_strLogFileName)) {
                return false;
            }

        pLogger->m_bInitialized = true;

        // Set the new log-assert function pointer.
        Assert* pLogAssert = new Assert(Log::logAssert);
        std::swap(pLogAssert, Assert::s_pAssert);
        delete pLogAssert;
        pLogAssert = nullptr;

        return true;
    }
    else {
        return false;
    }
}

// static
bool Log::IsInitialized()
{
    return nullptr != pLogger && pLogger->m_bInitialized;
}

// static
bool Log::Cleanup()
{
    if (nullptr != pLogger) {
        delete pLogger;
        pLogger = nullptr;
        return true;
    }
    return false;
}

// static
bool Log::CheckLogger(Log* pLogger)
{
    if (nullptr != pLogger && pLogger->m_bInitialized) return true;

    OT_FAIL;
}

// OTLog Constants.

// Compiled into OTLog:

const char* Log::Version()
{
    return Log::GetVersion().Get();
}
const String& Log::GetVersion()
{
    return m_strVersion;
}

const char* Log::PathSeparator()
{
    return Log::GetPathSeparator().Get();
}
const String& Log::GetPathSeparator()
{
    return m_strPathSeparator;
}

// Set in constructor:

const String& Log::GetThreadContext()
{
    return pLogger->m_strThreadContext;
}

const char* Log::LogFilePath()
{
    return Log::GetLogFilePath().Get();
}
const String& Log::GetLogFilePath()
{
    return pLogger->m_strLogFilePath;
}

// static
int32_t Log::LogLevel()
{
    if (nullptr != pLogger)
        return pLogger->m_nLogLevel;
    else
        return 0;
}

// static
bool Log::SetLogLevel(const int32_t& nLogLevel)
{
    if (nullptr == pLogger) {
        OT_FAIL;
    }
    else {
        pLogger->m_nLogLevel = nLogLevel;
        return true;
    }
}

//  OTLog Functions

// If there's no logfile, then send it to stderr.
// (So we can still see it on the screen, but it doesn't interfere with any
// command line utilities who might otherwise interpret it as their own input,
// if I was actually writing to stdout.)
//
// static
bool Log::LogToFile(const String& strOutput)
{
    // We now do this either way.
    {
        std::cerr << strOutput;
        std::cerr.flush();
    }

    // now log to file, if we can.

    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    bool bSuccess = false;

    if (bHaveLogger) {
        // Append to logfile
        if ((strOutput.Exists()) && (Log::pLogger->m_strLogFilePath.Exists())) {
            std::ofstream logfile;
            logfile.open(Log::LogFilePath(), std::ios::app);

            if (!logfile.fail()) {
                logfile << strOutput;
                logfile.close();
                bSuccess = true;
            }
        }
    }

    return bSuccess;
}

String Log::GetMemlogAtIndex(int32_t nIndex)
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    uint32_t uIndex = static_cast<uint32_t>(nIndex);

    if ((nIndex < 0) || (uIndex >= Log::pLogger->logDeque.size())) {
        otErr << __FUNCTION__ << ": index out of bounds: " << nIndex << "\n";
        return "";
    }

    if (nullptr != Log::pLogger->logDeque.at(uIndex))
        ; // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.at(uIndex);

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

// We keep 1024 logs in memory, to make them available via the API.

int32_t Log::GetMemlogSize()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    return static_cast<int32_t>(Log::pLogger->logDeque.size());
}

String Log::PeekMemlogFront()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (Log::pLogger->logDeque.size() <= 0) return nullptr;

    if (nullptr != Log::pLogger->logDeque.front())
        ; // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.front();

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

String Log::PeekMemlogBack()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (Log::pLogger->logDeque.size() <= 0) return nullptr;

    if (nullptr != Log::pLogger->logDeque.back())
        ; // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.back();

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

// static
bool Log::PopMemlogFront()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (Log::pLogger->logDeque.size() <= 0) return false;

    String* strLogFront = Log::pLogger->logDeque.front();
    if (nullptr != strLogFront) delete strLogFront;
    strLogFront = nullptr;

    Log::pLogger->logDeque.pop_front();

    return true;
}

// static
bool Log::PopMemlogBack()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (Log::pLogger->logDeque.size() <= 0) return false;

    String* strLogBack = Log::pLogger->logDeque.back();
    if (nullptr != strLogBack) delete strLogBack;
    strLogBack = nullptr;

    Log::pLogger->logDeque.pop_back();

    return true;
}

// static
bool Log::PushMemlogFront(const String& strLog)
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    OT_ASSERT(strLog.Exists());

    Log::pLogger->logDeque.push_front(new String(strLog));

    if (Log::pLogger->logDeque.size() > LOG_DEQUE_SIZE) {
        Log::PopMemlogBack(); // We start removing from the back when it
                              // reaches this size.
    }

    return true;
}

// static
bool Log::Sleep(const std::chrono::microseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
        std::this_thread::sleep_for(us);
    } while (std::chrono::high_resolution_clock::now() < end);
    return true;
}

// This function is for things that should NEVER happen.
// In fact you should never even call it -- use the OT_ASSERT() macro instead.
// This Function is now only for logging, you
// static private
size_t Log::logAssert(const char* szFilename, size_t nLinenumber,
                      const char* szMessage)
{
    if (nullptr != szMessage) {
#ifndef ANDROID // if NOT android
        std::cerr << szMessage << "\n";

        LogToFile(szMessage);
        LogToFile("\n");

#else // if Android
        __android_log_write(ANDROID_LOG_FATAL, "OT Assert (or Fail)",
                            szMessage);
#endif

        print_stacktrace();
    }

    if ((nullptr != szFilename)) {
#ifndef ANDROID // if NOT android

        // Pass it to LogToFile, as this always logs.
        //
        String strTemp;
        strTemp.Format("\nOT_ASSERT in %s at line %" PRI_SIZE "\n", szFilename,
                       nLinenumber);
        LogToFile(strTemp.Get());

#else // if Android
        String strAndroidAssertMsg;
        strAndroidAssertMsg.Format("\nOT_ASSERT in %s at line %d\n", szFilename,
                                   nLinenumber);
        __android_log_write(ANDROID_LOG_FATAL, "OT Assert",
                            strAndroidAssertMsg.Get());
#endif
    }

    print_stacktrace();

    return 1; // normal
}

// For normal output. The higher the verbosity, the less important the message.
// (Verbose level 0 ALWAYS logs.) Currently goes to stdout.

void Log::Output(int32_t nVerbosity, const char* szOutput)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    // If log level is 0, and verbosity of this message is 2, don't bother
    // logging it.
    //    if (nVerbosity > OTLog::__CurrentLogLevel || (nullptr == szOutput))
    if ((nVerbosity > LogLevel()) || (nullptr == szOutput) ||
        (LogLevel() == (-1)))
        return;

    // We store the last 1024 logs so programmers can access them via the API.
    if (bHaveLogger) Log::PushMemlogFront(szOutput);

#ifndef ANDROID // if NOT android

    LogToFile(szOutput);

#else // if IS Android
    /*
    typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    // only for SetMinPriority()
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,     // only for SetMinPriority(); must be last
    } android_LogPriority;
    */
    switch (nVerbosity) {
    case 0:
    case 1:
        __android_log_write(ANDROID_LOG_INFO, "OT Output", szOutput);
        break;
    case 2:
    case 3:
        __android_log_write(ANDROID_LOG_DEBUG, "OT Debug", szOutput);
        break;
    case 4:
    case 5:
        __android_log_write(ANDROID_LOG_VERBOSE, "OT Verbose", szOutput);
        break;
    default:
        __android_log_write(ANDROID_LOG_UNKNOWN, "OT Unknown", szOutput);
        break;
    }
#endif
}

// the vOutput is to avoid name conflicts.
void Log::vOutput(int32_t nVerbosity, const char* szOutput, ...)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    // If log level is 0, and verbosity of this message is 2, don't bother
    // logging it.
    if (((0 != LogLevel()) && (nVerbosity > LogLevel())) ||
        (nullptr == szOutput))
        return;

    va_list args;
    va_start(args, szOutput);

    std::string strOutput;

    const bool bFormatted = String::vformat(szOutput, &args, strOutput);

    va_end(args);

    if (bFormatted)
        Log::Output(nVerbosity, strOutput.c_str());
    else
        OT_FAIL;
    return;
}

// the vError name is to avoid name conflicts
void Log::vError(const char* szError, ...)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if ((nullptr == szError)) return;

    va_list args;
    va_start(args, szError);

    std::string strOutput;

    const bool bFormatted = String::vformat(szError, &args, strOutput);

    va_end(args);

    if (bFormatted)
        Log::Error(strOutput.c_str());
    else
        OT_FAIL;
}

// An error has occurred, that somehow doesn't match the Assert or Output
// functions.
// So use this one instead.  This ALWAYS logs and currently it all goes to
// stderr.

void Log::Error(const char* szError)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if ((nullptr == szError)) return;

    // We store the last 1024 logs so programmers can access them via the API.
    if (bHaveLogger) Log::PushMemlogFront(szError);

#ifndef ANDROID // if NOT android

    LogToFile(szError);

#else // if Android
    __android_log_write(ANDROID_LOG_ERROR, "OT Error", szError);
#endif
}

// NOTE: if you have problems compiling on certain platforms, due to the use
// of errno, then just use preprocessor directives to carve those portions out
// of this function, replacing with a message about the unavailability of errno.
//
// static
void Log::Errno(const char* szLocation) // stderr
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    const int32_t errnum = errno;
    char buf[128];
    buf[0] = '\0';

    int32_t nstrerr = 0;
    char* szErrString = nullptr;

//#if((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) &&
//!defined(_GNU_SOURCE))

#if defined(_GNU_SOURCE) && defined(__linux__) && !defined(ANDROID)
    szErrString = strerror_r(errnum, buf, 127);
#elif defined(_POSIX_C_SOURCE)
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    // (strerror_r is threadsafe version of strerror)
    nstrerr = strerror_r(errnum, buf, 127);
#endif
#endif

    const char* szFunc = "OTLog::Errno";
    const char* sz_location = (nullptr == szLocation) ? "" : szLocation;

    if (nullptr == szErrString) szErrString = buf;

    if (0 == nstrerr)
        otErr << szFunc << " " << sz_location << ": errno " << errnum << ": "
              << (szErrString[0] != '\0' ? szErrString : "") << ".\n";
    else
        otErr << szFunc << " " << sz_location << ": errno: " << errnum
              << ". (Unable to retrieve error string for that number.)\n";
}

// String Helpers

bool Log::StringFill(String& out_strString, const char* szString,
                     int32_t iLength, const char* szAppend)
{
    std::string strString(szString);

    if (nullptr != szAppend) strString.append(szAppend);

    for (; (static_cast<int32_t>(strString.length()) < iLength);
         strString.append(" "))
        ;

    out_strString.Set(strString.c_str());

    return true;
}

// SIGNALS
//
// To get the most mileage out of this signal handler,
// compile it with the options:  -g -rdynamic
//
//  Signal Handler
//
//

void ot_terminate(void);

namespace
{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800) // warning C4800: forcing constant value.
#endif

// invoke set_terminate as part of global constant initialization
static const bool SET_TERMINATE __attribute__ ((unused)) = std::set_terminate(ot_terminate);

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

// This is our custom std::terminate(). Also called for uncaught exceptions.
void ot_terminate()
{
// exception header is needed, on latter android NDK the header is not available
// see https://code.google.com/p/android/issues/detail?id=62648
#if !defined(ANDROID) || defined(_EXCEPTION_PTR_H)
    if (auto e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        }
        catch (const std::exception& e2) {
            std::cerr << "ot_terminate: " << __FUNCTION__
                      << " caught unhandled exception."
                      << " type: " << typeid(e2).name()
                      << " what(): " << e2.what() << std::endl;
        }
        catch (...) {
            std::cerr << "ot_terminate: " << __FUNCTION__
                      << " caught unknown/unhandled exception." << std::endl;
        }
    }
#endif
    print_stacktrace();

    // Call the default std::terminate() handler.
    std::abort();
}

#ifdef _WIN32 // Windows SIGNALS

// The windows version is from Stefan Wörthmüller, who wrote an excellent
// article
// at Dr. Dobbs Journal here:
// http://www.drdobbs.com/architecture-and-design/185300443
//

// static
void OTLog::SetupSignalHandler()
{
    static int32_t nCount = 0;

    if (0 == nCount) {
        ++nCount;
        SetUnhandledExceptionFilter(
            (LPTOP_LEVEL_EXCEPTION_FILTER)Win32FaultHandler);
    }
}

#else // if _WIN32, else:      UNIX -- SIGNALS

// CREDIT: the Linux / GNU portion of the signal handler comes from
// StackOverflow,
// where several answers are combined here.
// http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
//

struct sig_ucontext_t
{
    struct ucontext* uc_link;
    stack_t uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t uc_sigmask;
};

extern "C" {
// This structure mirrors the one found in /usr/include/asm/ucontext.h
//

void crit_err_hdlr(int32_t sig_num, siginfo_t* info, void* ucontext);
}

#if defined(OT_NO_DEMANGLING_STACK_TRACE)

// this version doesn't do demangling.
void crit_err_hdlr(int32_t sig_num, siginfo_t* info, void* ucontext)
{
    void* array[50];
    void* caller_address;
    char** messages;
    int32_t size, i;
    sig_ucontext_t* uc;

    static std::mutex the_Mutex;

    std::lock_guard<std::mutex> lock(the_Mutex);

    uc = static_cast<sig_ucontext_t*>(ucontext);

    // Get the address at the time the signal was raised from the EIP (x86)
    caller_address = (void*)uc->uc_mcontext.eip;

    fprintf(stderr, "signal %d (%s), address is %p from %p\n", sig_num,
            strsignal(sig_num), info->si_addr, (void*)caller_address);

    size = backtrace(array, 50);

    // overwrite sigaction with caller's address
    //
    array[1] = caller_address;

    messages = backtrace_symbols(array, size);

    // skip first stack frame (points here)
    //
    for (i = 1; i < size && messages != nullptr; ++i) {
        fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
    }

    free(messages);

    _exit(0);
}

#else // #if no demangling, #else...

// This version DOES do demangling.
//
/*
void crit_err_hdlr(int32_t sig_num, siginfo_t* info, void* ucontext)
{
    sig_ucontext_t * uc = (sig_ucontext_t *)ucontext;

    void * caller_address = (void *) uc->uc_mcontext.eip; // x86 specific

    std::cerr << "signal " << sig_num
    << " (" << strsignal(sig_num) << "), address is "
    << info->si_addr << " from " << caller_address
    << std::endl << std::endl;

    void * array[50];
    int32_t size = backtrace(array, 50);

    array[1] = caller_address;

    char ** messages = backtrace_symbols(array, size);

    // skip first stack frame (points here)
    for (int32_t i = 1; i < size && messages != nullptr; ++i)
    {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // find parantheses and +address offset surrounding mangled name
        for (char *p = messages[i]; *p; ++p)
        {
            if (*p == '(')
            {
                mangled_name = p;
            }
            else if (*p == '+')
            {
                offset_begin = p;
            }
            else if (*p == ')')
            {
                offset_end = p;
                break;
            }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin)
        {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int32_t status;
            char * real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

            // if demangling is successful, output the demangled function name
            if (status == 0)
            {
                std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                << real_name << "+" << offset_begin << offset_end
                << std::endl;

            }
            // otherwise, output the mangled function name
            else
            {
                std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                << mangled_name << "+" << offset_begin << offset_end
                << std::endl;
            }
            free(real_name);
        }
        // otherwise, print the whole line
        else
        {
            std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }
    }
    std::cerr << std::endl;

    free(messages);

    _exit(0);
}
*/



void crit_err_hdlr(ANDROID_UNUSED int32_t sig_num,
                   ANDROID_UNUSED siginfo_t* info, ANDROID_UNUSED void* v)
{
#ifndef ANDROID
    static std::mutex the_Mutex;

    std::lock_guard<std::mutex> lock(the_Mutex);

    OT_ASSERT(nullptr != v);

#ifdef _LP64
    typedef uint64_t ot_ulong;
#else
    typedef uint32_t ot_ulong;
#endif // lp64

    ot_ulong eip = 0;
    ucontext_t* uc = static_cast<ucontext_t*>(v);

#if defined(__APPLE__)
#ifdef __arm__
    _STRUCT_MCONTEXT* mc; // mcontext_t seems to be missing from arm/_structs.h
    // cppcheck-suppress unreadVariable
    mc = uc->uc_mcontext;
    eip = mc->__ss.__pc;
//  eip = mc->__ss.__eip; // arm doesn't have eip
#else
    mcontext_t mc;
    mc = uc->uc_mcontext;
#ifdef _LP64
    #if TARGET_IPHONE_SIMULATOR
      eip = mc->__ss.__rip;
    #elif TARGET_OS_IPHONE
      eip = mc->__ss.__pc;
    #else
      eip = mc->__ss.__rip;
    #endif
#else
    eip = mc->__ss.__eip;
#endif
#endif // __arm__
#elif defined(__linux__)
    mcontext_t* mc;
    struct sigcontext* ctx;
    mc = &uc->uc_mcontext;
    ctx = reinterpret_cast<struct sigcontext*>(mc);
#ifdef __i386__
    eip = ctx->eip;
#else
    eip = ctx->rip;
#endif
#elif defined(__FreeBSD__)
    mcontext_t* mc;
    mc = &uc->uc_mcontext;
#ifdef __i386__
    eip = mc->mc_eip;
#elif defined(__amd64__)
    eip = mc->mc_rip;
#endif
    ot_ulong addr = (ot_ulong)info->si_addr;
    if (__FreeBSD__ < 7) {
        /*
        * FreeBSD /usr/src/sys/i386/i386/trap.c kludgily reuses
        * frame->tf_err as somewhere to put the faulting address
        * (cr2) when calling into the generic signal dispatcher.
        * Unfortunately, that means that the bit in tf_err that says
        * whether this is a read or write fault is irretrievably gone.
        * So we have to figure it out.  Let's assume that if the page
        * is already mapped in core, it is a write fault.  If not, it is a
        * read fault.
        *
        * This is apparently fixed in FreeBSD 7, but I don't have any
        * FreeBSD 7 machines on which to verify this.
        */
        char vec;
        int32_t r;

        vec = 0;
        r = mincore((void*)addr, 1, &vec);
        // iprint("FreeBSD fault [%d]: addr=%p[%p] mincore=%d vec=%#x
        // errno=%d\n", signo, addr, (uchar*)addr-uzero, r, vec, errno);
        if (r < 0 || vec == 0)
            mc->mc_err = 0; /* read fault */
        else
            mc->mc_err = 2; /* write fault */
    }
#else
#error "Unknown OS in sigsegv"
#endif

    void* caller_address = reinterpret_cast<void*>(eip);

    std::cerr << "signal " << sig_num << " (" << strsignal(sig_num)
              << "), address is " << info->si_addr << " from " << caller_address
              << std::endl << std::endl;

    void* array[50];
    int32_t size = backtrace(array, 50);

    array[1] = caller_address;

    char** messages = backtrace_symbols(array, size);

    // skip first stack frame (points here)
    for (int32_t i = 1; i < size && messages != nullptr; ++i) {
        char* mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // find parantheses and +address offset surrounding mangled name
        for (char* p = messages[i]; *p; ++p) {
            if (*p == '(') {
                mangled_name = p;
            }
            else if (*p == '+') {
                offset_begin = p;
            }
            else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin) {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int32_t status;
            char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

            // if demangling is successful, output the demangled function name
            if (status == 0) {
                std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                          << real_name << "+" << offset_begin << offset_end
                          << std::endl;

            }
            // otherwise, output the mangled function name
            else {
                std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                          << mangled_name << "+" << offset_begin << offset_end
                          << std::endl;
            }
            free(real_name);
        }
        // otherwise, print the whole line
        else {
            std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }
    }
    std::cerr << std::endl;

    free(messages);
#endif // #ifndef ANDROID
    _exit(0);
}

#endif // defined(OT_NO_DEMANGLING_STACK_TRACE)

#ifndef OT_HANDLE_SIGNAL
#define OT_HANDLE_SIGNAL(OT_SIGNAL_TYPE)                                       \
    {                                                                          \
        struct sigaction new_action, old_action;                               \
        new_action.sa_sigaction = crit_err_hdlr;                               \
        sigemptyset(&new_action.sa_mask);                                      \
        new_action.sa_flags = SA_RESTART | SA_SIGINFO;                         \
                                                                               \
        sigaction(OT_SIGNAL_TYPE, nullptr, &old_action);                       \
                                                                               \
        if (old_action.sa_handler != SIG_IGN) {                                \
            if (sigaction(OT_SIGNAL_TYPE, &new_action, nullptr) != 0) {        \
                otErr << "OTLog::SetupSignalHandler: Failed setting signal "   \
                         "handler for error " << OT_SIGNAL_TYPE << " ("        \
                      << strsignal(OT_SIGNAL_TYPE) << ")\n";                   \
                abort();                                                       \
            }                                                                  \
        }                                                                      \
    }
#endif

// static
void Log::SetupSignalHandler()
{
    static int32_t nCount = 0;

    if (0 == nCount) {
        ++nCount;

        OT_HANDLE_SIGNAL(SIGINT)  // Ctrl-C. (So we can shutdown gracefully, I
                                  // suppose, on Ctrl-C.)
        OT_HANDLE_SIGNAL(SIGSEGV) // Segmentation fault.
                                  //      OT_HANDLE_SIGNAL(SIGABRT) // Abort.
        OT_HANDLE_SIGNAL(SIGBUS)  // Bus error
        //      OT_HANDLE_SIGNAL(SIGHUP)  // I believe this is for sending a
        // "restart" signal to your process, that sort of thing.
        OT_HANDLE_SIGNAL(SIGTERM) // Used by kill pid (NOT kill -9 pid). Used
                                  // for "killing softly."
        OT_HANDLE_SIGNAL(SIGILL)  // Illegal instruction.
        OT_HANDLE_SIGNAL(SIGTTIN) // SIGTTIN may be sent to a background process
                                  // that attempts to read from its controlling
                                  // terminal.
        OT_HANDLE_SIGNAL(SIGTTOU) // SIGTTOU may be sent to a background process
                                  // that attempts to write to its controlling
                                  // terminal.
        //      OT_HANDLE_SIGNAL(SIGPIPE) // Unix supports the principle of
        // piping. When a pipe is broken, the process writing to it is sent the
        // SIGPIPE.
        //      OT_HANDLE_SIGNAL(SIGKILL) // kill -9. "The receiving process
        // cannot perform any clean-up upon receiving this signal."
        OT_HANDLE_SIGNAL(SIGFPE)  // Floating point exception.
        OT_HANDLE_SIGNAL(SIGXFSZ) // SIGXFSZ is the signal sent to a process
                                  // when it grows a file larger than the
                                  // maximum allowed size.
        //      OT_HANDLE_SIGNAL(SIGQUIT) // SIGQUIT is the signal sent to a
        // process by its controlling terminal when the user requests that the
        // process perform a core dump.
        OT_HANDLE_SIGNAL(SIGSYS) // sent when a process supplies an incorrect
                                 // argument to a system call.
        //      OT_HANDLE_SIGNAL(SIGTRAP) // used by debuggers
    }
}

#endif // #if windows, #else (unix) #endif. (SIGNAL handling.)

} // namespace opentxs

#ifdef _WIN32 // Windows SIGNALS

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS* ExInfo)
{
    char* FaultTx = "";

    switch (ExInfo->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        FaultTx = "ACCESS VIOLATION";
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        FaultTx = "DATATYPE MISALIGNMENT";
        break;
    case EXCEPTION_BREAKPOINT:
        FaultTx = "BREAKPOINT";
        break;
    case EXCEPTION_SINGLE_STEP:
        FaultTx = "SINGLE STEP";
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        FaultTx = "ARRAY BOUNDS EXCEEDED";
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        FaultTx = "FLT DENORMAL OPERAND";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        FaultTx = "FLT DIVIDE BY ZERO";
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        FaultTx = "FLT INEXACT RESULT";
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        FaultTx = "FLT INVALID OPERATION";
        break;
    case EXCEPTION_FLT_OVERFLOW:
        FaultTx = "FLT OVERFLOW";
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        FaultTx = "FLT STACK CHECK";
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        FaultTx = "FLT UNDERFLOW";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        FaultTx = "INT DIVIDE BY ZERO";
        break;
    case EXCEPTION_INT_OVERFLOW:
        FaultTx = "INT OVERFLOW";
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        FaultTx = "PRIV INSTRUCTION";
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        FaultTx = "IN PAGE ERROR";
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        FaultTx = "ILLEGAL INSTRUCTION";
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        FaultTx = "NONCONTINUABLE EXCEPTION";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        FaultTx = "STACK OVERFLOW";
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        FaultTx = "INVALID DISPOSITION";
        break;
    case EXCEPTION_GUARD_PAGE:
        FaultTx = "GUARD PAGE";
        break;
    default:
        FaultTx = "(unknown)";
        break;
    }
    int32_t wsFault = ExInfo->ExceptionRecord->ExceptionCode;
    void* CodeAdress = ExInfo->ExceptionRecord->ExceptionAddress;

    // (using stderr.)
    //  sgLogFile = fopen("Win32Fault.log", "w");

    if (stderr != nullptr) {
        fprintf(stderr,
                "****************************************************\n");
        fprintf(stderr, "*** A Programm Fault occured:\n");
        fprintf(stderr, "*** Error code %08X: %s\n", wsFault, FaultTx);
        fprintf(stderr,
                "****************************************************\n");
        fprintf(stderr, "***   Address: %08X\n", (int32_t)CodeAdress);
        fprintf(stderr, "***     Flags: %08X\n",
                ExInfo->ExceptionRecord->ExceptionFlags);

#if defined(_CONSOLE)
        printf("\n");
        printf("*** A Programm Fault occured:\n");
        printf("*** Error code %08X: %s\n", wsFault, FaultTx);
#endif
/* This infomation ssems to be wrong
if(ExInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
{
fprintf(stderr, "****************************************************\n");
fprintf(stderr, "*** Attempted to %s to address %08LX \n",
ExInfo->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
ExInfo->ExceptionRecord->ExceptionInformation[1]);

}
*/
#ifdef _WIN64
//        LogStackFrames(CodeAdress, (char *)ExInfo->ContextRecord->Rbp);
#else
        LogStackFrames(CodeAdress, (char*)ExInfo->ContextRecord->Ebp);
#endif

        //      fclose(sgLogFile);
    }

    /*if(want to continue)
    {
    ExInfo->ContextRecord->Eip++;
    #if defined (_CONSOLE)
    printf("*** Trying to continue\n");
    printf("\n");
    #endif
    return EXCEPTION_CONTINUE_EXECUTION;
    }
    */

    printf("*** Terminating\n");
    printf("\n");
    return EXCEPTION_EXECUTE_HANDLER;
}
/////////////////////////////////////////////////////////////////////////////
// Unwind the stack and save its return addresses to the logfile
/////////////////////////////////////////////////////////////////////////////

void LogStackFrames(void* FaultAdress, char* eNextBP)

{
#if defined(_WIN64)

    typedef USHORT(WINAPI * CaptureStackBackTraceType)(
        __in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);

    HMODULE lLoadedLib = LoadLibrary(L"kernel32.dll");
    if (nullptr == lLoadedLib) OT_FAIL;
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(
        GetProcAddress(lLoadedLib, "RtlCaptureStackBackTrace"));

    if (func == nullptr) return;

    // Quote from Microsoft Documentation:
    // ## Windows Server 2003 and Windows XP:
    // ## The sum of the FramesToSkip and FramesToCapture parameters must be
    // less than 63.
    const int32_t kMaxCallers = 62;

    void* callers[kMaxCallers];
    int32_t count = (func)(0, kMaxCallers, callers, nullptr);
    for (int32_t i = 0; i < count; i++)
        fprintf(stderr, "*** %d called from %p\n", i, callers[i]);

#elif defined(_WIN32) // not _WIN64 ? Must be _WIN32

    char* pBP = nullptr;
    uint32_t i = 0, x = 0, BpPassed = 0;
    static int32_t CurrentlyInTheStackDump = 0;

    if (CurrentlyInTheStackDump) {
        fprintf(stderr, "\n***\n*** Recursive Stack Dump skipped\n***\n");
        return;
    }

    fprintf(stderr, "****************************************************\n");
    fprintf(stderr, "*** CallStack:\n");
    fprintf(stderr, "****************************************************\n");

    /* ====================================================================== */
    /*                                                                        */
    /*      BP +x ...    -> == SP (current top of stack)                      */
    /*            ...    -> Local data of current function                    */
    /*      BP +4 0xabcd -> 32 address of calling function                    */
    /*  +<==BP    0xabcd -> Stack address of next stack frame (0, if end)     */
    /*  |   BP -1 ...    -> Aruments of function call                         */
    /*  Y                                                                     */
    /*  |   BP -x ...    -> Local data of calling function                    */
    /*  |                                                                     */
    /*  Y  (BP)+4 0xabcd -> 32 address of calling function                    */
    /*  +==>BP)   0xabcd -> Stack address of next stack frame (0, if end)     */
    /*            ...                                                         */
    /* ====================================================================== */
    CurrentlyInTheStackDump = 1;

    BpPassed = (eNextBP != nullptr);

    if (!eNextBP) {
        _asm mov eNextBP, eBp
    }
    else
        fprintf(stderr, "\n  Fault Occured At $ADDRESS:%08LX\n",
                (int32_t)FaultAdress);

    // prevent infinite loops
    for (i = 0; eNextBP && i < 100; i++) {
        pBP = eNextBP;          // keep current BasePointer
        eNextBP = *(char**)pBP; // dereference next BP

        char* p = pBP + 8;

        // Write 20 Bytes of potential arguments
        fprintf(stderr, "         with ");
        for (x = 0; p < eNextBP && x < 20; p++, x++)
            fprintf(stderr, "%02X ", *(uint8_t*)p);

        fprintf(stderr, "\n\n");

        if (i == 1 && !BpPassed)
            fprintf(stderr,
                    "****************************************************\n"
                    "         Fault Occured Here:\n");

        // Write the backjump address
        fprintf(stderr, "*** %2d called from $ADDRESS:%08X\n", i,
                *(char**)(pBP + 4));

        if (*(char**)(pBP + 4) == nullptr) break;
    }

    fprintf(stderr,
            "************************************************************\n");
    fprintf(stderr, "\n\n");

    CurrentlyInTheStackDump = 0;

    fflush(stderr);
#endif                // _WIN64 else (_WIN32) endif
}

#endif
