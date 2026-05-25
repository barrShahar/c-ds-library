#include "HashMap.h"
#include "../DoubleLinkedList/gen_dlist.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct Item
{
    void* m_key;
    void* m_value;
}Item;

struct HashMap
{
    List** m_buckets;
    size_t (*m_hashFunction)(const void* _data);
    int (*m_equalityFunction)(const void* _firstData, const void* _secondData);
    size_t m_hashSize; /*original size given by the client*/
    size_t m_capacity; /*real hash size */
    size_t m_numOfItems; /*number of occupied places in the table*/
};

// Macros
#define HASH_GET_IDX(map,key) ((map)->m_hashFunction((key)) % (map)->m_capacity)
#define GET_BUCKET(map,key) (map->m_buckets[HASH_GET_IDX(map,key)])

// Helper functions
static const Item* HashMapMakeItem(const void* _key, const void* _value);
static ListItr FindKeyInBucket(const void* _key, const EqualityFunction _eq, const ListItr _start, const ListItr _end);
static MapResult CheckParams(const HashMap* _map, const void* _key);
static MapResult InsertItemToBucket(List* _bucket, const void* _key, const void* _value);
static size_t GetNextPrime(size_t _number);
static int BucketActionAdapter(void* _element, void* _context);

HashMap* HashMap_Create(size_t _capacity, HashFunction _hashFunc, EqualityFunction _keysEqualFunc)
{
    if (_capacity == 0 || !_hashFunc || !_keysEqualFunc) 
    { 
        return NULL; 
    }

    HashMap* hash = malloc(sizeof(HashMap));
    if (!hash) 
    { 
        return NULL; 
    }

    hash->m_capacity = GetNextPrime(_capacity);
    hash->m_hashSize = _capacity;
    hash->m_numOfItems = 0;
    hash->m_hashFunction = _hashFunc;
    hash->m_equalityFunction = _keysEqualFunc;

    hash->m_buckets = calloc(hash->m_capacity, sizeof(List*));
    if (!hash->m_buckets)
    {
        free(hash);
        return NULL;
    }

    return hash;
}

void HashMap_Destroy(HashMap** _map, void (*_keyDestroy)(void* _key), void (*_valDestroy)(void* _value))
{
    if (_map == NULL || *_map == NULL)
    {
        return;
    }

    HashMap* map = *_map;

    for (size_t i = 0; i < map->m_capacity; ++i)
    {
        List* bucket = map->m_buckets[i];
        if (bucket)
        {
            Item* poppedItem;
            while ((poppedItem = (Item*)ListPopHead(bucket)) != NULL)
            {
                if (_keyDestroy)
                {
                    _keyDestroy(poppedItem->m_key);
                }
                if (_valDestroy)
                {
                    _valDestroy(poppedItem->m_value);
                }
                free(poppedItem);
            }
            ListDestroy(&bucket, NULL);
        }
    }
    free(map->m_buckets);
    free(map);
    *_map = NULL;

    return;
}

MapResult HashMap_Insert(HashMap* _map, const void* _key, const void* _value)
{
    // Check params
    MapResult status = CheckParams(_map, _key);
    if (status != MAP_SUCCESS) { return status; }

    // Get bucket
    List* bucket = GET_BUCKET(_map, _key);
    if (bucket == NULL)
    {
        bucket = ListCreate();
        if (bucket == NULL) { return MAP_ALLOCATION_ERROR; }
        GET_BUCKET(_map, _key) = bucket;
    }

    ListItr begin = ListItrBegin(bucket);
    ListItr end   = ListItrEnd(bucket);

    if (FindKeyInBucket(_key, _map->m_equalityFunction, begin, end) != end)
    {
        return MAP_KEY_DUPLICATE_ERROR;
    }

    MapResult result = InsertItemToBucket(bucket, _key, _value);
    if (result == MAP_SUCCESS)
    {
        _map->m_numOfItems += 1;
    }
    return result;
}


MapResult HashMap_Remove(HashMap* _map, const void* _searchKey, void** _pKey, void** _pValue)
{
    // Check params
    MapResult status = CheckParams(_map, _searchKey);
    if (status != MAP_SUCCESS)
    {
        *_pKey = NULL;
        *_pValue = NULL;
        return status;
    }

    // Get bucket
    List* bucket = GET_BUCKET(_map, _searchKey);
    if (bucket == NULL)
    {
        *_pKey = NULL;
        *_pValue = NULL;
        return MAP_KEY_NOT_FOUND_ERROR;
    }

    ListItr beginBucket = ListItrBegin(bucket);
    ListItr endBucket   = ListItrEnd(bucket);

    // Iterating for key in bucket
    ListItr it = FindKeyInBucket(_searchKey, _map->m_equalityFunction, beginBucket, endBucket);
    if (it == endBucket)
    {
        *_pKey = NULL;
        *_pValue = NULL;
        return MAP_KEY_NOT_FOUND_ERROR;
    }

    // Returning key-value
    Item* item = ListItrRemove(it);
    *_pKey = item->m_key;
    *_pValue = item->m_value;

    // Free key-value item holder
    free(item);
    _map->m_numOfItems -= 1;
    return MAP_SUCCESS;
}

MapResult HashMap_Find(const HashMap* _map, const void* _key, void** _pValue)
{
    // Check params
    MapResult status = CheckParams(_map, _key);
    if (status != MAP_SUCCESS) 
    {
        *_pValue = NULL;
        return status; 
    }

    // Get bucket
    List* bucket = GET_BUCKET(_map, _key);
    if (bucket == NULL)
    {
        *_pValue = NULL;
        return MAP_KEY_NOT_FOUND_ERROR;
    }

    ListItr beginBucket = ListItrBegin(bucket);
    ListItr endBucket   = ListItrEnd(bucket);

    // Look for key's entry
    ListItr it = FindKeyInBucket(_key, _map->m_equalityFunction, beginBucket, endBucket);
    if (it == endBucket)
    {
        *_pValue = NULL;
        return MAP_KEY_NOT_FOUND_ERROR;
    }

    Item* item = ListItrGet(it);
    *_pValue = item->m_value;

    return MAP_SUCCESS;
}

size_t HashMap_Size(const HashMap* _map)
{
    return _map ? _map->m_numOfItems : 0;
}

typedef struct ForEachContext
{
    KeyValueActionFunction m_userAction;
    void* m_userContext;
    size_t m_count;
}ForEachContext;


size_t HashMap_ForEach(const HashMap* _map, KeyValueActionFunction _action, void* _context)
{
    if (_map == NULL || _action == NULL) { return 0; }
    ForEachContext ctxWrapper = {_action, _context, 0};

    for (size_t i = 0 ; i < _map->m_capacity ; ++i)
    {
        List* bucket = _map->m_buckets[i];
        if (bucket == NULL)
        {
            continue;
        }
        ListItrForEach(ListItrBegin(bucket), ListItrEnd(bucket), BucketActionAdapter, &ctxWrapper);   
    }

    size_t iterationsNumber = ctxWrapper.m_count;
    return iterationsNumber;
}


// static functions

static bool IsPrime(size_t _number)
{
    if (_number < 2) { return false; }
    if (_number % 2 == 0) { return (_number == 2); }

    for (size_t i = 3; i * i <= _number; i += 2)
    {
        if (_number % i == 0)
        {
            return false;
        }
    }

    return true;
}

static size_t GetNextPrime(size_t _number)
{
    /* If even, start at the next odd number */
    if (_number % 2 == 0)
    {
        ++_number;
    }

    while (!IsPrime(_number))
    {
        _number += 2;
    }
    return _number;
}

static const Item* HashMapMakeItem(const void* _key,const void* _value)
{
    Item* item = malloc(sizeof(Item));

    if (item)
    {
        item->m_key = (void*)_key;
        item->m_value = (void*)_value;
    }

    return item;
}

static ListItr FindKeyInBucket(const void* _key, const EqualityFunction _eq, const ListItr _start, const ListItr _end)
{
    ListItr it;
    for (it = _start ; it != _end ; it = ListItrNext(it))
    {
        Item* itItem = ListItrGet(it);
        if (_eq(_key, itItem->m_key))
        {
            return it;
        }
    }

    return it;
}

static MapResult CheckParams(const HashMap* _map, const void* _key)
{
    if (_map == NULL)
    {
        return MAP_UNINITIALIZED_ERROR;
    }

    if (_key == NULL)
    {
        return MAP_KEY_NULL_ERROR;
    }

    return MAP_SUCCESS;
}

static MapResult InsertItemToBucket(List* _bucket, const void* _key, const void* _value)
{
    const Item* itemToInsert = HashMapMakeItem(_key, _value);
    if (itemToInsert == NULL)
    {
        return MAP_ALLOCATION_ERROR;
    }

    if (ListPushTail(_bucket, (void*) itemToInsert) == NULL)
    {
        free((void*)itemToInsert);
        return MAP_ALLOCATION_ERROR;
    }
    return MAP_SUCCESS;
}

static int BucketActionAdapter(void* _element, void* _context)
{
    ForEachContext* con = (ForEachContext*)_context;
    Item* item = (Item*)_element;
    
    int actResult = con->m_userAction(item->m_key, item->m_value, con->m_userContext);
    if (actResult)
    {
        con->m_count += 1;
        return 1;
    }

    return 0; // action failed, stop
}