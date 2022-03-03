// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

namespace opentxs
{
class Ticket;

class Gatekeeper
{
public:
    struct Imp;

    auto get() const noexcept -> Ticket;

    /// Once this function has been called all future Tickets returned by the
    /// get function will be invalid. This function will block until all valid
    /// tickets have destructed.
    auto shutdown() noexcept -> void;

    Gatekeeper() noexcept;

    ~Gatekeeper();

private:
    std::unique_ptr<Imp> imp_;

    Gatekeeper(const Gatekeeper&) = delete;
    Gatekeeper(Gatekeeper&&) = delete;
    auto operator=(const Gatekeeper&) -> Gatekeeper& = delete;
    auto operator=(Gatekeeper&&) -> Gatekeeper& = delete;
};

class Ticket
{
public:
    struct Imp;

    /// If this function returns true then a shutdown has been ordered. If it
    /// returns false then the shutdown function on the gatekeeper will block
    /// until this Ticket destructs.
    operator bool() const noexcept;

    Ticket(Gatekeeper::Imp&) noexcept;
    Ticket(Ticket&&) noexcept;

    ~Ticket();

private:
    std::unique_ptr<Imp> imp_;

    Ticket(const Ticket&) = delete;
    auto operator=(const Ticket&) -> Ticket& = delete;
    auto operator=(Ticket&&) -> Ticket& = delete;
};
}  // namespace opentxs
