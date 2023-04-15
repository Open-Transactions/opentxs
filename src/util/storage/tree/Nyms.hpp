// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNymList.pb.h>
#include <memory>

#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Nym.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Nyms final : public Node
{
public:
    auto Default() const -> identifier::Nym;
    auto Exists(const identifier::Nym& id) const -> bool;
    auto LocalNyms() const noexcept -> Set<identifier::Nym>;
    auto Map(NymLambda lambda) const -> void;
    auto Migrate(const Driver& to) const -> bool final;
    auto NeedUpgrade() const noexcept -> bool;
    auto Nym(const identifier::Nym& id) const -> const storage::Nym&;

    auto mutable_Nym(const identifier::Nym& id) -> Editor<storage::Nym>;
    auto RelabelThread(
        const UnallocatedCString& threadID,
        const UnallocatedCString label) -> bool;
    auto SetDefault(const identifier::Nym& id) -> bool;
    auto Upgrade() noexcept -> void;

    Nyms() = delete;
    Nyms(const Nyms&) = delete;
    Nyms(Nyms&&) = delete;
    auto operator=(const Nyms&) -> Nyms = delete;
    auto operator=(Nyms&&) -> Nyms = delete;

    ~Nyms() final;

private:
    friend Tree;

    static constexpr auto current_version_ = VersionNumber{5};

    mutable opentxs::Map<identifier::Nym, std::unique_ptr<storage::Nym>> nyms_;
    Set<identifier::Nym> local_nyms_;
    identifier::Nym default_local_nym_;

    auto nym(const identifier::Nym& id) const -> storage::Nym*;
    auto nym(const Lock& lock, const identifier::Nym& id) const
        -> storage::Nym*;
    auto save(storage::Nym* nym, const Lock& lock, const identifier::Nym& id)
        -> void;

    auto init(const UnallocatedCString& hash) -> void final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;
    auto set_default(const Lock& lock, const identifier::Nym& id) -> void;
    auto upgrade_create_local_nym_index(const Lock& lock) noexcept -> void;

    Nyms(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash);
};
}  // namespace opentxs::storage
