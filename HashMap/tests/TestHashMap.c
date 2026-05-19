#include "../HashMap.h"
#include <stdio.h>

#define ASSERT_TEST(test, errorMessage) \
    if (!(test)) { \
        printf("\033[31m%s (at %s:%d): %s\033[0m\n", errorMessage, __FILE__, __LINE__, #test); \
        return 0; \
    }

int ActionFunctionSumAndPrint(const void* _key, void* _value, void* _context)
{
    int* sum = (int*)_context;
    *sum += 1;
    printf("Key: %d, Value: %d, Sum: %d\n", *(int*)_key, *(int*)_value, *sum);
    return 1;
}


size_t HashFunctionInteger(const void* _key)
{
    return (size_t)_key;
}

int EqualityFunctionInteger(const void* _firstKey, const void* _secondKey)
{
    return (*(int*)_firstKey == *(int*)_secondKey);
}
int main(void)
{
    // Test HashMap_Create
    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    // Test HashMap_Destroy
    HashMap_Destroy(&map, NULL, NULL);
    ASSERT_TEST(map == NULL, "HashMap_Destroy failed");

    // Test HashMap_Insert
    int key = 1;
    int value = 2;
    MapResult result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_UNINITIALIZED_ERROR, "Error type is wrong");

    map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");
    result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");

    // Test HashMap_Remove
    void* pKey = NULL;  
    void* pValue = NULL;
    result = HashMap_Remove(map, &key, &pKey, &pValue);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Remove failed");
    ASSERT_TEST(pKey != NULL, "HashMap_Remove failed");
    ASSERT_TEST(pValue != NULL, "HashMap_Remove failed");
    ASSERT_TEST(*(int*)pKey == key, "HashMap_Remove failed");
    ASSERT_TEST(*(int*)pValue == value, "HashMap_Remove failed");

    result = HashMap_Remove(map, &key, &pKey, &pValue);
    ASSERT_TEST(result == MAP_KEY_NOT_FOUND_ERROR, "Error should be: MAP_KEY_NOT_FOUND_ERROR");
    ASSERT_TEST(pKey == NULL, "HashMap_Remove failed");
    ASSERT_TEST(pValue == NULL, "HashMap_Remove failed");

    // Test HashMap_Find
    pValue = NULL;
    result = HashMap_Find(map, &key, &pValue);
    ASSERT_TEST(result == MAP_KEY_NOT_FOUND_ERROR, "Error should be: MAP_KEY_NOT_FOUND_ERROR58");
    ASSERT_TEST(pValue == NULL, "HashMap_Find should return NULL59");
    
    result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");

    result = HashMap_Find(map, &key, &pValue);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Find failed");
    ASSERT_TEST(pValue != NULL, "HashMap_Find failed");
    ASSERT_TEST(*(int*)pValue == value, "HashMap_Find failed");

    // Test HashMap_Size
    size_t size = HashMap_Size(map);
    ASSERT_TEST(size == 1, "HashMap_Size failed");

    // Test HashMap_ForEach
    size_t count = HashMap_ForEach(map, NULL, NULL);
    ASSERT_TEST(count == 0, "HashMap_ForEach failed");

    // Test HashMap_ForEach with ActionFunctionSumAndPrint
    int sum = 0;
    count = HashMap_ForEach(map, ActionFunctionSumAndPrint, &sum);
    ASSERT_TEST(count == 1, "HashMap_ForEach failed");
    ASSERT_TEST(sum == 1, "HashMap_ForEach failed");
    ASSERT_TEST(HashMap_Size(map) == 1, "HashMap_Size failed");

    int key2 = 2;
    int value2 = 3;
    result = HashMap_Insert(map, &key2, &value2);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");
    ASSERT_TEST(HashMap_Size(map) == 2, "HashMap_Size failed");
    printf("Inserted key2: %d, value2: %d\n", key2, value2);
    sum = 0;
    count = HashMap_ForEach(map, ActionFunctionSumAndPrint, &sum);
    ASSERT_TEST(count == 2, "HashMap_ForEach failed");
    ASSERT_TEST(sum == 2, "HashMap_ForEach failed");
    ASSERT_TEST(HashMap_Size(map) == 2, "HashMap_Size failed");

    // Test HashMap_Destroy
    HashMap_Destroy(&map, NULL, NULL);
    ASSERT_TEST(map == NULL, "HashMap_Destroy failed");

    return 0;
}