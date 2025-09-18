#ifndef UNTITLED1_LIBRARY_H
#define UNTITLED1_LIBRARY_H
#include <algorithm>
#include <memory>
#include <initializer_list>
#include <ostream>
#include <random>
#include "Iterator.h"


template<typename Key, typename T, typename Allocator=std::allocator<T> >
class SkipList {
    using Node = SkipNode<Key, T>;
    Node *root;
    Node *tail;
    int m_currLevel;
    int m_maxLevel;
    size_t m_size;

    [[nodiscard]] int randomLevel() const {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::geometric_distribution<> dst(0.5);
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

    using iterator = Iterator<false, Key, T>;
    using const_iterator = Iterator<true, Key, T>;

    iterator begin() noexcept { return iterator(root->forward[0]); }
    iterator end() noexcept { return iterator(nullptr); }

    [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(root->forward[0]); }
    [[nodiscard]] const_iterator end() const noexcept { return const_iterator(nullptr); }

    [[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator(root->forward[0]); }
    [[nodiscard]] const_iterator cend() const noexcept { return const_iterator(nullptr); }


    explicit SkipList()
        : m_maxLevel{16}, m_currLevel{0}, m_size{0} {
        root = NodeTraits::allocate(m_alloc, 1);
        NodeTraits::construct(m_alloc, root, Key{}, T{}, 16);
        tail = nullptr;
        std::cout << m_maxLevel << " ";
    }

    SkipList(std::initializer_list<std::pair<Key, T> > init)
        : SkipList() {
        for (auto &&p: init) {
            insert(p.first, p.second);
        }
    }

    ~SkipList() {
        clear();
        NodeTraits::destroy(m_alloc, root);
        NodeTraits::deallocate(m_alloc, root, 1);
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return m_size;
    }

    void clear() noexcept {
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

    T &front() {
        return *begin();
    }

    T &front() const {
        return *begin();
    }

    T &back() {
        return tail->value;
    }


    T &back() const {
        return tail->value;
    }

    [[nodiscard]] size_t maxlevel() const noexcept {
        return m_maxLevel;
    }


    template<typename U>
    iterator insert(const Key &k, U &&v) {
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
            return iterator(x);
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

        return iterator(tmp);
    }

    iterator find(const Key &k) {
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return iterator(x);
        }
        return iterator(nullptr);
    }

    const_iterator find(const Key &k) const {
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return const_iterator(x);
        }
        return const_iterator(nullptr);
    }

    template<typename K>
    iterator find(const K &k) {
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return iterator(x);
        }
        return iterator(nullptr);
    }

    template<typename K>
    const_iterator find(const K &k) const {
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && (k > x->forward[i]->key)) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x != nullptr && x->key == k) {
            return const_iterator(x);
        }
        return const_iterator(nullptr);
    }

    bool contains(const Key &k) const {
        return find(k) != end();
    }

    template<typename K>
    bool contains(const K &k) const {
        return find(k) != end();
    }

    iterator erase(const Key &k) {
        std::vector<Node *> update(m_maxLevel + 1, nullptr);
        Node *x = root;
        for (int i = m_currLevel; i >= 0; i--) {
            while (x->forward[i] != nullptr && k > x->forward[i]->key)
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[0];
        if (!x || x->key != k) return end();
        for (int i = 0; i <= m_currLevel; ++i) {
            if (update[i]->forward[i] == x)
                update[i]->forward[i] = x->forward[i];
        }


        if (x->forward[0] == nullptr) {
            tail = update[0] == root ? nullptr : update[0];
        }
        Node *next = x->forward[0];
        NodeTraits::destroy(m_alloc, x);
        NodeTraits::deallocate(m_alloc, x, 1);
        --m_size;
        while (m_currLevel > 0 && !root->forward[m_currLevel])
            --m_currLevel;

        return next == nullptr ? end() : iterator(next);
    }

    //not good
    void print() const {
        for (int lvl = m_currLevel; lvl >= 0; --lvl) {
            Node *curr = root->forward[lvl];
            std::cout << "Level " << lvl << ": ";
            while (curr) {
                std::cout << "[" << curr->key << ":] -> ";
                curr = curr->forward[lvl];
            }
            std::cout << "nullptr\n";
        }
    }
};


#endif // UNTITLED1_LIBRARY_H
