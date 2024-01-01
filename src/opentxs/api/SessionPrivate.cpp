// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/SessionPrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Ciphertext.pb.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/api/Network.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/base/Scheduler.hpp"
#include "opentxs/api/session/base/ZMQ.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/NullCallback.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Shutdown.hpp"

namespace
{
opentxs::PasswordCaller* external_password_callback_{nullptr};

extern "C" auto internal_password_cb(
    char* output,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata) -> std::int32_t
{
    opentxs::assert_false(nullptr == userdata);
    opentxs::assert_false(nullptr == external_password_callback_);

    const bool askTwice = (1 == rwflag);
    const auto& reason = *static_cast<opentxs::PasswordPrompt*>(userdata);
    const auto& api = opentxs::api::SessionPrivate::get_api(reason);
    const auto lock = opentxs::Lock{api.Internal().Lock()};
    auto secret = api.Factory().Secret(0);

    if (false == api.Internal().GetSecret(lock, secret, reason, askTwice)) {
        opentxs::LogError()()("Callback error").Flush();

        return 0;
    }

    if (static_cast<std::uint64_t>(secret.size()) >
        static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
        opentxs::LogError()()("Secret too big").Flush();

        return 0;
    }

    const auto len = std::min(static_cast<std::int32_t>(secret.size()), size);

    if (len <= 0) {
        opentxs::LogError()()("Callback error").Flush();

        return 0;
    }

    std::memcpy(output, secret.data(), len);

    return len;
}
}  // namespace

namespace opentxs::api
{
SessionPrivate::SessionPrivate(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& zmq,
    const std::filesystem::path& dataFolder,
    const int instance,
    NetworkMaker network,
    std::unique_ptr<api::session::internal::Factory> factory)
    : session::base::ZMQ(crypto, zmq, instance)
    , session::base::Scheduler(parent, running)
    , session::base::Storage(
          std::move(args),
          *this,
          endpoints_,
          crypto,
          config,
          parent.Internal().Paths(),
          zmq,
          dataFolder,
          std::move(factory))
    , network_(network(zmq_context_, endpoints_, *this))
    , shutdown_sender_(
          network_.Asio(),
          zmq_context_,
          endpoints_.Shutdown(),
          CString{"api instance "}.append(std::to_string(instance_)))
    , wallet_(nullptr)
    , encrypted_secret_()
    , master_key_lock_()
    , master_secret_()
    , master_key_(std::nullopt)
    , password_duration_(-1)
    , last_activity_()
    , init_promise_()
    , init_(init_promise_.get_future())
    , shutdown_promise_()
{
    auto& caller = parent.Internal().GetPasswordCaller();
    external_password_callback_ = &caller;

    assert_false(nullptr == external_password_callback_);

    if (master_secret_) {
        const auto lock = opentxs::Lock{master_key_lock_};
        bump_password_timer(lock);
    }
}

auto SessionPrivate::asClient() noexcept -> session::internal::Client&
{
    LogAbort()()("not a client session").Abort();
}

auto SessionPrivate::asClientPublic() noexcept -> session::Client&
{
    LogAbort()()("not a client session").Abort();
}

auto SessionPrivate::asNotary() noexcept -> session::internal::Notary&
{
    LogAbort()()("not a notary session").Abort();
}

auto SessionPrivate::asNotaryPublic() noexcept -> session::Notary&
{
    LogAbort()()("not a notary session").Abort();
}

auto SessionPrivate::bump_password_timer(const opentxs::Lock& lock) const
    -> void
{
    last_activity_ = Clock::now();
}

auto SessionPrivate::cleanup() noexcept -> void
{
    network_.Internal().Shutdown();
    wallet_.reset();
    Storage::cleanup();
}

auto SessionPrivate::get_api(const PasswordPrompt& reason) noexcept
    -> const api::Session&
{
    return reason.Internal().API();
}

auto SessionPrivate::GetInternalPasswordCallback() const
    -> INTERNAL_PASSWORD_CALLBACK*
{
    return &internal_password_cb;
}

auto SessionPrivate::GetSecret(
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

    assert_true(master_secret_.has_value());

    auto& callback = *external_password_callback_;
    static const auto defaultPassword =
        factory_.SecretFromText(DefaultPassword());
    auto prompt = factory_.PasswordPrompt(reason.GetDisplayString());
    prompt.Internal().SetPassword(defaultPassword);
    const auto& masterKey = MasterKey(lock);
    auto unlocked = masterKey.Unlock(prompt);
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
        unlocked = masterKey.Unlock(prompt);

        if (false == unlocked) { ++tries; }
    }

    if (false == unlocked) {
        opentxs::LogError()()("Failed to unlock master key").Flush();

        return success;
    }

    const auto decrypted = masterKey.Internal().Decrypt(
        encrypted_secret_,
        master_secret_.value().WriteInto(Secret::Mode::Mem),
        prompt);

    if (false == decrypted) {
        opentxs::LogError()()("Failed to decrypt master secret").Flush();

        return success;
    }

    secret.Assign(master_secret_.value());
    success = true;

    return success;
}

auto SessionPrivate::Paths() const noexcept -> const api::internal::Paths&
{
    return parent_.Internal().Paths();
}

auto SessionPrivate::make_master_key(
    const api::Context& parent,
    const api::session::Factory& factory,
    protobuf::Ciphertext& encrypted,
    std::optional<Secret>& master_secret,
    const api::crypto::Symmetric& symmetric,
    const api::session::Storage& storage) -> opentxs::crypto::symmetric::Key
{
    auto& caller = parent.Internal().GetPasswordCaller();
    using enum opentxs::storage::ErrorReporting;
    const auto have = storage.Internal().Load(encrypted, silent);

    if (have) {

        return symmetric.InternalSymmetric().Key(
            encrypted.key(),
            opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305);
    }

    master_secret = factory.Secret(0);

    assert_true(master_secret.has_value());

    master_secret.value().Randomize(32);

    auto reason = factory.PasswordPrompt("Choose a master password");
    auto masterPassword = factory.Secret(0);
    caller.AskTwice(reason, masterPassword, parent.ProfileId());
    reason.Internal().SetPassword(masterPassword);
    auto output = symmetric.Key(
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305, reason);
    auto saved = output.Internal().Encrypt(
        master_secret.value().Bytes(),
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
        encrypted,
        reason,
        true);

    assert_true(saved);

    saved = storage.Internal().Store(encrypted);

    assert_true(saved);

    return output;
}

auto SessionPrivate::MasterKey(const opentxs::Lock& lock)
    -> opentxs::crypto::symmetric::Key&
{
    if (false == master_key_.has_value()) {
        master_key_.emplace(make_master_key(
            parent_,
            factory_,
            encrypted_secret_,
            master_secret_,
            crypto_.Symmetric(),
            *storage_));
    }

    return *master_key_;
}

auto SessionPrivate::MasterKey(const opentxs::Lock& lock) const
    -> const opentxs::crypto::symmetric::Key&
{
    return const_cast<SessionPrivate*>(this)->MasterKey(lock);
}

void SessionPrivate::SetMasterKeyTimeout(
    const std::chrono::seconds& timeout) const noexcept
{
    const auto lock = opentxs::Lock{master_key_lock_};
    password_duration_ = timeout;
}

auto SessionPrivate::shutdown_complete() noexcept -> void
{
    shutdown_promise_.set_value();
}

auto SessionPrivate::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto SessionPrivate::start(std::shared_ptr<const internal::Session> me) noexcept
    -> void
{
    assert_false(nullptr == me);

    init_promise_.set_value();
    storage_->Internal().Start(me);
}

auto SessionPrivate::Stop() noexcept -> std::future<void>
{
    shutdown_sender_.Activate();

    return shutdown_promise_.get_future();
}

auto SessionPrivate::Storage() const noexcept -> const api::session::Storage&
{
    assert_false(nullptr == storage_);

    return *storage_;
}

// TODO
// void SessionPrivate::password_timeout() const
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
//     auto lock = opentxs::Lock{master_key_lock_, std::defer_lock};
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
//         sleep(250ms);
//     }
// }

auto SessionPrivate::Wallet() const noexcept -> const api::session::Wallet&
{
    assert_false(nullptr == wallet_);

    return wallet_->Self();
}

SessionPrivate::~SessionPrivate() { cleanup(); }
}  // namespace opentxs::api
