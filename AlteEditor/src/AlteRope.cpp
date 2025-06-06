#include "AlteRope.h"
#include <stdexcept> // For std::out_of_range
#include <iostream>  // For debugging (can be removed later)
#include <vector>    // Potentially for splitting string in build_rope
#include <algorithm> // For std::min

// --- UTF-8 Helper Functions ---

static size_t count_utf8_chars(const std::string& s) {
    size_t count = 0;
    size_t i = 0;
    while (i < s.length()) {
        unsigned char c = s[i];
        if (c <= 0x7F) { i += 1;
        } else if ((c & 0xE0) == 0xC0) { if (i + 1 < s.length() && (s[i+1] & 0xC0) == 0x80) i += 2; else { i++; /* invalid seq */ }
        } else if ((c & 0xF0) == 0xE0) { if (i + 2 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80) i += 3; else { i++; /* invalid seq */ }
        } else if ((c & 0xF8) == 0xF0) { if (i + 3 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80 && (s[i+3] & 0xC0) == 0x80) i += 4; else { i++; /* invalid seq */ }
        } else { i++; /* invalid start byte */ }
        count++;
    }
    return count;
}

static std::string get_nth_utf8_char(const std::string& s, size_t n) {
    size_t current_char_index = 0;
    size_t i = 0;
    while (i < s.length() && current_char_index <= n) {
        size_t char_start_byte_index = i;
        unsigned char c = s[i];
        size_t char_byte_len = 0;
        if (c <= 0x7F) { char_byte_len = 1;
        } else if ((c & 0xE0) == 0xC0) { if (i + 1 < s.length() && (s[i+1] & 0xC0) == 0x80) char_byte_len = 2; else { i++; continue; }
        } else if ((c & 0xF0) == 0xE0) { if (i + 2 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80) char_byte_len = 3; else { i++; continue; }
        } else if ((c & 0xF8) == 0xF0) { if (i + 3 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80 && (s[i+3] & 0xC0) == 0x80) char_byte_len = 4; else { i++; continue; }
        } else { i++; continue; }
        if (current_char_index == n) { return s.substr(char_start_byte_index, char_byte_len); }
        i += char_byte_len;
        current_char_index++;
    }
    return "";
}

static size_t get_byte_offset_for_char_index(const std::string& s, size_t target_char_idx) {
    size_t current_char_count = 0;
    size_t byte_idx = 0;
    while (byte_idx < s.length() && current_char_count < target_char_idx) {
        unsigned char c = s[byte_idx];
        if (c <= 0x7F) { byte_idx += 1;
        } else if ((c & 0xE0) == 0xC0) { if (byte_idx + 1 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80) byte_idx += 2; else { byte_idx++; }
        } else if ((c & 0xF0) == 0xE0) { if (byte_idx + 2 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80 && (s[byte_idx+2] & 0xC0) == 0x80) byte_idx += 3; else { byte_idx++; }
        } else if ((c & 0xF8) == 0xF0) { if (byte_idx + 3 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80 && (s[byte_idx+2] & 0xC0) == 0x80 && (s[byte_idx+3] & 0xC0) == 0x80) byte_idx += 4; else { byte_idx++; }
        } else { byte_idx++; }
        current_char_count++;
    }
    if (current_char_count == target_char_idx) return byte_idx;
    if (target_char_idx > current_char_count) return s.length();
    return byte_idx;
}

// Helper to get byte length of a specific number of UTF-8 characters from a character offset
static size_t get_byte_length_for_char_count(const std::string& s, size_t char_offset_start, size_t num_chars_to_count) {
    size_t current_char_count_in_str = 0;
    size_t byte_idx = 0;
    size_t byte_len_of_target_chars = 0;
    size_t chars_counted_for_target_segment = 0;

    while (byte_idx < s.length() && chars_counted_for_target_segment < num_chars_to_count) {
        unsigned char c = s[byte_idx];
        size_t current_char_byte_len = 0;

        if (c <= 0x7F) { current_char_byte_len = 1;
        } else if ((c & 0xE0) == 0xC0) { if (byte_idx + 1 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80) current_char_byte_len = 2; else { byte_idx++; continue; }
        } else if ((c & 0xF0) == 0xE0) { if (byte_idx + 2 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80 && (s[byte_idx+2] & 0xC0) == 0x80) current_char_byte_len = 3; else { byte_idx++; continue; }
        } else if ((c & 0xF8) == 0xF0) { if (byte_idx + 3 < s.length() && (s[byte_idx+1] & 0xC0) == 0x80 && (s[byte_idx+2] & 0xC0) == 0x80 && (s[byte_idx+3] & 0xC0) == 0x80) current_char_byte_len = 4; else { byte_idx++; continue; }
        } else { byte_idx++; continue; }

        if (current_char_count_in_str >= char_offset_start) {
            byte_len_of_target_chars += current_char_byte_len;
            chars_counted_for_target_segment++;
        }

        byte_idx += current_char_byte_len;
        current_char_count_in_str++;
    }
    return byte_len_of_target_chars;
}


// --- RopeNode Member function ---
void RopeNode::set_leaf_weight() {
    if (is_leaf()) {
        this->weight = count_utf8_chars(this->data);
    }
}

// --- AlteRope Class Implementation ---

const size_t MAX_LEAF_LEN_BYTES = 64;
const size_t MAX_LEAF_LEN_CHARS_SPLIT_THRESHOLD = MAX_LEAF_LEN_BYTES * 2;

AlteRope::AlteRope() : root(nullptr) {}

AlteRope::AlteRope(const std::string& str) : root(nullptr) {
    if (!str.empty()) {
        root = build_rope(str, 0, str.length());
    }
}

AlteRope::~AlteRope() {
    delete_nodes(root);
    root = nullptr;
}

void AlteRope::delete_nodes(RopeNode* node) {
    if (node == nullptr) return;
    delete_nodes(node->left);
    delete_nodes(node->right);
    delete node;
}

RopeNode* AlteRope::build_rope(const std::string& str, size_t start_byte, size_t end_byte) {
    size_t byte_length = end_byte - start_byte;
    if (byte_length == 0) return nullptr;

    if (byte_length <= MAX_LEAF_LEN_BYTES) {
        RopeNode* leaf = new RopeNode(str.substr(start_byte, byte_length));
        leaf->weight = count_utf8_chars(leaf->data);
        return leaf;
    }

    size_t mid_byte = start_byte + byte_length / 2;

    RopeNode* left_child = build_rope(str, start_byte, mid_byte);
    RopeNode* right_child = build_rope(str, mid_byte, end_byte);

    RopeNode* internal_node = new RopeNode(left_child, right_child);
    internal_node->weight = calculate_length(left_child);
    return internal_node;
}

size_t AlteRope::length() const {
    if (root == nullptr) return 0;
    return calculate_length(root);
}

size_t AlteRope::calculate_length(RopeNode* node) const {
    if (node == nullptr) return 0;
    if (node->is_leaf()) return node->weight;
    return calculate_length(node->left) + calculate_length(node->right);
}

std::string AlteRope::toString() const {
    std::string result_str;
    if (root == nullptr) return "";
    build_string(root, result_str);
    return result_str;
}

void AlteRope::build_string(RopeNode* node, std::string& out_str) const {
    if (node == nullptr) return;
    if (node->is_leaf()) {
        out_str += node->data;
    } else {
        build_string(node->left, out_str);
        build_string(node->right, out_str);
    }
}

std::string AlteRope::character_at(size_t char_index) const {
    size_t total_len = length();
    if (char_index >= total_len) {
        throw std::out_of_range("Character index out of range in character_at.");
    }
    std::string result_char_str;
    size_t current_char_idx_ref = char_index;
    find_char_at(root, current_char_idx_ref, result_char_str);
    return result_char_str;
}

void AlteRope::find_char_at(RopeNode* node, size_t& char_index_ref, std::string& result) const {
    if (node == nullptr || !result.empty()) return;

    if (node->is_leaf()) {
        if (char_index_ref < node->weight) {
            result = get_nth_utf8_char(node->data, char_index_ref);
        }
    } else {
        if (char_index_ref < node->weight) { // node->weight is char length of left subtree
            find_char_at(node->left, char_index_ref, result);
        } else {
            char_index_ref -= node->weight;
            find_char_at(node->right, char_index_ref, result);
        }
    }
}

void AlteRope::insert(size_t char_index, const std::string& text) {
    if (text.empty()) return;
    size_t current_len = length();
    if (char_index > current_len) {
        throw std::out_of_range("Character index out of range in insert.");
    }
    if (root == nullptr) {
        if (char_index == 0) { root = build_rope(text, 0, text.length()); }
        else { throw std::logic_error("Cannot insert at non-zero index into an empty rope."); }
        return;
    }
    size_t mutable_char_index = char_index;
    root = insert_recursive(root, mutable_char_index, text);
}

RopeNode* AlteRope::insert_recursive(RopeNode* node, size_t& char_index_ref, const std::string& text) {
    if (node == nullptr) {
        if (char_index_ref == 0) { return build_rope(text, 0, text.length()); }
        return nullptr;
    }
    if (node->is_leaf()) {
        if (char_index_ref <= node->weight) {
            size_t byte_offset = get_byte_offset_for_char_index(node->data, char_index_ref);
            std::string new_data = node->data;
            new_data.insert(byte_offset, text);
            size_t new_char_len = count_utf8_chars(new_data);
            if (new_char_len <= MAX_LEAF_LEN_CHARS_SPLIT_THRESHOLD) {
                node->data = new_data;
                node->weight = new_char_len;
                return node;
            } else {
                RopeNode* new_subtree_root = build_rope(new_data, 0, new_data.length());
                delete node;
                return new_subtree_root;
            }
        } else {
            char_index_ref -= node->weight;
            // This implies insertion is after this leaf. The current design expects
            // char_index_ref to guide to the correct leaf or split point *within* a leaf.
            // If it's still > 0 here, it means we're effectively trying to append *after* this leaf,
            // which should be handled by creating a new node to the right in the parent.
            // For now, returning node and letting parent handle might be one way, but it's tricky.
            // This indicates a need for a more robust way to return "insertion happened" AND "where next".
            // A simple solution: if index is past this leaf, it's an error for this recursive step.
            // The public method should ensure overall index is valid.
            // This path should not be hit if logic is perfect.
             throw std::logic_error("Insert_recursive: char_index out of bounds for leaf node processing.");
        }
    } else {
        if (char_index_ref <= node->weight) {
            node->left = insert_recursive(node->left, char_index_ref, text);
        } else {
            char_index_ref -= node->weight;
            node->right = insert_recursive(node->right, char_index_ref, text);
        }
        node->weight = calculate_length(node->left);
    }
    return node;
}

void AlteRope::remove(size_t char_index, size_t char_count) {
    if (char_count == 0) return;
    size_t current_len = length();
    if (char_index >= current_len || char_index + char_count > current_len) {
        // Allow char_index + char_count == current_len for deleting to the end
        if (char_index + char_count > current_len && !(char_index + char_count == current_len && char_count > 0) ) {
             if(char_index == current_len && char_count == 0) { /* allow */ }
             else if (char_index + char_count > current_len) {
                throw std::out_of_range("Deletion range out of bounds.");
             }
        }
        if (char_index >= current_len && char_count > 0) { // Cannot start deleting past the end
            throw std::out_of_range("Deletion start index out of bounds.");
        }
    }


    if (root == nullptr) return;

    size_t mutable_char_count = char_count;
    root = delete_recursive(root, char_index, mutable_char_count);
    // TODO: Add rebalancing or node merging/concatenation if necessary
}

RopeNode* AlteRope::delete_recursive(RopeNode* node, size_t char_idx_in_subtree, size_t& count_ref) {
    if (node == nullptr || count_ref == 0) {
        return node;
    }

    if (node->is_leaf()) {
        size_t leaf_char_len = node->weight;
        if (char_idx_in_subtree < leaf_char_len) {
            size_t chars_to_delete_here = std::min(count_ref, leaf_char_len - char_idx_in_subtree);

            size_t byte_offset_start = get_byte_offset_for_char_index(node->data, char_idx_in_subtree);
            size_t byte_len_to_delete = get_byte_length_for_char_count(node->data, char_idx_in_subtree, chars_to_delete_here);

            node->data.erase(byte_offset_start, byte_len_to_delete);
            node->weight = count_utf8_chars(node->data);
            count_ref -= chars_to_delete_here;
        }
        // If char_idx_in_subtree >= leaf_char_len, deletion starts past this leaf, so do nothing to this leaf.
        // count_ref remains unchanged, char_idx_in_subtree would be adjusted by caller (internal node).

        if (node->data.empty()) {
            delete node;
            return nullptr;
        }
        return node;
    }

    // Internal node
    size_t left_len = calculate_length(node->left); // node->weight is already this

    if (char_idx_in_subtree < left_len) { // Deletion starts in or spans into the left child
        node->left = delete_recursive(node->left, char_idx_in_subtree, count_ref);
        // If deletion spanned into the right child (count_ref > 0 after left recursion)
        if (count_ref > 0) {
            // All remaining chars to delete must be from the start of the right child now
            node->right = delete_recursive(node->right, 0, count_ref);
        }
    } else { // Deletion starts purely in the right child
        node->right = delete_recursive(node->right, char_idx_in_subtree - left_len, count_ref);
    }

    // Update weight and handle pruning
    if (node->left == nullptr && node->right == nullptr) { // Both children gone
        delete node;
        return nullptr;
    } else if (node->left == nullptr) { // Only right child remains
        RopeNode* temp = node->right;
        delete node;
        return temp;
    } else if (node->right == nullptr) { // Only left child remains
        RopeNode* temp = node->left;
        delete node;
        return temp;
    } else { // Both children might still exist or one became null and was handled
        node->weight = calculate_length(node->left);
        return node;
    }
}
