#ifndef UNTITLED1_LIBRARY_H
#define UNTITLED1_LIBRARY_H
#include <algorithm>
#include <memory>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <ostream>
#include <random>
#include "SkipNode.h"
#include <shared_mutex>


template<typename Key, typename T, typename Allocator=std::allocator<T> >
class SkipSet {
    using Node = SkipNode<Key, T>;
    Node *root;
    Node *tail;
    int m_currLevel;
    int m_maxLevel;
    size_t m_size;
    mutable std::shared_mutex mu;

    [[nodiscard]] int randomLevel() const {
        //implicitly static
        thread_local std::mt19937 gen(std::random_device{}());
        thread_local std::geometric_distribution<> dst(0.5);
        int lvl{0};
        while (dst(gen) == 0 && lvl < m_maxLevel) {
            lvl++;
        }
        return lvl;
    }

public:
    using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeTraits = std::allocator_traits<NodeAlloc>;
    NodeAlloc m_alloc;

    explicit SkipSet()
        : m_currLevel{0}, m_maxLevel{16}, m_size{0} {
        root = NodeTraits::allocate(m_alloc, 1);
        NodeTraits::construct(m_alloc, root, Key{}, T{}, 16);
        tail = nullptr;
    }

    SkipSet(std::initializer_list<std::pair<Key, T> > init)
        : SkipSet() {
        for (auto &&p: init) {
            insert(p.first, p.second);
        }
    }

    ~SkipSet() {
        clear();
        NodeTraits::destroy(m_alloc, root);
        NodeTraits::deallocate(m_alloc, root, 1);
    }

    [[nodiscard]] bool empty() const noexcept {
        std::shared_lock lock(mu);
        return m_size == 0;
    }

    [[nodiscard]] size_t size() const noexcept {
        std::shared_lock lock(mu);
        return m_size;
    }

    void clear() noexcept {
        std::unique_lock lock(mu);
        Node *current = root->forward[0];
        while (current) {
            Node *next = current->forward[0];
            NodeTraits::destroy(m_alloc, current);
            NodeTraits::deallocate(m_alloc, current, 1);
            current = next;
        }
        for (int i = 0; i <= m_maxLevel; ++i)
            root->forward[i] = nullptr;

        tail = nullptr;
        m_currLevel = 0;
        m_size = 0;
    }


    T front() const {
        std::shared_lock lock(mu);
        if (m_size == 0) {
            throw std::runtime_error("Empty SkipSet");
        }
        return root->forward[0]->value;
    }

    T back() const {
        std::shared_lock lock(mu);
        if (m_size == 0) {
            throw std::runtime_error("Empty SkipSet");
        }
        return tail->value;
    }

    [[nodiscard]] size_t maxlevel() const noexcept {
        std::shared_lock lock(mu);
        return m_maxLevel;
    }

    /*TODO: "add false"
     */
    template<typename U>
    bool insert(const Key &k, U &&v) {
        std::unique_lock lock(mu);
        std::vector<Node *> update(m_maxLevel + 1, nullptr);
        Node *x = root;

        //find if curr node points to new node
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && k > x->forward[i]->key)
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[0];
        if (x && x->key == k) {
            //return Key position
            if (x->forward[0] == nullptr) {
                tail = x;
            }
            return false;
        }

        int lvl{randomLevel()};
        //check correct level
        if (lvl > m_currLevel) {
            for (int i = m_currLevel + 1; i <= lvl; i++)
                update[i] = root;
            m_currLevel = lvl;
        }

        Node *tmp = NodeTraits::allocate(m_alloc, 1);
        NodeTraits::construct(m_alloc, tmp, k, std::forward<U>(v), lvl);
        //insert links between all nodes
        for (int i = 0; i <= lvl; i++) {
            tmp->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = tmp;
        }
        m_size++;
        if (tmp->forward[0] == nullptr) {
            tail = tmp;
        }

        return true;
    }

    std::optional<T> find(const Key &k) {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return x->value;
        }
        return {};
    }

    std::optional<const T> find(const Key &k) const {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return x->value;
        }
        return {};
    }

    template<typename K>
    std::optional<T> find(const K &k) {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return x->value;
        }
        return {};
    }

    template<typename K>
    std::optional<const T> find(const K &k) const {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return x->value;
        }
        return {};
    }

    bool contains(const Key &k) const {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        return (x != nullptr && x->key == k);
    }

    template<typename K>
    bool contains(const K &k) const {
        std::shared_lock lock(mu);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        return (x != nullptr && x->key == k);
    }

    void erase(const Key &k) {
        std::unique_lock lock(mu);
        std::vector<Node *> update(m_maxLevel + 1, nullptr);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && k > x->forward[i]->key)
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[0];
        if (!x || x->key != k) return;
        for (int i = 0; i <= m_currLevel; ++i) {
            if (update[i]->forward[i] == x)
                update[i]->forward[i] = x->forward[i];
        }


        if (x->forward[0] == nullptr) {
            tail = update[0] == root ? nullptr : update[0];
        }
        NodeTraits::destroy(m_alloc, x);
        NodeTraits::deallocate(m_alloc, x, 1);
        --m_size;
        while (m_currLevel > 0 && !root->forward[m_currLevel])
            --m_currLevel;
    }

    //not good
    void print() const {
        std::shared_lock lock(mu);
        for (int lvl = m_currLevel; lvl >= 0; --lvl) {
            Node *curr = root->forward[lvl];
            std::cout << "Level " << lvl << ": ";
            while (curr) {
                std::cout << "[" << "K:" << curr->key << ":" << "V:" << curr->value << "] -> ";
                curr = curr->forward[lvl];
            }
            std::cout << "nullptr\n";
        }
    }
};


#endif // UNTITLED1_LIBRARY_H
