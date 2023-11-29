#ifndef loxmin_table_h
#define loxmin_table_h

#include "common.h"
#include "value.h"

/**
 * @brief Represents a hash table entry.
 */
typedef struct
{
    ObjectString* key;
    Value value;
} Entry;

/**
 * @brief Represents a hash table.
 */
typedef struct
{
    int count;
    int capacity;
    Entry* entries;
} Table;

/**
 * @brief Initializes a hash table.
 * 
 * @param table A Table to initialize.
 */
void InitTable(Table* table);

/**
 * @brief Frees a hash table.
 * 
 * @param table A Table to free.
 */
void FreeTable(Table* table);

/**
 * @brief Attempts to get a Value from a hash table.
 * 
 * @param table A Table to get from.
 * @param key The key of the desired Value.
 * @param value A resulting Value.
 * @return true If the provided key was found.
 * @return false If the provided key was not found.
 */
bool TableGet(Table* table, ObjectString* key, Value* value);

/**
 * @brief Adds a key-value pair to a hash table.
 * 
 * @param table A Table to add to.
 * @param key A string key.
 * @param value A Value value.
 * @return true If the provided pair added is an new entry.
 * @return false If the provided pair added is not new.
 */
bool TableSet(Table* table, ObjectString* key, Value value);

/**
 * @brief Deletes a key-value pair from a hash table.
 * 
 * @param table A Table to delete from.
 * @param key The key of the desired Value.
 * @return true If the provided key was found and deleted.
 * @return false If the provided key was not found.
 */
bool TableDelete(Table* table, ObjectString* key);

/**
 * @brief Copies all Entries of one hash table into another.
 * 
 * @param from A source Table to copy from.
 * @param to A destination Table to copy to.
 */
void TableCopy(Table* from, Table* to);

/**
 * @brief Attempts to find a string value in a hash table.
 * 
 * @param table A Table to search.
 * @param chars A string to search for.
 * @param length The length of the string.
 * @param hash The hash of the string.
 * @return ObjectString* If found, a pointer to a ObjectString representing the string.
 */
ObjectString* TableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif
