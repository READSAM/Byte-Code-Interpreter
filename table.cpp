#include <cstdlib>
#include <cstring>

#include "memory.hpp"
#include "object.hpp"
#include "table.hpp"
#include "value.hpp"

#define TABLE_MAX_LOAD 0.75

Table::~Table() {
    free();
}

Table::Table(Table&& other) noexcept
    : count(other.count), capacity(other.capacity), entries(other.entries) {
    other.count = 0;
    other.capacity = 0;
    other.entries = nullptr;
}

Table& Table::operator=(Table&& other) noexcept {
    if (this != &other) {
        free();
        count = other.count;
        capacity = other.capacity;
        entries = other.entries;
        other.count = 0;
        other.capacity = 0;
        other.entries = nullptr;
    }
    return *this;
}

void Table::init() {
    count = 0;
    capacity = 0;
    entries = nullptr;
}

void Table::free() {
    freeArray(entries, capacity);
    init();
}

Entry* Table::findEntry(Entry* entriesArray, int cap, ObjString* targetKey) const {
    uint32_t index = targetKey->hash % cap;
    Entry* tombstone = nullptr;
    for (;;) {
        Entry* entry = &entriesArray[index];
        if (entry->key == nullptr) {
            if (IS_NIL(entry->value)) {
                return tombstone != nullptr ? tombstone : entry;
            } else {
                if (tombstone == nullptr) tombstone = entry;
            }
        } else if (entry->key == targetKey) {
            return entry;
        }
        
        index = (index + 1) % cap;
    }
}

bool Table::get(ObjString* targetKey, Value* value) const {
    if (count == 0) return false;
    Entry* entry = findEntry(entries, capacity, targetKey);
    if (entry->key == nullptr) return false;

    *value = entry->value;
    return true;
}

void Table::adjustCapacity(int newCapacity) {
    Entry* newEntries = allocate<Entry>(newCapacity);
    for (int i = 0; i < newCapacity; i++) {
        newEntries[i].key = nullptr;
        newEntries[i].value = NIL_VAL();
    }
    
    count = 0;
    for (int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if (entry->key == nullptr) continue;

        Entry* dest = findEntry(newEntries, newCapacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        count++;
    }
    
    freeArray(entries, capacity);
    entries = newEntries;
    capacity = newCapacity;
}

bool Table::set(ObjString* targetKey, Value value) {
    if (count + 1 > capacity * TABLE_MAX_LOAD) {
        int newCapacity = growCapacity(capacity);
        adjustCapacity(newCapacity);
    }

    Entry* entry = findEntry(entries, capacity, targetKey);
    bool isNewKey = (entry->key == nullptr);
    if (isNewKey && IS_NIL(entry->value)) count++;

    entry->key = targetKey;
    entry->value = value;
    return isNewKey;
}

bool Table::remove(ObjString* targetKey) {
    if (count == 0) return false;

    Entry* entry = findEntry(entries, capacity, targetKey);
    if (entry->key == nullptr) return false;

    entry->key = nullptr;
    entry->value = BOOL_VAL(true); // tombstone of the deleted entry
    return true;
}

void Table::addAll(const Table& from) {
    for (int i = 0; i < from.capacity; i++) {
        Entry* entry = &from.entries[i];
        if (entry->key != nullptr) {
            set(entry->key, entry->value);
        }
    }
}

ObjString* Table::findString(const char* chars, int length, uint32_t hash) const {
    if (count == 0) return nullptr;

    uint32_t index = hash % capacity;
    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == nullptr) {
            if (IS_NIL(entry->value)) return nullptr;
        } else if (entry->key->length == length && entry->key->hash == hash && 
                   std::memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) % capacity;
    }
}