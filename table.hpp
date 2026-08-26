#pragma once

#include "common.hpp"
#include "value.hpp"

struct Entry {
    ObjString* key{nullptr};
    Value value{NIL_VAL()};
};

class Table {
public:
    int count{0};
    int capacity{0};
    Entry* entries{nullptr};

    Table() = default;
    ~Table();

    // Disable copy semantics to prevent shallow-copying raw pointer arrays
    Table(const Table&) = delete;
    Table& operator=(const Table&) = delete;

    // Move semantics for safe ownership transfer
    Table(Table&& other) noexcept;
    Table& operator=(Table&& other) noexcept;

    void init();
    void free();
    bool get(ObjString* key, Value* value) const;
    bool set(ObjString* key, Value value);
    bool remove(ObjString* key);
    void addAll(const Table& from);
    ObjString* findString(const char* chars, int length, uint32_t hash) const;

private:
    Entry* findEntry(Entry* entriesArray, int cap, ObjString* targetKey) const;
    void adjustCapacity(int newCapacity);
};
