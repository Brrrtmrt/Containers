

#ifndef UNTITLED1_ITERATOR_H
#define UNTITLED1_ITERATOR_H
#include <cstddef>
#include <iterator>
#include "SkipNode.h"

template<bool IsConst, typename Key, typename T>
class Iterator {
    using Node = SkipNode<Key, T>;
    using NodePtr = std::conditional_t<IsConst, const Node *, Node *>;

    NodePtr node;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T *, T *>;
    using reference = std::conditional_t<IsConst, const T &, T &>;

    explicit Iterator(NodePtr n = nullptr) : node(n) {
    }

    reference operator*() const { return node->value; }
    pointer operator->() const { return &node->value; }

    Iterator &operator++() {
        node = node->forward[0];
        return *this;
    }

    Iterator operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const Iterator &a, const Iterator &b) {
        return a.node == b.node;
    }

    friend bool operator!=(const Iterator &a, const Iterator &b) {
        return a.node != b.node;
    }
};

#endif //UNTITLED1_ITERATOR_H
