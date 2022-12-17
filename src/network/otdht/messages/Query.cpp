// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::zeromq::Message

#include "opentxs/network/otdht/Query.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/Types.hpp"

namespace opentxs::factory
{
auto BlockchainSyncQuery() noexcept -> network::otdht::Query
{
    using ReturnType = network::otdht::Query;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncQuery(int arg) noexcept -> network::otdht::Query
{
    using ReturnType = network::otdht::Query;

    return {std::make_unique<ReturnType::Imp>(arg).release()};
}

auto BlockchainSyncQuery_p(int arg) noexcept
    -> std::unique_ptr<network::otdht::Query>
{
    using ReturnType = network::otdht::Query;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(arg).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class Query::Imp final : public Base::Imp
{
public:
    Query* parent_;

    auto asQuery() const noexcept -> const Query& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asQuery();
        }
    }

    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        return serialize_type(out);
    }

    Imp() noexcept
        : Base::Imp()
        , parent_(nullptr)
    {
    }
    Imp(int) noexcept
        : Base::Imp(Base::Imp::default_version_, MessageType::query, {}, {}, {})
        , parent_(nullptr)
    {
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Query::Query(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

Query::~Query()
{
    if (nullptr != Query::imp_) {
        delete Query::imp_;
        Query::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht
