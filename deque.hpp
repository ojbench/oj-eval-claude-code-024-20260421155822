#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>
#include <iostream>

namespace sjtu {

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = 64;
    T** map;
    size_t map_size;
    size_t start_idx;
    size_t current_size;

    void expand_map() {
        size_t new_map_size = map_size * 2;
        if (new_map_size == 0) new_map_size = 8;
        T** new_map = new T*[new_map_size];
        for (size_t i = 0; i < new_map_size; ++i) new_map[i] = nullptr;

        size_t new_map_start = (new_map_size - (current_size / BLOCK_SIZE + 2)) / 2;
        if (map) {
            size_t old_start_block = start_idx / BLOCK_SIZE;
            size_t num_blocks = (start_idx + current_size + BLOCK_SIZE - 1) / BLOCK_SIZE - old_start_block;
            if (current_size == 0) num_blocks = 0;

            for (size_t i = 0; i < num_blocks; ++i) {
                new_map[new_map_start + i] = map[old_start_block + i];
            }
            delete[] map;
            start_idx = new_map_start * BLOCK_SIZE + (start_idx % BLOCK_SIZE);
        } else {
            start_idx = new_map_start * BLOCK_SIZE;
        }
        map = new_map;
        map_size = new_map_size;
    }

    void allocate_block(size_t block_idx) {
        if (block_idx >= map_size) {
            // This should not happen if map expansion is done correctly
            expand_map();
            block_idx = (start_idx / BLOCK_SIZE); // Update block_idx if needed? No, wait.
        }
        if (map[block_idx] == nullptr) {
            map[block_idx] = (T*)operator new(BLOCK_SIZE * sizeof(T));
        }
    }

public:
    class const_iterator;
    class iterator {
    private:
        deque* container;
        size_t index;

    public:
        iterator(deque* c = nullptr, size_t i = 0) : container(c), index(i) {}
        iterator(const iterator& other) : container(other.container), index(other.index) {}

        iterator operator+(const int &n) const {
            return iterator(container, index + n);
        }
        iterator operator-(const int &n) const {
            return iterator(container, index - n);
        }
        int operator-(const iterator &rhs) const {
            if (container != rhs.container) throw invalid_iterator();
            return (int)index - (int)rhs.index;
        }
        iterator operator+=(const int &n) {
            index += n;
            return *this;
        }
        iterator operator-=(const int &n) {
            index -= n;
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            index++;
            return old;
        }
        iterator& operator++() {
            index++;
            return *this;
        }
        iterator operator--(int) {
            iterator old = *this;
            index--;
            return old;
        }
        iterator& operator--() {
            index--;
            return *this;
        }
        T& operator*() const {
            if (!container || index < container->start_idx || index >= container->start_idx + container->current_size)
                throw index_out_of_bound();
            return (*container)[index - container->start_idx];
        }
        T* operator->() const noexcept {
            return &((*container)[index - container->start_idx]);
        }
        bool operator==(const iterator &rhs) const {
            return container == rhs.container && index == rhs.index;
        }
        bool operator==(const const_iterator &rhs) const {
            return container == rhs.container && index == rhs.index;
        }
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        size_t get_index() const { return index; }
        deque* get_container() const { return container; }
    };

    class const_iterator {
    private:
        const deque* container;
        size_t index;

    public:
        const_iterator(const deque* c = nullptr, size_t i = 0) : container(c), index(i) {}
        const_iterator(const iterator &other) : container(other.get_container()), index(other.get_index()) {}

        const_iterator operator+(const int &n) const {
            return const_iterator(container, index + n);
        }
        const_iterator operator-(const int &n) const {
            return const_iterator(container, index - n);
        }
        int operator-(const const_iterator &rhs) const {
            if (container != rhs.container) throw invalid_iterator();
            return (int)index - (int)rhs.index;
        }
        const_iterator operator+=(const int &n) {
            index += n;
            return *this;
        }
        const_iterator operator-=(const int &n) {
            index -= n;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            index++;
            return old;
        }
        const_iterator& operator++() {
            index++;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator old = *this;
            index--;
            return old;
        }
        const_iterator& operator--() {
            index--;
            return *this;
        }
        const T& operator*() const {
            if (!container || index < container->start_idx || index >= container->start_idx + container->current_size)
                throw index_out_of_bound();
            return (*container)[index - container->start_idx];
        }
        const T* operator->() const noexcept {
            return &((*container)[index - container->start_idx]);
        }
        bool operator==(const iterator &rhs) const {
            return container == rhs.get_container() && index == rhs.get_index();
        }
        bool operator==(const const_iterator &rhs) const {
            return container == rhs.container && index == rhs.index;
        }
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        size_t get_index() const { return index; }
        const deque* get_container() const { return container; }
    };

    deque() : map(nullptr), map_size(0), start_idx(0), current_size(0) {
        expand_map();
    }

    deque(const deque &other) : map(nullptr), map_size(other.map_size), start_idx(other.start_idx), current_size(other.current_size) {
        map = new T*[map_size];
        for (size_t i = 0; i < map_size; ++i) {
            if (other.map[i]) {
                map[i] = (T*)operator new(BLOCK_SIZE * sizeof(T));
                size_t b_start = i * BLOCK_SIZE;
                size_t b_end = (i + 1) * BLOCK_SIZE;
                for (size_t j = 0; j < BLOCK_SIZE; ++j) {
                    size_t global_idx = b_start + j;
                    if (global_idx >= start_idx && global_idx < start_idx + current_size) {
                        new (map[i] + j) T(other.map[i][j]);
                    }
                }
            } else {
                map[i] = nullptr;
            }
        }
    }

    ~deque() {
        clear();
        for (size_t i = 0; i < map_size; ++i) {
            if (map[i]) {
                operator delete(map[i]);
            }
        }
        delete[] map;
    }

    deque &operator=(const deque &other) {
        if (this == &other) return *this;
        clear();
        for (size_t i = 0; i < map_size; ++i) {
            if (map[i]) operator delete(map[i]);
        }
        delete[] map;

        map_size = other.map_size;
        start_idx = other.start_idx;
        current_size = other.current_size;
        map = new T*[map_size];
        for (size_t i = 0; i < map_size; ++i) {
            if (other.map[i]) {
                map[i] = (T*)operator new(BLOCK_SIZE * sizeof(T));
                size_t b_start = i * BLOCK_SIZE;
                for (size_t j = 0; j < BLOCK_SIZE; ++j) {
                    size_t global_idx = b_start + j;
                    if (global_idx >= start_idx && global_idx < start_idx + current_size) {
                        new (map[i] + j) T(other.map[i][j]);
                    }
                }
            } else {
                map[i] = nullptr;
            }
        }
        return *this;
    }

    T & at(const size_t &pos) {
        if (pos >= current_size) throw index_out_of_bound();
        return (*this)[pos];
    }

    const T & at(const size_t &pos) const {
        if (pos >= current_size) throw index_out_of_bound();
        return (*this)[pos];
    }

    T & operator[](const size_t &pos) {
        size_t global_idx = start_idx + pos;
        size_t b_idx = global_idx / BLOCK_SIZE;
        size_t b_off = global_idx % BLOCK_SIZE;
        return map[b_idx][b_off];
    }

    const T & operator[](const size_t &pos) const {
        size_t global_idx = start_idx + pos;
        size_t b_idx = global_idx / BLOCK_SIZE;
        size_t b_off = global_idx % BLOCK_SIZE;
        return map[b_idx][b_off];
    }

    const T & front() const {
        if (current_size == 0) throw container_is_empty();
        return (*this)[0];
    }

    const T & back() const {
        if (current_size == 0) throw container_is_empty();
        return (*this)[current_size - 1];
    }

    iterator begin() {
        return iterator(this, start_idx);
    }

    const_iterator cbegin() const {
        return const_iterator(this, start_idx);
    }

    iterator end() {
        return iterator(this, start_idx + current_size);
    }

    const_iterator cend() const {
        return const_iterator(this, start_idx + current_size);
    }

    bool empty() const {
        return current_size == 0;
    }

    size_t size() const {
        return current_size;
    }

    void clear() {
        for (size_t i = 0; i < current_size; ++i) {
            size_t global_idx = start_idx + i;
            size_t b_idx = global_idx / BLOCK_SIZE;
            size_t b_off = global_idx % BLOCK_SIZE;
            map[b_idx][b_off].~T();
        }
        current_size = 0;
        // Keep the map and blocks, just reset current_size and start_idx?
        // Standard clear() doesn't necessarily deallocate everything.
    }

    iterator insert(iterator pos, const T &value) {
        if (pos.get_container() != this) throw invalid_iterator();
        int offset = pos.get_index() - start_idx;
        if (offset < 0 || offset > (int)current_size) throw invalid_iterator();

        if (offset == 0) {
            push_front(value);
            return begin();
        } else if (offset == (int)current_size) {
            push_back(value);
            return end() - 1;
        }

        // Simpler implementation: push_back and then move elements
        push_back(value);
        for (int i = (int)current_size - 1; i > offset; --i) {
            T tmp = (*this)[i];
            (*this)[i] = (*this)[i-1];
            (*this)[i-1] = tmp;
        }
        return begin() + offset;
    }

    iterator erase(iterator pos) {
        if (pos.get_container() != this) throw invalid_iterator();
        int offset = pos.get_index() - (int)start_idx;
        if (offset < 0 || offset >= (int)current_size) throw invalid_iterator();

        for (size_t i = offset; i < current_size - 1; ++i) {
            (*this)[i] = (*this)[i + 1];
        }
        pop_back();
        return begin() + offset;
    }

    void push_back(const T &value) {
        size_t next_idx = start_idx + current_size;
        size_t b_idx = next_idx / BLOCK_SIZE;
        if (b_idx >= map_size) {
            expand_map();
            b_idx = (start_idx + current_size) / BLOCK_SIZE;
        }
        allocate_block(b_idx);
        new (map[b_idx] + (next_idx % BLOCK_SIZE)) T(value);
        current_size++;
    }

    void pop_back() {
        if (current_size == 0) throw container_is_empty();
        size_t last_idx = start_idx + current_size - 1;
        size_t b_idx = last_idx / BLOCK_SIZE;
        size_t b_off = last_idx % BLOCK_SIZE;
        map[b_idx][b_off].~T();
        current_size--;
    }

    void push_front(const T &value) {
        if (start_idx == 0) {
            expand_map();
        }
        start_idx--;
        size_t b_idx = start_idx / BLOCK_SIZE;
        allocate_block(b_idx);
        new (map[b_idx] + (start_idx % BLOCK_SIZE)) T(value);
        current_size++;
    }

    void pop_front() {
        if (current_size == 0) throw container_is_empty();
        size_t b_idx = start_idx / BLOCK_SIZE;
        size_t b_off = start_idx % BLOCK_SIZE;
        map[b_idx][b_off].~T();
        start_idx++;
        current_size--;
    }
};

}

#endif
