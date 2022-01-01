// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/StorageContacts.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace proto
{
class Contact;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Contacts final : public Node
{
public:
    auto Alias(const std::string& id) const -> std::string;
    auto List() const -> ObjectList final;
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& output,
        std::string& alias,
        const bool checking) const -> bool;
    auto NymOwner(std::string nym) const -> std::string;
    auto Save() const -> bool;

    auto Delete(const std::string& id) -> bool;
    auto SetAlias(const std::string& id, const std::string& alias) -> bool;
    auto Store(const proto::Contact& data, const std::string& alias) -> bool;

    ~Contacts() final = default;

private:
    friend Tree;
    using ot_super = Node;
    using Address = std::pair<contact::ClaimType, std::string>;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber MergeIndexVersion{1};
    static const VersionNumber NymIndexVersion{1};

    std::pmr::map<std::string, std::pmr::set<std::string>> merge_;
    std::pmr::map<std::string, std::string> merged_;
    mutable std::pmr::map<std::string, std::string> nym_contact_index_;

    void extract_nyms(const Lock& lock, const proto::Contact& data) const;
    auto nomalize_id(const std::string& input) const -> const std::string&;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageContacts;

    void init(const std::string& hash) final;
    void reconcile_maps(const Lock& lock, const proto::Contact& data);
    void reverse_merged();

    Contacts(const Driver& storage, const std::string& hash);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts = delete;
    auto operator=(Contacts&&) -> Contacts = delete;
};
}  // namespace opentxs::storage
