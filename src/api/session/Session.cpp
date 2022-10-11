// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "api/session/Session.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "api/session/base/Scheduler.hpp"
#include "api/session/base/ZMQ.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/crypto/Symmetric.hpp"
#include "internal/api/network/Network.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Types.hpp"
#include "util/NullCallback.hpp"
#include "util/ScopeGuard.hpp"

namespace
{
opentxs::PasswordCaller* external_password_callback_{nullptr};

extern "C" auto internal_password_cb(
    char* output,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata) -> std::int32_t
{
    OT_ASSERT(nullptr != userdata);
    OT_ASSERT(nullptr != external_password_callback_);

    const bool askTwice = (1 == rwflag);
    const auto& reason = *static_cast<opentxs::PasswordPrompt*>(userdata);
    const auto& api = opentxs::api::session::imp::Session::get_api(reason);
    opentxs::Lock lock(api.Internal().Lock());
    auto secret = api.Factory().Secret(0);

    if (false == api.Internal().GetSecret(lock, secret, reason, askTwice)) {
        opentxs::LogError()(__func__)(": Callback error").Flush();

        return 0;
    }

    if (static_cast<std::uint64_t>(secret.size()) >
        static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
        opentxs::LogError()(__func__)(": Secret too big").Flush();

        return 0;
    }

    const auto len = std::min(static_cast<std::int32_t>(secret.size()), size);

    if (len <= 0) {
        opentxs::LogError()(__func__)(": Callback error").Flush();

        return 0;
    }

    std::memcpy(output, secret.data(), len);

    return len;
}
}  // namespace

namespace opentxs::api::session::imp
{
Session::Session(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& zmq,
    const std::filesystem::path& dataFolder,
    const int instance,
    NetworkMaker network,
    std::unique_ptr<api::session::Factory> factory)
    : session::ZMQ(zmq, instance)
    , session::Scheduler(parent, running)
    , base::Storage(
          running,
          std::move(args),
          *this,
          endpoints_,
          crypto,
          config,
          parent.Internal().Legacy(),
          parent.Asio(),
          zmq,
          dataFolder,
          std::move(factory))
    , network_(network(zmq_context_, endpoints_, *this))
    , shutdown_sender_(
          network_->Asio(),
          zmq_context_,
          endpoints_.Shutdown(),
          CString{"api instance "}.append(std::to_string(instance_)))
    , wallet_(nullptr)
    , encrypted_secret_()
    , master_key_lock_()
    , master_secret_()
    , master_key_(make_master_key(
          parent,
          factory_,
          encrypted_secret_,
          master_secret_,
          crypto_.Symmetric(),
          *storage_))
    , password_duration_(-1)
    , last_activity_()
    , init_promise_()
    , init_(init_promise_.get_future())
    , shutdown_promise_()
    , self_()
{
    OT_ASSERT(network_);

    if (master_secret_) {
        opentxs::Lock lock(master_key_lock_);
        bump_password_timer(lock);
    }
}

void Session::bump_password_timer(const opentxs::Lock& lock) const
{
    last_activity_ = Clock::now();
}

auto Session::cleanup() noexcept -> void
{
    network_->Internal().Shutdown();
    wallet_.reset();
    Storage::cleanup();
}

auto Session::get_api(const PasswordPrompt& reason) noexcept
    -> const api::Session&
{
    return reason.Internal().API();
}

auto Session::GetInternalPasswordCallback() const -> INTERNAL_PASSWORD_CALLBACK*
{
    return &internal_password_cb;
}

auto Session::GetSecret(
    const opentxs::Lock& lock,
    Secret& secret,
    const PasswordPrompt& reason,
    const bool twice,
    const UnallocatedCString& key) const -> bool
{
    bump_password_timer(lock);

    if (master_secret_.has_value()) {
        secret.Assign(master_secret_.value());

        return true;
    }

    auto success{false};
    auto postcondition = ScopeGuard{[this, &success] {
        if (false == success) { master_secret_ = {}; }
    }};

    master_secret_ = factory_.Secret(0);

    OT_ASSERT(master_secret_.has_value());

    auto& callback = *external_password_callback_;
    static const auto defaultPassword =
        factory_.SecretFromText(DefaultPassword());
    auto prompt = factory_.PasswordPrompt(reason.GetDisplayString());
    prompt.Internal().SetPassword(defaultPassword);
    auto unlocked = master_key_->Unlock(prompt);
    auto tries{0};

    if ((false == unlocked) && (tries < 3)) {
        auto masterPassword = factory_.Secret(256);
        const UnallocatedCString password_key{
            key.empty() ? parent_.ProfileId() : key};

        if (twice) {
            callback.AskTwice(reason, masterPassword, password_key);
        } else {
            callback.AskOnce(reason, masterPassword, password_key);
        }

        prompt.Internal().SetPassword(masterPassword);
        unlocked = master_key_->Unlock(prompt);

        if (false == unlocked) { ++tries; }
    }

    if (false == unlocked) {
        opentxs::LogError()(OT_PRETTY_CLASS())("Failed to unlock master key")
            .Flush();

        return success;
    }

    const auto decrypted = master_key_->Decrypt(
        encrypted_secret_,
        prompt,
        master_secret_.value().WriteInto(Secret::Mode::Mem));

    if (false == decrypted) {
        opentxs::LogError()(OT_PRETTY_CLASS())(
            "Failed to decrypt master secret")
            .Flush();

        return success;
    }

    secret.Assign(master_secret_.value());
    success = true;

    return success;
}

auto Session::GetShared() const noexcept -> std::shared_ptr<const api::Session>
{
    init_.get();
    auto out = self_.lock();

    OT_ASSERT(out);

    return out;
}

auto Session::Legacy() const noexcept -> const api::Legacy&
{
    return parent_.Internal().Legacy();
}

auto Session::make_master_key(
    const api::Context& parent,
    const api::session::Factory& factory,
    proto::Ciphertext& encrypted,
    std::optional<Secret>& master_secret,
    const api::crypto::Symmetric& symmetric,
    const api::session::Storage& storage) -> OTSymmetricKey
{
    auto& caller = parent.Internal().GetPasswordCaller();
    external_password_callback_ = &caller;

    OT_ASSERT(nullptr != external_password_callback_);

    const auto have = storage.Load(encrypted, true);

    if (have) {

        return symmetric.InternalSymmetric().Key(
            encrypted.key(),
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);
    }

    master_secret = factory.Secret(0);

    OT_ASSERT(master_secret.has_value());

    master_secret.value().Randomize(32);

    auto reason = factory.PasswordPrompt("Choose a master password");
    auto masterPassword = factory.Secret(0);
    caller.AskTwice(reason, masterPassword, parent.ProfileId());
    reason.Internal().SetPassword(masterPassword);
    auto output = symmetric.Key(
        reason, opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);
    auto saved = output->Encrypt(
        master_secret.value().Bytes(),
        reason,
        encrypted,
        true,
        opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305);

    OT_ASSERT(saved);

    saved = storage.Store(encrypted);

    OT_ASSERT(saved);

    return output;
}

auto Session::MasterKey(const opentxs::Lock& lock) const
    -> const opentxs::crypto::key::Symmetric&
{
    return master_key_;
}

void Session::SetMasterKeyTimeout(
    const std::chrono::seconds& timeout) const noexcept
{
    opentxs::Lock lock(master_key_lock_);
    password_duration_ = timeout;
}

auto Session::shutdown_complete() noexcept -> void
{
    shutdown_promise_.set_value();
}

auto Session::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto Session::Start(std::shared_ptr<const api::Session> me) noexcept -> void
{
    OT_ASSERT(me);

    self_ = std::move(me);
    init_promise_.set_value();
    network_->Internal().Start(
        self_.lock(),
        crypto_.Blockchain(),
        parent_.Internal().Legacy(),
        data_folder_,
        args_);
}

auto Session::Stop() noexcept -> std::future<void>
{
    shutdown_sender_.Activate();

    return shutdown_promise_.get_future();
}

auto Session::Storage() const noexcept -> const api::session::Storage&
{
    OT_ASSERT(storage_);

    return *storage_;
}

void Session::storage_gc_hook()
{
    if (storage_) { storage_->RunGC(); }
}

// TODO
// void Session::password_timeout() const
// {
//     struct Cleanup {
//         std::atomic<bool>& running_;
//
//         Cleanup(std::atomic<bool>& running)
//             : running_(running)
//         {
//             running_.store(true);
//         }
//
//         ~Cleanup() { running_.store(false); }
//     };
//
//     opentxs::Lock lock(master_key_lock_, std::defer_lock);
//     Cleanup cleanup(timeout_thread_running_);
//
//     while (running_) {
//         lock.lock();
//
//         // Negative durations means never time out
//         if (0 > password_duration_.count()) { return; }
//
//         const auto now = Clock::now();
//         const auto interval = now - last_activity_;
//
//         if (interval > password_duration_) {
//             master_secret_.reset();
//
//             return;
//         }
//
//         lock.unlock();
//         Sleep(250ms);
//     }
// }

auto Session::Wallet() const noexcept -> const api::session::Wallet&
{
    OT_ASSERT(wallet_);

    return *wallet_;
}

Session::~Session() { cleanup(); }
}  // namespace opentxs::api::session::imp
