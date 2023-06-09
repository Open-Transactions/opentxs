// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/blockchain/Blocks.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "opentxs/opentxs.hpp"

namespace ottest
{
using namespace std::literals;

auto GetTnDashBlock7000() noexcept
    -> std::pair<opentxs::blockchain::block::Hash, opentxs::ReadView>
{
    static constexpr auto idHex =
        "bf44608ca14df02e1f563e99a95f2818f95fef6db8b2a7789171878b1e000000"sv;
    static constexpr auto hex =
        "010000205d1b3522921a4ba6794d673cb87ca52c9658f10f02d2fadf482a40141f000000f2ef87de6ce3da5b71c542ec4c13a0de58896bd39451e7c141fc4970b3c339536729185c3bc72d1d0046c32d0203000500010000000000000000000000000000000000000000000000000000000000000000ffffffff1202581b0e2f5032506f6f6c2d74444153482fffffffff044d125b96010000001976a9144f79c383bc5d3e9d4d81b98f87337cedfa78953688ac40c3609a010000001976a914bafef41416718b231d5ca0143dccbc360d06b77688acf3b00504000000001976a914badadfdebaa6d015a0299f23fbc1fcbdd72ba96f88ac00000000000000002a6a28421df280ea438199057112738c5149e4307689d1201c96a66fbe83f4aa0c4016000000000300000000000000260100581b0000000000000000000000000000000000000000000000000000000000000000000003000600000000000000fd49010100581b0000010001e3aeae4a2d013f6bdd3525318bcc579f95c3420e8897a23e8a479f1c39000000320000000000000032000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"sv;
    static const auto id =
        opentxs::blockchain::block::Hash{opentxs::IsHex, idHex};
    static const auto data = opentxs::ByteArray{opentxs::IsHex, hex};

    return std::make_pair(id, data.Bytes());
}
}  // namespace ottest
