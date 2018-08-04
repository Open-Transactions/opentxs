#ifndef OPENTXS_STORAGE_TREE_BIP47_CONTEXTS_HPP
#define OPENTXS_STORAGE_TREE_BIP47_CONTEXTS_HPP

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"

#include "Node.hpp"

namespace opentxs
{
namespace storage
{

class Nym;
class Bip47Context;

class Bip47Contexts : public Node
{
private:
    friend class Nym;

    mutable std::map<std::string, std::unique_ptr<class Bip47Context>>
        bip47contexts_;
    bool save(const Lock& lock) const override;
    proto::StorageNymList serialize() const;
    class Bip47Context* bip47context(const std::string& id) const;
    class Bip47Context* bip47context(
        const std::string& id,
        const std::unique_lock<std::mutex>& lock) const;
    Bip47Contexts(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);

    void init(const std::string& hash) override;
    void save(
        class Bip47Context* bip47context,
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);

    Bip47Contexts() = delete;
    Bip47Contexts(const Bip47Contexts&) = delete;
    Bip47Contexts(Bip47Contexts&&) = delete;
    Bip47Contexts operator=(const Bip47Contexts&) = delete;
    Bip47Contexts operator=(Bip47Contexts&&) = delete;

public:
    bool Migrate(const opentxs::api::storage::Driver& to) const override;
    const class Bip47Context& Bip47Context(const std::string& id) const;
    Editor<class Bip47Context> mutable_Bip47Context(const std::string& id);
    bool Rename(const std::string& existingID, const std::string& newID);
    ~Bip47Contexts() = default;
};
}  // namespace storage
}  // namespace opentxs

#endif  // OPENTXS_STORAGE_TREE_BIP47CONTEXTS_HPP
