#ifndef ALTEROPE_H
#define ALTEROPE_H

#include <string>
#include <vector>

struct RopeNode {
    std::string data;
    RopeNode* left = nullptr;
    RopeNode* right = nullptr;
    size_t weight;

    explicit RopeNode(std::string d) : data(std::move(d)), weight(0), left(nullptr), right(nullptr) {
    }

    RopeNode(RopeNode* l, RopeNode* r) : left(l), right(r), weight(0), data("") {
        if (left) {
        }
    }

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
    void set_leaf_weight();
};

class AlteRope {
public:
    AlteRope();
    explicit AlteRope(const std::string& initial_str);
    ~AlteRope();

    size_t length() const;
    std::string toString() const;

    void insert(size_t char_index, const std::string& text);
    void remove(size_t char_index, size_t char_count);
    std::string character_at(size_t char_index) const;

private:
    RopeNode* root = nullptr;

    size_t calculate_length(RopeNode* node) const;
    void delete_nodes(RopeNode* node);
    RopeNode* build_rope(const std::string& str, size_t start, size_t end);
    void build_string(RopeNode* node, std::string& out) const;
    void find_char_at(RopeNode* node, size_t& char_index, std::string& result) const;
    RopeNode* insert_recursive(RopeNode* node, size_t& char_index, const std::string& text);
    RopeNode* delete_recursive(RopeNode* node, size_t char_index_in_subtree, size_t& chars_to_delete_count);

};

#endif // ALTEROPE_H
