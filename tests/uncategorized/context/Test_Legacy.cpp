// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <type_traits>

#include "internal/api/Legacy.hpp"
#include "ottest/Basic.hpp"  // IWYU pragma: keep
#include "ottest/fixtures/context/Filename.hpp"

namespace ottest
{
TEST_F(Filename, GetFilenameBin)
{
    ot::UnallocatedCString exp{"filename.bin"};
    ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameBin("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameBin_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin(nullptr).empty());
}

TEST_F(Filename, GetFilenameA)
{
    ot::UnallocatedCString exp{"filename.a"};
    ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameA("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameA_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA(nullptr).empty());
}

TEST_F(Filename, GetFilenameR)
{
    ot::UnallocatedCString exp{"filename.r"};
    ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameR("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameR_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR(nullptr).empty());
}

TEST_F(Filename, GetFilenameRct)
{
    {
        ot::UnallocatedCString exp{"123.rct"};
        ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameRct(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        ot::UnallocatedCString exp{"0.rct"};
        ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameRct(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameRct_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameRct(-1).empty());
}

TEST_F(Filename, GetFilenameCrn)
{
    {
        ot::UnallocatedCString exp{"123.crn"};
        static_assert(
            std::is_same_v<int64_t, opentxs::TransactionNumber>,
            "type is not matching");  // detect if type change
        ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameCrn(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        ot::UnallocatedCString exp{"0.crn"};
        ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameCrn(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameCrn_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameCrn(-1).empty());
}

TEST_F(Filename, GetFilenameSuccess)
{
    ot::UnallocatedCString exp{"filename.success"};
    ot::UnallocatedCString s{
        opentxs::api::Legacy::GetFilenameSuccess("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameSuccess_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess(nullptr).empty());
}

TEST_F(Filename, GetFilenameFail)
{
    ot::UnallocatedCString exp{"filename.fail"};
    ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameFail("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameFail_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail(nullptr).empty());
}

TEST_F(Filename, GetFilenameError)
{
    ot::UnallocatedCString exp{"filename.error"};
    ot::UnallocatedCString s{
        opentxs::api::Legacy::GetFilenameError("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameError_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError(nullptr).empty());
}

TEST_F(Filename, GetFilenameLst)
{
    ot::UnallocatedCString exp{"filename.lst"};
    ot::UnallocatedCString s{opentxs::api::Legacy::GetFilenameLst("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameLst_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError(nullptr).empty());
}
}  // namespace ottest
