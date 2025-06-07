#include "AlteRope.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

static size_t count_utf8_chars(const std::string& s) {
    size_t count = 0;
    size_t i = 0;
    while (i < s.length()) {
        unsigned char c = s[i];
        if (c <= 0x7F) { i += 1;
        } else if ((c & 0xE0) == 0xC0) { if (i + 1 < s.length() && (s[i+1] & 0xC0) == 0x80) i += 2; else { i++; }
        } else if ((c & 0xF0) == 0xE0) { if (i + 2 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80) i += 3; else { i++; }
        } else if ((c & 0xF8) == 0xF0) { if (i + 3 < s.length() && (s[i+1] & 0xC0) == 0x80 && (s[i+2] & 0xC0) == 0x80 && (s[i+3] & 0xC0) == 0x80) i += 4; else { i++; }
        } else { i++; }
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

void RopeNode::set_leaf_weight() {
    if (is_leaf()) {
        this->weight = count_utf8_chars(this->data);
    }
}

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
        if (char_index_ref < node->weight) {
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
             throw std::logic_error("Insert_recursive: char_index out of bounds for leaf node processing.");
        }
    } else { // Internal node
        if (char_index_ref <= node->weight) {
            node->left = insert_recursive(node->left, char_index_ref, text);
            // Update weight only if left child was affected
            node->weight = calculate_length(node->left);
        } else {
            char_index_ref -= node->weight;
            node->right = insert_recursive(node->right, char_index_ref, text);
            // No change to node->weight (length of left subtree) if right subtree is modified
        }
    }
    return node;
}

void AlteRope::remove(size_t char_index, size_t char_count) {
    if (char_count == 0) {
        return; // Nothing to remove
    }

    size_t current_len = length();

    // Case 1: Start index is beyond or at the end of the rope.
    if (char_index >= current_len) {
        // If char_index == current_len, it means trying to delete AT the end (e.g. index 5 for length 5)
        // which is only valid if char_count is also 0, but that's handled above.
        // So, if char_count > 0, and char_index == current_len, it's an error.
        // If char_index > current_len, it's definitely an error.
        throw std::out_of_range("Deletion start index is out of bounds.");
    }

    // Case 2: Deletion range extends beyond the end of the rope.
    // (char_index < current_len is established from above)
    if (char_index + char_count > current_len) {
        throw std::out_of_range("Deletion range (index + count) exceeds rope length.");
    }

    // If we reach here, the range is valid (char_index < current_len and char_index + char_count <= current_len).

    // The original code had a check for root == nullptr here.
    // If current_len > 0 (implied if char_index < current_len for a valid range),
    // then root cannot be nullptr.
    // If current_len == 0, then char_index >= current_len would have already thrown.
    // So, an explicit check for root == nullptr might be redundant if length() is accurate
    // and the range checks are correct. However, keeping it as a safeguard is fine if desired.
    // For now, let's assume length() is correct and the above checks are sufficient before delete_recursive.
    // The original code had: if (root == nullptr) return;
    // This implies that if the rope is empty (current_len == 0), and somehow the previous checks passed
    // (which they shouldn't if char_count > 0), then we just return.
    // Given the checks, if current_len is 0, an exception is thrown if char_index >= 0 and char_count > 0.
    // If char_index is 0, current_len is 0, char_count is 0, we return at the top.
    // So the `if (root == nullptr) return;` seems truly redundant now.

    size_t count_to_delete = char_count; // Use a different variable name for clarity
    root = delete_recursive(root, char_index, count_to_delete);
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

        if (node->data.empty()) {
            delete node;
            return nullptr;
        }
        return node;
    }

    size_t left_len = calculate_length(node->left);

    if (char_idx_in_subtree < left_len) {
        node->left = delete_recursive(node->left, char_idx_in_subtree, count_ref);
        // After left child is processed, its length might have changed. So, node's weight needs update.
        node->weight = calculate_length(node->left);
        if (count_ref > 0) { // If deletion continues to right child
            node->right = delete_recursive(node->right, 0, count_ref);
        }
    } else { // Deletion is only in the right subtree
        node->right = delete_recursive(node->right, char_idx_in_subtree - left_len, count_ref);
        // node->weight (length of left subtree) does not change
    }

    if (node->left == nullptr && node->right == nullptr) {
        delete node;
        return nullptr;
    } else if (node->left == nullptr) {
        RopeNode* temp = node->right;
        delete node;
        return temp;
    } else if (node->right == nullptr) {
        RopeNode* temp = node->left;
        delete node;
        return temp;
    } else {
        node->weight = calculate_length(node->left);
        return node;
    }
}
