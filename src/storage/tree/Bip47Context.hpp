#ifndef OPENTXS_STORAGE_TREE_BIP47_CONTEXT_HPP
#define OPENTXS_STORAGE_TREE_BIP47_CONTEXT_HPP

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include "Node.hpp"

#include <list>
#include <map>
#include <set>

namespace opentxs
{
namespace storage
{

/*
better name:
NodeList<Bip47Context>
*/
class Bip47Contexts;

class Bip47Context : public Node
{
private:
    friend class Bip47Contexts;

    std::string id_;
    std::string alias_;
    std::size_t index_{0};
    std::map<proto::ContactItemType, proto::Bip47Chain> items_;

    void init(const std::string& hash) override;
    bool save(const Lock& lock) const override;
    proto::Bip47Context serialize(const Lock& lock) const;

    Bip47Context(
        const opentxs::api::storage::Driver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias);

    Bip47Context() = delete;
    Bip47Context(const Bip47Context&) = delete;
    Bip47Context(Bip47Context&&) = delete;
    Bip47Context operator=(const Bip47Context&) = delete;
    Bip47Context operator=(Bip47Context&&) = delete;

public:
    proto::Bip47Context Items() const;
    ~Bip47Context() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_BIP47_CONTEXT_HPP
