// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace ottest
{
class Interface
{
public:
    virtual bool someFunction(const int& input) = 0;
    virtual bool someConstFunction(const int& input) = 0;
    virtual bool someFunctionWithNoExcept(const int& input) noexcept = 0;
    virtual bool someConstFunctionWithNoExcept(
        const int& input) const noexcept = 0;

    virtual ~Interface() = default;

protected:
    Interface() = default;

private:
    Interface(const Interface&) = delete;
    Interface(Interface&&) = delete;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(const Interface&&) = delete;
};
}  // namespace ottest
