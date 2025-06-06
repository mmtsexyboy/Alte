#ifndef ALTEROPE_H
#define ALTEROPE_H

#include <string>
#include <vector> // Will be needed for more advanced node structures later

// Define RopeNode struct
struct RopeNode {
    std::string data;    // For leaf nodes
    RopeNode* left = nullptr;
    RopeNode* right = nullptr;
    size_t weight;       // Number of characters in the left subtree OR in data if leaf
    // size_t depth;     // Optional for balancing, can be added later

    // Constructor for leaf node
    explicit RopeNode(std::string d) : data(std::move(d)), weight(0), left(nullptr), right(nullptr) {
        // weight will be set properly by a char counting function
    }

    // Constructor for internal node
    RopeNode(RopeNode* l, RopeNode* r) : left(l), right(r), weight(0), data("") {
        if (left) {
            // weight will be set by calculate_length on left subtree or similar
        }
    }

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
    void set_leaf_weight(); // Declaration for the helper
};

class AlteRope {
public:
    // Constructors and Destructor
    AlteRope();
    explicit AlteRope(const std::string& initial_str);
    ~AlteRope();

    // Basic Rope operations
    size_t length() const; // Changed return type to size_t
    std::string toString() const; // Mainly for debugging

    // Editing operations (to be re-implemented)
    void insert(size_t char_index, const std::string& text);
    void remove(size_t char_index, size_t char_count);
    std::string character_at(size_t char_index) const; // Get character at UTF-8 index

private:
    RopeNode* root = nullptr; // Replaced internal_buffer

    // Private helper functions
    size_t calculate_length(RopeNode* node) const;
    void delete_nodes(RopeNode* node);
    RopeNode* build_rope(const std::string& str, size_t start, size_t end); // Helper for constructor
    void build_string(RopeNode* node, std::string& out) const; // For toString
    void find_char_at(RopeNode* node, size_t& char_index, std::string& result) const; // char_index is ref
    RopeNode* insert_recursive(RopeNode* node, size_t& char_index, const std::string& text); // char_index is ref
    RopeNode* delete_recursive(RopeNode* node, size_t char_index_in_subtree, size_t& chars_to_delete_count); // count is ref


    // Helper functions for future tree implementation
    // void balance(RopeNode*& node);
    // RopeNode* concatenate(RopeNode* n1, RopeNode* n2);
    // void split(RopeNode* original_node, size_t index, RopeNode** left_part, RopeNode** right_part);
    // static size_t count_utf8_chars(const std::string& s); // Declaration for UTF-8 helper
};

#endif // ALTEROPE_H
