#include "AlteRope.h"
#include <stdexcept> // For std::out_of_range in at()
#include <iostream>  // For placeholder messages

// Constructors and Destructor
AlteRope::AlteRope() {
    // internal_buffer is already default-initialized (empty string)
    // std::cout << "AlteRope default constructor called." << std::endl; // For debugging
}

AlteRope::AlteRope(const std::string& initial_str) : internal_buffer(initial_str) {
    // std::cout << "AlteRope constructor with string called. Length: " << initial_str.length() << std::endl; // For debugging
}

AlteRope::~AlteRope() {
    // std::cout << "AlteRope destructor called." << std::endl; // For debugging
    // For internal_buffer, no explicit cleanup needed as it's an std::string.
    // When 'root' (tree structure) is implemented, cleanup will be needed here.
}

// Basic Rope operations
long long AlteRope::length() const {
    return internal_buffer.length();
}

std::string AlteRope::toString() const {
    return internal_buffer;
}

// Editing operations (initial simple implementation using std::string methods)
void AlteRope::insert(long long index, const std::string& str) {
    std::cout << "AlteRope::insert called. Index: " << index << ", Str: \"" << str << "\"" << std::endl;
    if (index < 0 || index > static_cast<long long>(internal_buffer.length())) {
        // Or throw std::out_of_range("Index out of bounds for insert");
        std::cerr << "Error: Insert index out of bounds." << std::endl;
        return;
    }
    internal_buffer.insert(static_cast<size_t>(index), str);
}

void AlteRope::remove(long long index, long long count) {
    std::cout << "AlteRope::remove called. Index: " << index << ", Count: " << count << std::endl;
    if (index < 0 || index >= static_cast<long long>(internal_buffer.length()) || count <= 0) {
        // Or throw std::out_of_range("Index out of bounds or invalid count for remove");
        std::cerr << "Error: Remove index out of bounds or invalid count." << std::endl;
        return;
    }
    // Prevent removing beyond the end of the string
    if (index + count > static_cast<long long>(internal_buffer.length())) {
        count = internal_buffer.length() - index;
    }
    internal_buffer.erase(static_cast<size_t>(index), static_cast<size_t>(count));
}

char AlteRope::at(long long index) const {
    std::cout << "AlteRope::at called. Index: " << index << std::endl;
    if (index < 0 || index >= static_cast<long long>(internal_buffer.length())) {
        throw std::out_of_range("Index out of bounds for at()");
    }
    return internal_buffer[static_cast<size_t>(index)];
}

// Future private helper implementations would go here.
// e.g., balance, concatenate, split etc.
