// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <class C>
class SharedPimpl
{
public:
    operator const C&() const noexcept { return *pimpl_; }

    auto operator->() const -> const C* { return pimpl_.get(); }

    template <typename Type>
    auto as() noexcept -> SharedPimpl<Type>
    {
        return SharedPimpl<Type>{std::static_pointer_cast<const Type>(pimpl_)};
    }
    template <typename Type>
    auto dynamic() noexcept(false) -> SharedPimpl<Type>
    {
        auto pointer = std::dynamic_pointer_cast<const Type>(pimpl_);

        if (pointer) {
            return SharedPimpl<Type>{std::move(pointer)};
        } else {
            throw std::runtime_error("Invalid dynamic cast");
        }
    }
    auto get() const noexcept -> const C& { return *pimpl_; }

    explicit SharedPimpl(const std::shared_ptr<const C>& in) noexcept
        : pimpl_(in)
    {
        if (false == bool(pimpl_)) {
            std::cout << PrintStackTrace() << '\n';
            abort();
        }
    }
    SharedPimpl() = delete;
    SharedPimpl(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl(SharedPimpl&& rhs) noexcept = default;
    auto operator=(const SharedPimpl& rhs) noexcept -> SharedPimpl& = default;
    auto operator=(SharedPimpl&& rhs) noexcept -> SharedPimpl& = default;

    ~SharedPimpl() = default;

private:
    std::shared_ptr<const C> pimpl_{nullptr};
};  // class SharedPimpl
}  // namespace opentxs
