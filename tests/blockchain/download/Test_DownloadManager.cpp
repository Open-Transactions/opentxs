// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Helpers.hpp"
#include "blockchain/DownloadTask.hpp"

constexpr auto batchSize{3};
auto manager_ = DownloadManager{batchSize, 0, 0};

TEST(Test_DownloadManager, initial_state)
{
    EXPECT_FALSE(manager_.batch_ready_);

    const auto batch = manager_.GetBatch();

    EXPECT_FALSE(batch);
    EXPECT_FALSE(batch.isDownloaded());
    EXPECT_EQ(batch.data_.size(), 0);
    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
    EXPECT_EQ(manager_.ready_.size(), 0);
}

TEST(Test_DownloadManager, allocate_batches)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 0);

    manager_.UpdatePosition(manager_.MakePositions(1, [] {
        auto output = std::pmr::vector<std::string>{};

        for (auto i{1}; i < 12; ++i) { output.emplace_back(std::to_string(i)); }

        return output;
    }()));

    EXPECT_EQ(manager_.state_machine_triggers_, 1);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    auto batch0 = manager_.GetBatch();

    ASSERT_EQ(batch0.data_.size(), batchSize);
    EXPECT_EQ(batch0.data_.at(0)->position_, manager_.GetPosition(0));
    EXPECT_EQ(batch0.data_.at(1)->position_, manager_.GetPosition(1));
    EXPECT_EQ(batch0.data_.at(2)->position_, manager_.GetPosition(2));

    auto batch1 = manager_.GetBatch();

    EXPECT_TRUE(batch1);
    ASSERT_EQ(batch1.data_.size(), batchSize);
    EXPECT_EQ(batch1.data_.at(0)->position_, manager_.GetPosition(3));
    EXPECT_EQ(batch1.data_.at(1)->position_, manager_.GetPosition(4));
    EXPECT_EQ(batch1.data_.at(2)->position_, manager_.GetPosition(5));

    auto batch2 = manager_.GetBatch();

    EXPECT_TRUE(batch2);
    ASSERT_EQ(batch2.data_.size(), batchSize);
    EXPECT_EQ(batch2.data_.at(0)->position_, manager_.GetPosition(6));
    EXPECT_EQ(batch2.data_.at(1)->position_, manager_.GetPosition(7));
    EXPECT_EQ(batch2.data_.at(2)->position_, manager_.GetPosition(8));

    auto batch3 = manager_.GetBatch();

    EXPECT_TRUE(batch3);
    ASSERT_EQ(batch3.data_.size(), 2);
    EXPECT_EQ(batch3.data_.at(0)->position_, manager_.GetPosition(9));
    EXPECT_EQ(batch3.data_.at(1)->position_, manager_.GetPosition(10));

    auto batch4 = manager_.GetBatch();

    EXPECT_FALSE(batch4);
    EXPECT_EQ(batch4.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_EQ(manager_.state_machine_triggers_, 1);
    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
    EXPECT_EQ(manager_.ready_.size(), 0);
}

TEST(Test_DownloadManager, previous_batches_not_completed)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 5);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    auto batch = manager_.GetBatch();

    EXPECT_TRUE(batch);
    ASSERT_EQ(batch.data_.size(), batchSize);
    EXPECT_EQ(batch.data_.at(0)->position_, manager_.GetPosition(0));
    EXPECT_EQ(batch.data_.at(1)->position_, manager_.GetPosition(1));
    EXPECT_EQ(batch.data_.at(2)->position_, manager_.GetPosition(2));
    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
    EXPECT_EQ(manager_.ready_.size(), 0);
}

TEST(Test_DownloadManager, partial_download)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 6);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    auto batch0 = manager_.GetBatch();

    EXPECT_TRUE(batch0);
    ASSERT_EQ(batch0.data_.size(), batchSize);
    EXPECT_EQ(batch0.data_.at(0)->position_, manager_.GetPosition(0));
    EXPECT_EQ(batch0.data_.at(1)->position_, manager_.GetPosition(1));
    EXPECT_EQ(batch0.data_.at(2)->position_, manager_.GetPosition(2));

    auto batch1 = manager_.GetBatch();

    EXPECT_TRUE(batch1);
    ASSERT_EQ(batch1.data_.size(), batchSize);
    EXPECT_EQ(batch1.data_.at(0)->position_, manager_.GetPosition(3));
    EXPECT_EQ(batch1.data_.at(1)->position_, manager_.GetPosition(4));
    EXPECT_EQ(batch1.data_.at(2)->position_, manager_.GetPosition(5));

    auto batch2 = manager_.GetBatch();

    EXPECT_TRUE(batch2);
    ASSERT_EQ(batch2.data_.size(), batchSize);
    EXPECT_EQ(batch2.data_.at(0)->position_, manager_.GetPosition(6));
    EXPECT_EQ(batch2.data_.at(1)->position_, manager_.GetPosition(7));
    EXPECT_EQ(batch2.data_.at(2)->position_, manager_.GetPosition(8));

    EXPECT_TRUE(batch0.Download(manager_.GetPosition(0), 1));
    EXPECT_TRUE(batch1.Download(manager_.GetPosition(4), 5));
    EXPECT_FALSE(batch2.Download(manager_.GetPosition(8), 9, 1));
    EXPECT_TRUE(batch2.Download(manager_.GetPosition(8), 9));
    EXPECT_FALSE(batch2.Download(manager_.GetPosition(8), 9));

    EXPECT_FALSE(batch0.isDownloaded());
    EXPECT_FALSE(batch1.isDownloaded());
    EXPECT_FALSE(batch2.isDownloaded());

    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
    EXPECT_EQ(manager_.ready_.size(), 0);
}

TEST(Test_DownloadManager, download_missing)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 9);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);
    EXPECT_TRUE(manager_.ProcessData(1, "0 1"));
    EXPECT_EQ(manager_.ready_.size(), 0);

    auto batch0 = manager_.GetBatch();

    EXPECT_TRUE(batch0);
    ASSERT_EQ(batch0.data_.size(), batchSize);
    EXPECT_EQ(batch0.data_.at(0)->position_, manager_.GetPosition(1));
    EXPECT_EQ(batch0.data_.at(1)->position_, manager_.GetPosition(2));
    EXPECT_EQ(batch0.data_.at(2)->position_, manager_.GetPosition(3));

    auto batch1 = manager_.GetBatch();

    EXPECT_TRUE(batch1);
    ASSERT_EQ(batch1.data_.size(), batchSize);
    EXPECT_EQ(batch1.data_.at(0)->position_, manager_.GetPosition(5));
    EXPECT_EQ(batch1.data_.at(1)->position_, manager_.GetPosition(6));
    EXPECT_EQ(batch1.data_.at(2)->position_, manager_.GetPosition(7));

    auto batch2 = manager_.GetBatch();

    EXPECT_TRUE(batch2);
    ASSERT_EQ(batch2.data_.size(), 2);
    EXPECT_EQ(batch2.data_.at(0)->position_, manager_.GetPosition(9));
    EXPECT_EQ(batch2.data_.at(1)->position_, manager_.GetPosition(10));

    auto batch3 = manager_.GetBatch();

    EXPECT_FALSE(batch3);
    EXPECT_EQ(batch3.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);

    EXPECT_TRUE(batch0.Download(manager_.GetPosition(1), 2));
    EXPECT_TRUE(batch0.Download(manager_.GetPosition(2), 3));
    EXPECT_TRUE(batch0.Download(manager_.GetPosition(3), 4));
    EXPECT_TRUE(batch1.Download(manager_.GetPosition(5), 6));
    EXPECT_TRUE(batch1.Download(manager_.GetPosition(6), 7));
    EXPECT_TRUE(batch1.Download(manager_.GetPosition(7), 8));
    EXPECT_TRUE(batch2.Download(manager_.GetPosition(9), 10));
    EXPECT_TRUE(batch2.Download(manager_.GetPosition(10), 11));

    EXPECT_TRUE(batch0.isDownloaded());
    EXPECT_TRUE(batch1.isDownloaded());
    EXPECT_TRUE(batch2.isDownloaded());

    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
}

TEST(Test_DownloadManager, download_complete)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 12);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.ProcessData(10, "0 1 2 3 4 5 6 7 8 9 10 11"));
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(0));
    EXPECT_EQ(manager_.best_data_, "0 1");
    EXPECT_EQ(manager_.ready_.size(), 0);

    const auto batch = manager_.GetBatch();

    EXPECT_FALSE(batch);
    EXPECT_EQ(batch.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_FALSE(manager_.RunStateMachine());
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(10));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 10 11");
}

TEST(Test_DownloadManager, simple_reorg)
{
    manager_.UpdatePosition(
        manager_.MakePositions(10, {"10a", "11a", "12"}),
        ManagerType::Previous{manager_.GetFinished(8)});

    EXPECT_EQ(manager_.state_machine_triggers_, 13);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    {
        auto batch = manager_.GetBatch();

        EXPECT_TRUE(batch);
        ASSERT_EQ(batch.data_.size(), batchSize);
        EXPECT_EQ(batch.data_.at(0)->position_, manager_.GetPosition(11));
        EXPECT_EQ(batch.data_.at(1)->position_, manager_.GetPosition(12));
        EXPECT_EQ(batch.data_.at(2)->position_, manager_.GetPosition(13));

        EXPECT_TRUE(batch.Download(manager_.GetPosition(11), 110));
        EXPECT_TRUE(batch.Download(manager_.GetPosition(12), 111));
        EXPECT_TRUE(batch.Download(manager_.GetPosition(13), 12));

        EXPECT_TRUE(batch.isDownloaded());
    }

    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(8));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9");
}

TEST(Test_DownloadManager, after_simple_reorg)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 14);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.ProcessData(3, "0 1 2 3 4 5 6 7 8 9 110 111 12"));
    EXPECT_FALSE(manager_.RunStateMachine());

    const auto batch = manager_.GetBatch();

    EXPECT_FALSE(batch);
    EXPECT_EQ(batch.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(13));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 110 111 12");
}

TEST(Test_DownloadManager, reorg_outstanding_batches)
{
    manager_.UpdatePosition(manager_.MakePositions(13, [] {
        auto output = std::pmr::vector<std::string>{};

        for (auto i{13}; i < 22; ++i) {
            output.emplace_back(std::to_string(i));
        }

        return output;
    }()));

    EXPECT_EQ(manager_.state_machine_triggers_, 15);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    {
        auto batch1 = manager_.GetBatch();

        EXPECT_TRUE(batch1);
        ASSERT_EQ(batch1.data_.size(), batchSize);
        EXPECT_EQ(batch1.data_.at(0)->position_, manager_.GetPosition(14));
        EXPECT_EQ(batch1.data_.at(1)->position_, manager_.GetPosition(15));
        EXPECT_EQ(batch1.data_.at(2)->position_, manager_.GetPosition(16));

        auto batch2 = manager_.GetBatch();

        EXPECT_TRUE(batch2);
        ASSERT_EQ(batch2.data_.size(), batchSize);
        EXPECT_EQ(batch2.data_.at(0)->position_, manager_.GetPosition(17));
        EXPECT_EQ(batch2.data_.at(1)->position_, manager_.GetPosition(18));
        EXPECT_EQ(batch2.data_.at(2)->position_, manager_.GetPosition(19));

        auto batch3 = manager_.GetBatch();

        EXPECT_TRUE(batch3);
        ASSERT_EQ(batch3.data_.size(), batchSize);
        EXPECT_EQ(batch3.data_.at(0)->position_, manager_.GetPosition(20));
        EXPECT_EQ(batch3.data_.at(1)->position_, manager_.GetPosition(21));
        EXPECT_EQ(batch3.data_.at(2)->position_, manager_.GetPosition(22));

        auto batch4 = manager_.GetBatch();

        EXPECT_FALSE(batch4);
        EXPECT_EQ(batch4.data_.size(), 0);
        EXPECT_FALSE(manager_.batch_ready_);

        EXPECT_FALSE(batch1.isDownloaded());
        EXPECT_FALSE(batch2.isDownloaded());
        EXPECT_FALSE(batch3.isDownloaded());

        EXPECT_TRUE(batch1.Download(manager_.GetPosition(14), 13));
        EXPECT_TRUE(batch1.Download(manager_.GetPosition(15), 14));
        EXPECT_TRUE(batch1.Download(manager_.GetPosition(16), 15));
        EXPECT_TRUE(batch2.Download(manager_.GetPosition(17), 16));
        EXPECT_TRUE(batch2.Download(manager_.GetPosition(18), 17));
        EXPECT_TRUE(batch2.Download(manager_.GetPosition(19), 18));
        EXPECT_TRUE(batch3.Download(manager_.GetPosition(20), 19));
        EXPECT_TRUE(batch3.Download(manager_.GetPosition(21), 20));
        EXPECT_TRUE(batch3.Download(manager_.GetPosition(22), 21));

        EXPECT_TRUE(batch1.isDownloaded());
        EXPECT_TRUE(batch2.isDownloaded());
        EXPECT_TRUE(batch3.isDownloaded());

        EXPECT_EQ(manager_.state_machine_triggers_, 15);

        manager_.UpdatePosition(manager_.MakePositions(17, {"17a"}));

        EXPECT_EQ(manager_.state_machine_triggers_, 16);
    }

    EXPECT_EQ(manager_.state_machine_triggers_, 18);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.batch_ready_);

    auto batch5 = manager_.GetBatch();

    EXPECT_TRUE(batch5);
    ASSERT_EQ(batch5.data_.size(), 1);
    EXPECT_EQ(batch5.data_.at(0)->position_, manager_.GetPosition(23));

    EXPECT_TRUE(batch5.Download(manager_.GetPosition(23), 117));

    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(13));
    EXPECT_EQ(manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 110 111 12");
}

TEST(Test_DownloadManager, final_state)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 19);
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(
        manager_.ProcessData(4, "0 1 2 3 4 5 6 7 8 9 110 111 12 13 14 15 16"));
    EXPECT_TRUE(manager_.RunStateMachine());
    EXPECT_TRUE(manager_.ProcessData(
        1, "0 1 2 3 4 5 6 7 8 9 110 111 12 13 14 15 16 117"));
    EXPECT_FALSE(manager_.RunStateMachine());

    const auto batch = manager_.GetBatch();

    EXPECT_FALSE(batch);
    EXPECT_EQ(batch.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_EQ(manager_.best_position_, manager_.GetPosition(23));
    EXPECT_EQ(
        manager_.best_data_, "0 1 2 3 4 5 6 7 8 9 110 111 12 13 14 15 16 117");
}
