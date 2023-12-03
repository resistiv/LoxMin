#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

static Entry* FindEntry(Entry* entries, int capacity, ObjectString* key);
static void AdjustCapacity(Table* table, int capacity);

void InitTable(Table* table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void FreeTable(Table* table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    InitTable(table);
}

bool TableGet(Table* table, ObjectString* key, Value* value)
{
    if (table->count == 0)
    {
        return false;
    }

    Entry* entry = FindEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
    {
        return false;
    }

    *value = entry->value;
    return true;
}

bool TableSet(Table* table, ObjectString* key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity= GROW_CAPACITY(table->capacity);
        AdjustCapacity(table, capacity);
    }

    Entry* entry = FindEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_NIL(entry->value))
    {
        table->count++;
    }

    entry->key = key;
    entry->value = value;

    return isNewKey;
}

bool TableDelete(Table* table, ObjectString* key)
{
    if (table->count == 0)
    {
        return false;
    }

    // Find the entry
    Entry* entry = FindEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
    {
        return false;
    }

    // Insert a tombstone in place of the old pair
    entry->key = NULL;
    entry->value = BOOL_VALUE(true);
    return true;
}

void TableCopy(Table* from, Table* to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL)
        {
            TableSet(to, entry->key, entry->value);
        }
    }
}

/**
 * @brief Attempts to find an Entry in an array of Entries.
 * 
 * @param entries An array of Entries to search.
 * @param capacity The capacity of the array.
 * @param key A key to search with.
 * @return Entry* A pointer to the resulting Entry.
 */
static Entry* FindEntry(Entry* entries, int capacity, ObjectString* key)
{
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;
    
    while (1)
    {
        Entry* entry = &entries[index];
        if (entry->key == NULL)
        {
            // Empty entry
            if (IS_NIL(entry->value))
            {
                return tombstone != NULL ? tombstone : entry;
            }
            // Tombstone
            else
            {
                if (tombstone == NULL)
                {
                    tombstone = entry;
                }
            }
        }
        // We found our key!
        else if (entry->key == key)
        {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

/**
 * @brief Adjusts the capacity of a hash table.
 * 
 * @param table A Table to adjust.
 * @param capacity A new capacity to adjust to.
 */
static void AdjustCapacity(Table* table, int capacity)
{
    Entry* entries = ALLOCATE(Entry, capacity);

    // Clear newly allocated table
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VALUE;
    }

    // Re-insert old entries
    table->count = 0;
    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        // No key, no need to recalculate
        if (entry->key == NULL)
        {
            continue;
        }

        Entry* dest = FindEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // Free old table
    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

ObjectString* TableFindString(Table* table, const char* chars, int length, uint32_t hash)
{
    // Empty table
    if (table->count == 0)
    {
        return NULL;
    }

    uint32_t index = hash % table->capacity;
    while (1)
    {
        Entry* entry = &table->entries[index];
        // Is this entry empty & non-tombstone?
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value))
            {
                return NULL;
            }
        }
        // Is this the string we're looking for?
        else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0)
        {
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

void MarkTable(Table* table)
{
    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        MarkObject((Object*)entry->key);
        MarkValue(entry->value);
    }
}

void TableRemoveWhite(Table* table)
{
    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked)
        {
            TableDelete(table, entry->key);
        }
    }
}
