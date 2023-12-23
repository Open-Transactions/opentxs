// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/base/Storage.hpp"  // IWYU pragma: associated

#include <filesystem>
#include <utility>

#include "internal/api/session/Crypto.hpp"
#include "internal/api/session/Storage.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/identifier/HDSeed.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace opentxs::api::session::base
{
Storage::Storage(
    Options&& args,
    const api::internal::Session& session,
    const api::session::Endpoints& endpoints,
    const api::Crypto& crypto,
    const api::Settings& config,
    const api::internal::Paths& legacy,
    const opentxs::network::zeromq::Context& zmq,
    const std::filesystem::path& dataFolder,
    std::unique_ptr<api::session::internal::Factory> factory)
    : config_(config)
    , args_(std::move(args))
    , data_folder_(dataFolder)
    , storage_config_(legacy, config_, args_, dataFolder)
    , factory_p_(std::move(factory))
    , factory_(factory_p_->SessionPublic())
    , storage_(opentxs::factory::StorageAPI(crypto, factory_, storage_config_))
    , crypto_p_(factory::SessionCryptoAPI(
          const_cast<api::Crypto&>(crypto),
          session,
          endpoints,
          factory_,
          *storage_,
          zmq))
    , crypto_(*crypto_p_)
    , storage_encryption_key_()
{
    assert_false(nullptr == storage_);
}

auto Storage::cleanup() noexcept -> void
{
    crypto_p_->InternalSession().Cleanup();
    factory_p_.reset();
}

auto Storage::init(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const api::crypto::Seed& seeds) noexcept -> void
{
    if (crypto::HaveHDKeys() &&
        (false == storage_config_.fs_encrypted_backup_directory_.empty())) {
        auto seed = seeds.DefaultSeed().first;

        if (seed.empty()) {
            LogError()()("No default seed.").Flush();
        } else {
            LogError()()("Default seed is: ")(seed, crypto)(".").Flush();
        }

        auto reason =
            factory.PasswordPrompt("Initializaing storage encryption");
        storage_encryption_key_ = seeds.GetStorageKey(seed, reason);

        if (storage_encryption_key_) {
            LogDetail()()("Obtained storage key ")(
                storage_encryption_key_.ID(reason), crypto_)
                .Flush();
        } else {
            LogError()()("Failed to load storage key ")(seed, crypto)(".")
                .Flush();
        }
    }

    start();
}

auto Storage::start() noexcept -> void
{
    assert_false(nullptr == storage_);

    auto& storage = storage_->Internal();

    storage.InitBackup();

    if (storage_encryption_key_) {
        storage.InitEncryptedBackup(storage_encryption_key_);
    }

    storage.start();
    storage.Upgrade();
}

Storage::~Storage() = default;
}  // namespace opentxs::api::session::base
