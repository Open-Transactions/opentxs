// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"  // IWYU pragma: associated

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>

#include "opentxs/Version.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::proto
{
UnallocatedCString ToString(const ProtobufType& input);

template <typename Output>
Output Factory(const void* input, const std::size_t size);
template <typename Output, typename Input>
Output Factory(const Pimpl<Input>& input);
template <typename Output, typename Input>
Output Factory(const Input& input);

template <typename Output>
Output Factory(const void* input, const std::size_t size)
{
    static_assert(sizeof(int) <= sizeof(std::size_t));
    assert(size <= static_cast<std::size_t>(std::numeric_limits<int>::max()));

    auto serialized = Output{};
    serialized.ParseFromArray(input, static_cast<int>(size));

    return serialized;
}

template <typename Output, typename Input>
Output Factory(const Pimpl<Input>& input)
{
    return Factory<Output>(input.get());
}

template <typename Output, typename Input>
Output Factory(const Input& input)
{
    return Factory<Output>(input.data(), input.size());
}

template <typename Output>
std::unique_ptr<Output> DynamicFactory(
    const void* input,
    const std::size_t size)
{
    if (std::numeric_limits<int>::max() < size) {
        std::cerr << __func__ << ": input too large\n";

        return {};
    }

    auto output = std::make_unique<Output>();

    if (output) { output->ParseFromArray(input, static_cast<int>(size)); }

    return output;
}

template <typename Output, typename Input>
std::unique_ptr<Output> DynamicFactory(const Pimpl<Input>& input)
{
    return DynamicFactory<Output>(input.get());
}

template <typename Output, typename Input>
std::unique_ptr<Output> DynamicFactory(const Input& input)
{
    return DynamicFactory<Output>(input.data(), input.size());
}

template <typename Output>
Output StringToProto(const String& input)
{
    auto armored = Armored::Factory();
    OTString unconstInput = String::Factory(input.Get());

    if (!armored->LoadFromString(unconstInput)) {
        std::cerr << __func__ << ": failed to decode armored protobuf\n";

        return Output();
    } else {

        return Factory<Output>(Data::Factory(armored));
    }
}
}  // namespace opentxs::proto
