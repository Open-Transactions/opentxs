// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>

#include "opentxs/Version.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "util/LMDB.hpp"

namespace opentxs::util
{
// Stores items in a memory mapped, sparse backing file.
struct IndexData {
    using MemoryPosition = std::size_t;
    using ItemSize = std::size_t;

    MemoryPosition position_{};
    ItemSize size_{};
};

class MappedFileStorage
{
protected:
    using LMDB = opentxs::storage::lmdb::LMDB;
    using UpdateCallback = std::function<bool(LMDB::Transaction&)>;

    LMDB& lmdb_;

    // NOTE: this class performs no locking. Inheritors must ensure these
    // functions are not called simultaneously from multiple threads.
    auto get_read_view(const IndexData& index) const noexcept -> ReadView;
    // Default construct an IndexData if you just want to append a new item, or
    // supply an existing IndexData if you want to (potentially) replace the
    // existing item. An existing item will be overwritten if the size of the
    // old items matches the size of the new item; to do otherwise would be
    // madness. If the size doesn't match then space will be allocated at the
    // end of the file and we just forget about the old data.
    //
    // Regardless after this function is called the supplied index will be
    // updated to the location at which the return value points so you should
    // retain that index for future calls to get_read_view.
    //
    // Storing a zero byte object is not allowed.
    auto get_write_view(
        LMDB::Transaction& tx,
        IndexData& index,
        UpdateCallback&& cb,  // will be called if index is changed
        std::size_t size) const noexcept -> WritableView;
    auto get_write_view(
        LMDB::Transaction& tx,
        IndexData& index,
        std::size_t size) const noexcept -> WritableView;

    MappedFileStorage(
        opentxs::storage::lmdb::LMDB& lmdb,
        const UnallocatedCString& basePath,
        const UnallocatedCString filenamePrefix,
        int table,
        std::size_t key) noexcept(false);

    virtual ~MappedFileStorage();

private:
    struct Imp;

    mutable std::unique_ptr<Imp> imp_p_;
    Imp& imp_;
};
}  // namespace opentxs::util
