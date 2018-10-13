// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/network/ZAP.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/util/Signals.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include "api/client/InternalClient.hpp"
#include "api/storage/StorageInternal.hpp"
#include "internal/api/Internal.hpp"
#include "internal/rpc/Internal.hpp"
#include "network/OpenDHT.hpp"
#include "storage/StorageConfig.hpp"
#include "Scheduler.hpp"

#include <algorithm>
#include <atomic>
#include <ctime>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Native.hpp"

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN 128
#endif

#define OT_METHOD "opentxs::api::implementation::Native::"

namespace
{
// If the password callback isn't set, then it uses the default ("test")
// password.
extern "C" std::int32_t default_pass_cb(
    char* buf,
    std::int32_t size,
    [[maybe_unused]] std::int32_t rwflag,
    void* userdata)
{
    std::int32_t len{0};
    const auto theSize{std::uint32_t(size)};
    const opentxs::OTPasswordData* pPWData{nullptr};
    std::string str_userdata;

    if (nullptr != userdata) {
        pPWData = static_cast<const opentxs::OTPasswordData*>(userdata);

        if (nullptr != pPWData) { str_userdata = pPWData->GetDisplayString(); }
    } else {
        str_userdata = "";
    }

    opentxs::LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Using DEFAULT TEST PASSWORD: 'test' (for  ")(str_userdata)(" ) ")
        .Flush();

    const char* tmp_passwd = "test";
    len = static_cast<std::int32_t>(strlen(tmp_passwd));

    if (len <= 0) {
        opentxs::otOut << __FUNCTION__ << ": Problem? Returning 0...\n";

        return 0;
    }

    if (len > size) { len = size; }

    const auto theLength = len;

    opentxs::OTPassword::safe_memcpy(
        buf,         // destination
        theSize,     // size of destination buffer.
        tmp_passwd,  // source
        theLength);  // length of source.

    return len;
}

// This is the function that OpenSSL calls when it wants to ask the user for his
// password. If we return 0, that's bad, that means the password caller and
// callback failed somehow.
extern "C" std::int32_t souped_up_pass_cb(
    char* buf,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata)
{
    OT_ASSERT(nullptr != userdata);

    const auto& native =
        dynamic_cast<const opentxs::api::internal::Native&>(opentxs::OT::App());
    const auto* pPWData = static_cast<const opentxs::OTPasswordData*>(userdata);
    const std::string str_userdata = pPWData->GetDisplayString();
    opentxs::OTPassword thePassword;
    bool bGotPassword = false;

    // Sometimes it's passed in, otherwise we use the global one.
    const auto providedKey = pPWData->GetCachedKey();
    auto& cachedKey =
        (nullptr == providedKey) ? native.Crypto().DefaultKey() : *providedKey;
    const bool b1 = pPWData->isForNormalNym();
    const bool b3 = !(cachedKey.isPaused());

    // For example, perhaps we need to collect a password for a symmetric key.
    // In that case, it has nothing to do with any master key, or any
    // public/private keys. It ONLY wants to do a simple password collect.
    const bool bOldSystem = pPWData->isUsingOldSystem();

    // It's for one of the normal Nyms. (NOT the master key.)
    // If it was for the master key, we'd just pop up the dialog and get the
    // master passphrase. But since it's for a NORMAL Nym, we have to call
    // OTCachedKey::GetMasterPassword. IT will pop up the dialog if it needs to,
    // by recursively calling this in master mode, and then it'll use the user
    // passphrase from that dialog to derive a key, and use THAT key to unlock
    // the actual "passphrase" (a random value) which is then passed back to
    // OpenSSL to use for the Nyms.
    if (b1 &&  // Normal Nyms, unlike Master passwords, have to look up the
               // master password first.
        !bOldSystem && b3)  // ...Unless they are still using the old system, in
                            // which case they do NOT look up the master
                            // password...
    {
        // Therefore we need to provide the password from an
        // crypto::key::LegacySymmetric stored here. (the "actual key" in the
        // crypto::key::LegacySymmetric IS the password that we are passing
        // back!)

        // So either the "actual key" is cached on a timer, from some previous
        // request like this, OR we have to pop up the passphrase dialog, ask
        // for the passphrase for the crypto::key::LegacySymmetric, and then use
        // it to GET the actual key from that crypto::key::LegacySymmetric. The
        // crypto::key::Symmetric should be stored in the OTWallet or Server,
        // which sets a pointer to itself inside the OTPasswordData class
        // statically, on initialization. That way, OTPasswordData can use that
        // pointer to get a pointer to the relevant crypto::key::LegacySymmetric
        // being used as the MASTER key.
        opentxs::LogDebug(OT_METHOD)(__FUNCTION__)(
            ": Using GetMasterPassword() call. ")
            .Flush();
        bGotPassword = cachedKey.GetMasterPassword(
            cachedKey,
            thePassword,
            str_userdata.c_str());  // bool bVerifyTwice=false

        // NOTE: shouldn't the above call to GetMasterPassword be passing the
        // rwflag as the final parameter? Just as we see below with the call to
        // GetPasswordFromConsole. Right? Of course, it DOES generate
        // internally, if necessary, and thus it forces an "ask twice" in that
        // situation anyway. (It's that smart.) Actually that's it. The master
        // already asks twice when it's generating.
    } else {
        opentxs::LogDebug(OT_METHOD)(__FUNCTION__)(
            ": Using OT Password Callback. ")
            .Flush();
        auto& caller = native.GetPasswordCaller();
        // The dialog should display this string (so the user knows what he is
        // authorizing.)
        caller.SetDisplay(
            str_userdata.c_str(),
            static_cast<std::int32_t>(str_userdata.size()));

        if (1 == rwflag) {
            opentxs::LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Using OT Password Callback (asks twice) for \"")(
                str_userdata)("\"...")
                .Flush();
            caller.callTwo();
        } else {
            opentxs::LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Using OT Password Callback (asks once) for \"")(
                str_userdata)("\"...")
                .Flush();
            caller.callOne();
        }
        /*
            NOTICE: (For security...)

            We are using an OTPassword object to collect the password from the
            caller. (We're not passing strings back and forth.) The OTPassword
            object is where wecan centralize our efforts to scrub the memory
            clean as soon as we're done with the password. It's also designed to
            be light (no baggage) and to be passed around easily, with a
            set-size array for the data.

            Notice I am copying the password directly from the OTPassword
            object into the buffer provided to me by OpenSSL. When the
            OTPassword object goes out of scope, then it cleans up
            automatically.
        */
        bGotPassword = caller.GetPassword(thePassword);
    }

    if (false == bGotPassword) {
        opentxs::otOut
            << __FUNCTION__
            << ": Failure: (false == bGotPassword.) (Returning 0.)\n";

        return 0;
    }

    opentxs::LogVerbose(OT_METHOD)(__FUNCTION__)(": Success! ").Flush();
    std::int32_t len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                                : thePassword.getMemorySize();

    if (len < 0) {
        opentxs::otOut << __FUNCTION__
                       << ": <0 length password was "
                          "returned from the API password callback. "
                          "Returning 0.\n";

        return 0;
    } else if (len == 0) {
        const char* szDefault = "test";
        opentxs::otOut << __FUNCTION__
                       << ": 0 length password was "
                          "returned from the API password callback. "
                          "Substituting default password 'test'.\n";

        if (thePassword.isPassword()) {
            thePassword.setPassword(
                szDefault,
                static_cast<std::int32_t>(
                    opentxs::String::safe_strlen(szDefault, _PASSWORD_LEN)));
        } else {
            thePassword.setMemory(
                static_cast<const void*>(szDefault),
                static_cast<std::uint32_t>(
                    opentxs::String::safe_strlen(szDefault, _PASSWORD_LEN)) +
                    1);  // setMemory doesn't assume the null
                         // terminator like setPassword does.
        }

        len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                       : thePassword.getMemorySize();
    }

    auto pMasterPW = pPWData->GetMasterPW();

    if (pPWData->isForCachedKey() && (nullptr != pMasterPW)) {
        *pMasterPW = thePassword;
    } else if (nullptr != buf) {
        if (len > size) { len = size; }

        const auto theSize = static_cast<std::uint32_t>(size);
        const auto theLength = static_cast<std::uint32_t>(len);

        if (thePassword.isPassword()) {
            opentxs::OTPassword::safe_memcpy(
                buf,                              // destination
                theSize,                          // size of destination buffer.
                thePassword.getPassword_uint8(),  // source
                theLength);                       // length of source.
            buf[theLength] = '\0';                // null terminator.
        } else {
            opentxs::OTPassword::safe_memcpy(
                buf,                            // destination
                theSize,                        // size of destination buffer.
                thePassword.getMemory_uint8(),  // source
                theLength);                     // length of source.
        }
    } else {
        //      OT_FAIL_MSG("This should never happen. (souped_up_pass_cb");
    }

    return len;
}
}  // namespace

namespace opentxs
{
api::internal::Native* Factory::Native(
    Flag& running,
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    return new api::implementation::Native(
        running, args, gcInterval, externalPasswordCallback);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Native::Native(
    Flag& running,
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
    : running_(running)
    , gc_interval_(gcInterval)
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , config_()
    , zmq_context_(opentxs::network::zeromq::Context::Factory())
    , signal_handler_(nullptr)
    , log_(opentxs::Factory::Log(zmq_context_))
    , crypto_(nullptr)
    , legacy_(opentxs::Factory::Legacy())
    , client_()
    , server_()
    , zap_(opentxs::Factory::ZAP(zmq_context_))
    , server_args_(args)
    , shutdown_callback_{nullptr}
    , null_callback_{nullptr}
    , default_external_password_callback_{nullptr}
    , external_password_callback_{externalPasswordCallback}
    , rpc_(opentxs::Factory::RPC(*this))
{
    // NOTE: OT_ASSERT is not available until Init() has been called
    assert(legacy_);
    assert(log_);
    assert(zap_);

    if (nullptr == external_password_callback_) {
        setup_default_external_password_callback();
    }

    assert(nullptr != external_password_callback_);

    for (const auto& [key, arg] : args) {
        if (key == OPENTXS_ARG_WORDS) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& word = *arg.cbegin();
            word_list_.setPassword(word.c_str(), word.size());
        } else if (key == OPENTXS_ARG_PASSPHRASE) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& passphrase = *arg.cbegin();
            passphrase_.setPassword(passphrase.c_str(), passphrase.size());
        }
    }

    assert(rpc_);
}

int Native::client_instance(const int count)
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count);
}

const api::client::Manager& Native::Client(const int instance) const
{
    auto& output = client_.at(instance);

    OT_ASSERT(output);

    return *output;
}

const api::Settings& Native::Config(const std::string& path) const
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) {
        config.reset(opentxs::Factory::Settings(String::Factory(path)));
    }

    OT_ASSERT(config);

    lock.unlock();

    return *config;
}

const api::Crypto& Native::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

INTERNAL_PASSWORD_CALLBACK* Native::GetInternalPasswordCallback() const
{
#if defined OT_TEST_PASSWORD
    opentxs::LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": WARNING, OT_TEST_PASSWORD *is* defined. The "
        "internal 'C'-based password callback was just "
        "requested by OT (to pass to OpenSSL). So, returning "
        "the default_pass_cb password callback, which will "
        "automatically return "
        "the 'test' password to OpenSSL, if/when it calls that "
        "callback function. ")
        .Flush();
    return &default_pass_cb;
#else
    return &souped_up_pass_cb;
#endif
}

OTCaller& Native::GetPasswordCaller() const
{
    OT_ASSERT(nullptr != external_password_callback_)

    return *external_password_callback_;
}

void Native::HandleSignals(ShutdownCallback* callback) const
{
    Lock lock(signal_handler_lock_);

    if (nullptr != callback) { shutdown_callback_ = callback; }

    if (false == bool(signal_handler_)) {
        signal_handler_.reset(new Signals(running_));
    }
}

void Native::Init()
{
    Init_Log();
    Init_Crypto();
}

void Native::Init_Crypto()
{
    crypto_.reset(
        opentxs::Factory::Crypto(Config(legacy_->CryptoConfigFilePath())));
}

void Native::Init_Log()
{
    OT_ASSERT(legacy_)

    const auto init = Log::Init(Config(legacy_->LogConfigFilePath()));

    if (false == init) { abort(); }
}

proto::RPCResponse Native::RPC(const proto::RPCCommand& command) const
{
    OT_ASSERT(rpc_);

    return rpc_->Process(command);
}

int Native::server_instance(const int count)
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count) + 1;
}

const api::server::Manager& Native::Server(const int instance) const
{
    auto& output = server_.at(instance);

    OT_ASSERT(output);

    return *output;
}

void Native::setup_default_external_password_callback()
{
    // NOTE: OT_ASSERT is not available yet because we're too early
    // in the startup process

    null_callback_.reset(opentxs::Factory::NullCallback());

    assert(null_callback_);

    default_external_password_callback_.reset(new OTCaller);

    assert(default_external_password_callback_);

    default_external_password_callback_->setCallback(null_callback_.get());

    assert(default_external_password_callback_->isCallbackSet());

    external_password_callback_ = default_external_password_callback_.get();
}

void Native::shutdown()
{
    running_.Off();

    if (nullptr != shutdown_callback_) {
        ShutdownCallback& callback = *shutdown_callback_;
        callback();
    }

    for (const auto& client : client_) {
        if (client) {
            auto wallet = client->OTAPI().GetWallet(nullptr);

            OT_ASSERT(nullptr != wallet);

            wallet->SaveWallet();
        }
    }

    server_.clear();
    client_.clear();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) { config.second.reset(); }

    config_.clear();
}

void Native::start_client(const Lock& lock, const ArgList& args) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(crypto_);
    OT_ASSERT(legacy_);

    const int next = client_.size();
    const auto instance = client_instance(next);
    client_.emplace_back(opentxs::Factory::ClientManager(
        running_,
        args,
        Config(legacy_->ClientConfigFilePath(next)),
        *crypto_,
        zmq_context_,
        legacy_->ClientDataFolder(next),
        instance));
}

const api::client::Manager& Native::StartClient(
    const ArgList& args,
    const int instance) const
{
    Lock lock(lock_);

    const std::size_t count = std::max(0, instance);
    const std::size_t effective = std::min(count, client_.size());

    if (effective == client_.size()) {
        ArgList arguments{args};
        start_client(lock, arguments);
    }

    const auto& output = client_.at(effective);

    OT_ASSERT(output)

    return *output;
}

void Native::start_server(const Lock& lock, const ArgList& args) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(crypto_);

    const auto next{server_.size()};
    const auto instance{server_instance(next)};
    server_.emplace_back(opentxs::Factory::ServerManager(
        running_,
        args,
        *crypto_,
        Config(legacy_->ServerConfigFilePath(next)),
        zmq_context_,
        legacy_->ServerDataFolder(next),
        instance));
}

const api::server::Manager& Native::StartServer(
    const ArgList& args,
    const int instance,
    const bool inproc) const
{
    Lock lock(lock_);

    const std::size_t count = std::max(0, instance);
    const std::size_t effective = std::min(count, server_.size());

    if (effective == server_.size()) {
        ArgList arguments{args};

        if (inproc) {
            auto& inprocSet = arguments[OPENTXS_ARG_INPROC];
            inprocSet.clear();
            inprocSet.insert(std::to_string(server_instance(effective)));
        }

        start_server(lock, arguments);
    }

    const auto& output = server_.at(effective);

    OT_ASSERT(output)

    return *output;
}

const api::network::ZAP& Native::ZAP() const
{
    OT_ASSERT(zap_);

    return *zap_;
}

Native::~Native()
{
    LogSource::Shutdown();
    log_.reset();
}
}  // namespace opentxs::api::implementation
