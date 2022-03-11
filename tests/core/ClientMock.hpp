// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#pragma once

#include <gmock/gmock.h>

#include "StorageMock.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"

namespace opentxs
{

namespace api::session::internal
{

class Session : virtual public api::Session
{
};

class Client : public api::session::Client, virtual public Session
{
};

}  // namespace api::session::internal

class ClientMock : virtual public api::session::internal::Client
{
private:
    api::session::Storage& storage_;

public:
    ClientMock(api::session::Storage& storage)
        : storage_(storage)
    {
    }

    MOCK_METHOD(const api::session::Activity&, Activity, (), (const, final));
    MOCK_METHOD(const api::session::Contacts&, Contacts, (), (const, final));
    MOCK_METHOD(const api::session::OTX&, OTX, (), (const, final));
    MOCK_METHOD(const api::session::UI&, UI, (), (const, final));
    MOCK_METHOD(const api::session::Workflow&, Workflow, (), (const, final));
    MOCK_METHOD(const api::network::ZMQ&, ZMQ, (), (const, final));
    MOCK_METHOD(bool, Cancel, (const int task), (const, final));
    MOCK_METHOD(
        bool,
        Reschedule,
        (const int, const std::chrono::seconds&),
        (const, final));
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const
        -> int final
    {
        return 0;
    }
    //    MOCK_METHOD(int, Schedule, (const std::chrono::seconds, const
    //    PeriodicTask&, const std::chrono::seconds&), (const, final));

    MOCK_METHOD(api::Settings&, Config, (), (const, noexcept, final));
    MOCK_METHOD(
        const api::session::Crypto&,
        Crypto,
        (),
        (const, noexcept, final));
    MOCK_METHOD(
        const UnallocatedCString&,
        DataFolder,
        (),
        (const, noexcept, final));
    MOCK_METHOD(
        const api::session::Endpoints&,
        Endpoints,
        (),
        (const, noexcept, final));
    MOCK_METHOD(
        const api::session::Factory&,
        Factory,
        (),
        (const, noexcept, final));
    MOCK_METHOD(const Options&, GetOptions, (), (const, noexcept, final));
    MOCK_METHOD(int, Instance, (), (const, noexcept, final));
    MOCK_METHOD(
        const api::network::Network&,
        Network,
        (),
        (const, noexcept, final));
    MOCK_METHOD(QObject*, QtRootObject, (), (const, noexcept, final));
    MOCK_METHOD(
        void,
        SetMasterKeyTimeout,
        (const std::chrono::seconds&),
        (const, noexcept, final));
    MOCK_METHOD(
        const api::session::Wallet&,
        Wallet,
        (),
        (const, noexcept, final));

    //    MOCK_METHOD(
    //        const api::session::internal::Client&,
    //        InternalClient,
    //        (),
    //        (noexcept, final));
    //    MOCK_METHOD(api::session::internal::Client&, InternalClient, (),
    //    (noexept, final)); OPENTXS_NO_EXPORT virtual auto InternalClient()
    //    const
    //        -> const api::session::internal::Client& noexcept final
    //    {
    //        return *this;
    //    }

    auto InternalClient() const noexcept
        -> const api::session::internal::Client& final
    {
        return *this;
    }
    auto InternalClient() noexcept -> api::session::internal::Client& final
    {
        return *this;
    }

    auto Storage() const noexcept -> const api::session::Storage& final
    {
        return storage_;
    }
    auto Internal() const noexcept
        -> const api::session::internal::Session& final
    {
        return *this;
    }
    auto Internal() noexcept -> api::session::internal::Session& final
    {
        return *this;
    }
};

}  // namespace opentxs

//}