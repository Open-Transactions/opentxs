// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "Helpers.hpp"
#include "blockchain/DownloadTask.hpp"

constexpr auto batchSize{1};
constexpr auto count_{3};
auto index_{1};
auto manager_ = DownloadManager{batchSize, 0, 0};
const auto AddBlocks = [] {
    manager_.UpdatePosition(manager_.MakePositions(index_, [] {
        auto output = std::pmr::vector<std::string>{};

        for (auto i{index_}; i < (index_ + count_); ++i) {
            output.emplace_back(std::to_string(i));
        }

        index_ += count_;

        return output;
    }()));
};

TEST(Test_DownloadManager, initial_state)
{
    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
}

TEST(Test_DownloadManager, abc)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    EXPECT_TRUE(batch0);

    batch0.Download(manager_.GetPosition(0), 1);
    batch0.Finish();

    EXPECT_FALSE(batch0);
    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(0));
    EXPECT_EQ(manager_.best_data_, "0 1");

    batch1.Download(manager_.GetPosition(1), 2);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(1));
    EXPECT_EQ(manager_.best_data_, "0 1 2");

    batch2.Download(manager_.GetPosition(2), 3);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(2));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3");
}

TEST(Test_DownloadManager, acb)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    batch0.Download(manager_.GetPosition(3), 4);
    batch0.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(3));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4");

    batch2.Download(manager_.GetPosition(5), 6);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(3));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4");

    batch1.Download(manager_.GetPosition(4), 5);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(5));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6");
}

TEST(Test_DownloadManager, bca)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    batch1.Download(manager_.GetPosition(7), 8);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(5));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6");

    batch2.Download(manager_.GetPosition(8), 9);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(5));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6");

    batch0.Download(manager_.GetPosition(6), 7);
    batch0.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(8));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9");
}

TEST(Test_DownloadManager, bac)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    batch1.Download(manager_.GetPosition(10), 11);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(8));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9");

    batch0.Download(manager_.GetPosition(9), 10);
    batch0.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(10));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11");

    batch2.Download(manager_.GetPosition(11), 12);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(11));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12");
}

TEST(Test_DownloadManager, cab)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    batch2.Download(manager_.GetPosition(14), 15);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(11));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12");

    batch0.Download(manager_.GetPosition(12), 13);
    batch0.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(12));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12 13");

    batch1.Download(manager_.GetPosition(13), 14);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(14));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15");
}

TEST(Test_DownloadManager, cba)
{
    AddBlocks();

    EXPECT_TRUE(manager_.RunStateMachine());

    auto batch0 = manager_.GetBatch();
    auto batch1 = manager_.GetBatch();
    auto batch2 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), 1);
    ASSERT_EQ(batch1.data_.size(), 1);
    ASSERT_EQ(batch2.data_.size(), 1);

    batch2.Download(manager_.GetPosition(17), 18);
    batch2.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(14));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15");

    batch1.Download(manager_.GetPosition(16), 17);
    batch1.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(14));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15");

    batch0.Download(manager_.GetPosition(15), 16);
    batch0.Finish();

    EXPECT_TRUE(manager_.RunStateMachine());

    manager_.ProcessData();

    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(17));
    EXPECT_EQ(
        manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18");
}
