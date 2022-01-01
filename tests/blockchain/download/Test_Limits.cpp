// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "Helpers.hpp"
#include "blockchain/DownloadTask.hpp"

constexpr auto batchSize{3};
auto manager_ = DownloadManager{batchSize, 10, 5};

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

TEST(Test_DownloadManager, attempt_excessive_queue)
{
    EXPECT_EQ(manager_.state_machine_triggers_, 0);

    manager_.UpdatePosition(manager_.MakePositions(1, [] {
        auto output = std::pmr::vector<std::string>{};

        for (auto i{1}; i < 21; ++i) { output.emplace_back(std::to_string(i)); }

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
    EXPECT_TRUE(batch1.Download(manager_.GetPosition(3), 4));

    auto batch2 = manager_.GetBatch();

    EXPECT_TRUE(batch2);
    ASSERT_EQ(batch2.data_.size(), batchSize);
    EXPECT_EQ(batch2.data_.at(0)->position_, manager_.GetPosition(6));
    EXPECT_EQ(batch2.data_.at(1)->position_, manager_.GetPosition(7));
    EXPECT_EQ(batch2.data_.at(2)->position_, manager_.GetPosition(8));

    auto batch3 = manager_.GetBatch();

    EXPECT_TRUE(batch3);
    ASSERT_EQ(batch3.data_.size(), 1);
    EXPECT_EQ(batch3.data_.at(0)->position_, manager_.GetPosition(9));

    auto batch4 = manager_.GetBatch();

    EXPECT_FALSE(batch4);
    EXPECT_EQ(batch4.data_.size(), 0);
    EXPECT_FALSE(manager_.batch_ready_);
    EXPECT_EQ(manager_.state_machine_triggers_, 1);
    EXPECT_EQ(manager_.best_position_, manager_.genesis_);
    EXPECT_EQ(manager_.best_data_, "0");
    EXPECT_EQ(manager_.ready_.size(), 0);
}
