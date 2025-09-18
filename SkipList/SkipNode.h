#ifndef UNTITLED1_SKIPNODE_H
#define UNTITLED1_SKIPNODE_H

template<typename Key, typename T>
struct SkipNode {
    Key key;
    T value;
    std::vector<SkipNode *> forward{};
    int level;

    SkipNode(const Key &k, const T &v, int lvl)
        : key{k}, value{v}, level{lvl} {
        forward.resize(lvl + 1, nullptr);
    }

    SkipNode(Key &&k, T &&v, int lvl)
        : key{std::move(k)}, value{std::move(v)}, level{lvl} {
        forward.resize(lvl + 1, nullptr);
    }

    ~SkipNode() = default;
};
#endif //UNTITLED1_SKIPNODE_H
