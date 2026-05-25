#include "../HashMap.h"
#include <stdio.h>
#include <stdlib.h>

#define ASSERT_TEST(test, errorMessage) \
    if (!(test)) { \
        printf("\033[31m%s (at %s:%d): %s\033[0m\n", errorMessage, __FILE__, __LINE__, #test); \
        return 0; \
    }

/* ── helpers ── */

static size_t HashFunctionInteger(const void* _key)
{
    return (size_t)(*(const int*)_key);
}

static size_t HashFunctionConstant(const void* _key)
{
    (void)_key;
    return 0;
}

static int EqualityFunctionInteger(const void* _firstKey, const void* _secondKey)
{
    return (*(const int*)_firstKey == *(const int*)_secondKey);
}

static int ActionFunctionSum(const void* _key, void* _value, void* _context)
{
    (void)_key;
    (void)_value;
    int* sum = (int*)_context;
    (*sum) += 1;
    return 1;
}

static int ActionFunctionStopImmediately(const void* _key, void* _value, void* _context)
{
    (void)_key;
    (void)_value;
    (void)_context;
    return 0;
}

typedef struct StopOnKeyContext
{
    int m_stopKey;
    int m_seen;
} StopOnKeyContext;

static int ActionFunctionStopOnKey(const void* _key, void* _value, void* _context)
{
    (void)_value;
    StopOnKeyContext* ctx = (StopOnKeyContext*)_context;
    int keyVal = *(const int*)_key;
    ctx->m_seen += 1;
    return (keyVal != ctx->m_stopKey);
}

static int s_keysFreed = 0;
static int s_valsFreed = 0;

static void DestroyKeyCounter(void* _key)
{
    free(_key);
    s_keysFreed += 1;
}

static void DestroyValCounter(void* _value)
{
    free(_value);
    s_valsFreed += 1;
}

/* ── tests ── */

static int test_create_invalid(void)
{
    printf("test_create_invalid\n");

    ASSERT_TEST(HashMap_Create(0, HashFunctionInteger, EqualityFunctionInteger) == NULL,
                "Create with capacity 0 should fail");
    ASSERT_TEST(HashMap_Create(10, NULL, EqualityFunctionInteger) == NULL,
                "Create with NULL hash func should fail");
    ASSERT_TEST(HashMap_Create(10, HashFunctionInteger, NULL) == NULL,
                "Create with NULL equality func should fail");

    printf("  PASS\n");
    return 1;
}

static int test_happy_path(void)
{
    printf("test_happy_path\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    HashMap_Destroy(&map, NULL, NULL);
    ASSERT_TEST(map == NULL, "HashMap_Destroy failed");

    int key = 1;
    int value = 2;
    MapResult result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_UNINITIALIZED_ERROR, "Insert on destroyed map should fail");

    map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");

    void* pKey = NULL;
    void* pValue = NULL;
    result = HashMap_Remove(map, &key, &pKey, &pValue);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Remove failed");
    ASSERT_TEST(pKey != NULL, "HashMap_Remove failed");
    ASSERT_TEST(pValue != NULL, "HashMap_Remove failed");
    ASSERT_TEST(*(int*)pKey == key, "HashMap_Remove failed");
    ASSERT_TEST(*(int*)pValue == value, "HashMap_Remove failed");

    result = HashMap_Remove(map, &key, &pKey, &pValue);
    ASSERT_TEST(result == MAP_KEY_NOT_FOUND_ERROR, "Second remove should fail");
    ASSERT_TEST(pKey == NULL, "HashMap_Remove failed");
    ASSERT_TEST(pValue == NULL, "HashMap_Remove failed");

    pValue = NULL;
    result = HashMap_Find(map, &key, &pValue);
    ASSERT_TEST(result == MAP_KEY_NOT_FOUND_ERROR, "Find after remove should fail");
    ASSERT_TEST(pValue == NULL, "HashMap_Find should return NULL");

    result = HashMap_Insert(map, &key, &value);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");

    result = HashMap_Find(map, &key, &pValue);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Find failed");
    ASSERT_TEST(pValue != NULL, "HashMap_Find failed");
    ASSERT_TEST(*(int*)pValue == value, "HashMap_Find failed");

    ASSERT_TEST(HashMap_Size(map) == 1, "HashMap_Size failed");

    ASSERT_TEST(HashMap_ForEach(map, NULL, NULL) == 0, "ForEach with NULL action should return 0");

    int sum = 0;
    size_t count = HashMap_ForEach(map, ActionFunctionSum, &sum);
    ASSERT_TEST(count == 1, "HashMap_ForEach failed");
    ASSERT_TEST(sum == 1, "HashMap_ForEach failed");

    int key2 = 2;
    int value2 = 3;
    result = HashMap_Insert(map, &key2, &value2);
    ASSERT_TEST(result == MAP_SUCCESS, "HashMap_Insert failed");
    ASSERT_TEST(HashMap_Size(map) == 2, "HashMap_Size failed");

    sum = 0;
    count = HashMap_ForEach(map, ActionFunctionSum, &sum);
    ASSERT_TEST(count == 2, "HashMap_ForEach failed");
    ASSERT_TEST(sum == 2, "HashMap_ForEach failed");

    HashMap_Destroy(&map, NULL, NULL);
    ASSERT_TEST(map == NULL, "HashMap_Destroy failed");

    printf("  PASS\n");
    return 1;
}

static int test_insert_errors(void)
{
    printf("test_insert_errors\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    int key = 1;
    int value = 2;

    ASSERT_TEST(HashMap_Insert(NULL, &key, &value) == MAP_UNINITIALIZED_ERROR,
                "Insert with NULL map should fail");
    ASSERT_TEST(HashMap_Insert(map, NULL, &value) == MAP_KEY_NULL_ERROR,
                "Insert with NULL key should fail");

    int missingKey = 99;
    void* pValue = NULL;
    ASSERT_TEST(HashMap_Find(map, &missingKey, &pValue) == MAP_KEY_NOT_FOUND_ERROR,
                "Find on empty map should fail");
    ASSERT_TEST(pValue == NULL, "Find output should be NULL");

    void* pKey = NULL;
    ASSERT_TEST(HashMap_Remove(map, &missingKey, &pKey, &pValue) == MAP_KEY_NOT_FOUND_ERROR,
                "Remove on empty map should fail");

    ASSERT_TEST(HashMap_Insert(map, &key, &value) == MAP_SUCCESS, "First insert should succeed");
    ASSERT_TEST(HashMap_Insert(map, &key, &value) == MAP_KEY_DUPLICATE_ERROR,
                "Duplicate insert should fail");
    ASSERT_TEST(HashMap_Size(map) == 1, "Size should remain 1 after duplicate");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_remove_errors(void)
{
    printf("test_remove_errors\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    int key = 1;
    int value = 2;
    ASSERT_TEST(HashMap_Insert(map, &key, &value) == MAP_SUCCESS, "Insert failed");

    void* pKey = NULL;
    void* pValue = NULL;

    ASSERT_TEST(HashMap_Remove(NULL, &key, &pKey, &pValue) == MAP_UNINITIALIZED_ERROR,
                "Remove with NULL map should fail");
    ASSERT_TEST(HashMap_Remove(map, NULL, &pKey, &pValue) == MAP_KEY_NULL_ERROR,
                "Remove with NULL key should fail");
    ASSERT_TEST(HashMap_Remove(map, &key, NULL, &pValue) == MAP_NULL_PTR_ERROR,
                "Remove with NULL pKey should fail");
    ASSERT_TEST(HashMap_Remove(map, &key, &pKey, NULL) == MAP_NULL_PTR_ERROR,
                "Remove with NULL pValue should fail");

    ASSERT_TEST(HashMap_Remove(map, &key, &pKey, &pValue) == MAP_SUCCESS, "Remove should succeed");
    ASSERT_TEST(HashMap_Size(map) == 0, "Size should be 0 after remove");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_find_errors(void)
{
    printf("test_find_errors\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    int key = 1;
    void* pValue = (void*)0x1;

    ASSERT_TEST(HashMap_Find(NULL, &key, &pValue) == MAP_UNINITIALIZED_ERROR,
                "Find with NULL map should fail");
    ASSERT_TEST(HashMap_Find(map, NULL, &pValue) == MAP_KEY_NULL_ERROR,
                "Find with NULL key should fail");

    pValue = (void*)0x1;
    ASSERT_TEST(HashMap_Find(map, &key, &pValue) == MAP_KEY_NOT_FOUND_ERROR,
                "Find on empty map should fail");
    ASSERT_TEST(pValue == NULL, "Find output should be NULL on not found");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_size_edges(void)
{
    printf("test_size_edges\n");

    ASSERT_TEST(HashMap_Size(NULL) == 0, "Size of NULL map should be 0");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");
    ASSERT_TEST(HashMap_Size(map) == 0, "Empty map size should be 0");

    int key = 1;
    int value = 2;
    ASSERT_TEST(HashMap_Insert(map, &key, &value) == MAP_SUCCESS, "Insert failed");
    ASSERT_TEST(HashMap_Size(map) == 1, "Size should be 1");

    void* pKey = NULL;
    void* pValue = NULL;
    ASSERT_TEST(HashMap_Remove(map, &key, &pKey, &pValue) == MAP_SUCCESS, "Remove failed");
    ASSERT_TEST(HashMap_Size(map) == 0, "Size should be 0 after removing last item");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_foreach_edges(void)
{
    printf("test_foreach_edges\n");

    ASSERT_TEST(HashMap_ForEach(NULL, ActionFunctionSum, NULL) == 0,
                "ForEach on NULL map should return 0");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    ASSERT_TEST(HashMap_ForEach(map, ActionFunctionSum, NULL) == 0,
                "ForEach on empty map should return 0");

    int key1 = 1;
    int val1 = 10;
    int key2 = 2;
    int val2 = 20;
    ASSERT_TEST(HashMap_Insert(map, &key1, &val1) == MAP_SUCCESS, "Insert failed");
    ASSERT_TEST(HashMap_Insert(map, &key2, &val2) == MAP_SUCCESS, "Insert failed");

    ASSERT_TEST(HashMap_ForEach(map, ActionFunctionStopImmediately, NULL) == 0,
                "ForEach should stop immediately and return 0");

    StopOnKeyContext ctx = { .m_stopKey = 2, .m_seen = 0 };
    size_t count = HashMap_ForEach(map, ActionFunctionStopOnKey, &ctx);
    ASSERT_TEST(count == 1, "ForEach should invoke action once before stopping on key 2");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_collisions(void)
{
    printf("test_collisions\n");

    HashMap* map = HashMap_Create(3, HashFunctionConstant, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    int keys[] = { 1, 2, 3, 4, 5 };
    int vals[] = { 10, 20, 30, 40, 50 };

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    {
        ASSERT_TEST(HashMap_Insert(map, &keys[i], &vals[i]) == MAP_SUCCESS,
                    "Insert failed in collision test");
    }
    ASSERT_TEST(HashMap_Size(map) == 5, "Size should be 5 after collision inserts");

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    {
        void* pValue = NULL;
        ASSERT_TEST(HashMap_Find(map, &keys[i], &pValue) == MAP_SUCCESS,
                    "Find failed in collision test");
        ASSERT_TEST(*(int*)pValue == vals[i], "Find returned wrong value");
    }

    void* pKey = NULL;
    void* pValue = NULL;
    ASSERT_TEST(HashMap_Remove(map, &keys[2], &pKey, &pValue) == MAP_SUCCESS,
                "Remove middle key failed");
    ASSERT_TEST(*(int*)pKey == keys[2], "Removed wrong key");
    ASSERT_TEST(HashMap_Size(map) == 4, "Size should be 4 after middle remove");

    ASSERT_TEST(HashMap_Insert(map, &keys[2], &vals[2]) == MAP_SUCCESS,
                "Re-insert after remove failed");
    ASSERT_TEST(HashMap_Size(map) == 5, "Size should be 5 after re-insert");

    pValue = NULL;
    ASSERT_TEST(HashMap_Find(map, &keys[2], &pValue) == MAP_SUCCESS,
                "Find after re-insert failed");
    ASSERT_TEST(*(int*)pValue == vals[2], "Find after re-insert returned wrong value");

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    {
        pKey = NULL;
        pValue = NULL;
        ASSERT_TEST(HashMap_Remove(map, &keys[i], &pKey, &pValue) == MAP_SUCCESS,
                    "Remove failed in collision test");
    }
    ASSERT_TEST(HashMap_Size(map) == 0, "Size should be 0 after removing all");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

static int test_destroy_with_callbacks(void)
{
    printf("test_destroy_with_callbacks\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    s_keysFreed = 0;
    s_valsFreed = 0;

    for (int i = 0; i < 3; ++i)
    {
        int* key = malloc(sizeof(int));
        int* val = malloc(sizeof(int));
        ASSERT_TEST(key != NULL && val != NULL, "malloc failed");
        *key = i;
        *val = i * 10;
        ASSERT_TEST(HashMap_Insert(map, key, val) == MAP_SUCCESS, "Insert failed");
    }

    HashMap_Destroy(&map, DestroyKeyCounter, DestroyValCounter);
    ASSERT_TEST(map == NULL, "HashMap_Destroy failed");
    ASSERT_TEST(s_keysFreed == 3, "Key destroy callback count wrong");
    ASSERT_TEST(s_valsFreed == 3, "Value destroy callback count wrong");

    printf("  PASS\n");
    return 1;
}

static int test_multiple_keys_size(void)
{
    printf("test_multiple_keys_size\n");

    HashMap* map = HashMap_Create(10, HashFunctionInteger, EqualityFunctionInteger);
    ASSERT_TEST(map != NULL, "HashMap_Create failed");

    int keys[] = { 1, 2, 3, 4, 5, 6 };
    int vals[] = { 10, 20, 30, 40, 50, 60 };

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    {
        ASSERT_TEST(HashMap_Insert(map, &keys[i], &vals[i]) == MAP_SUCCESS, "Insert failed");
    }
    ASSERT_TEST(HashMap_Size(map) == 6, "Size should be 6");

    void* pKey = NULL;
    void* pValue = NULL;
    for (size_t i = 0; i < 3; ++i)
    {
        ASSERT_TEST(HashMap_Remove(map, &keys[i], &pKey, &pValue) == MAP_SUCCESS, "Remove failed");
    }
    ASSERT_TEST(HashMap_Size(map) == 3, "Size should be 3 after removing half");

    int sum = 0;
    size_t count = HashMap_ForEach(map, ActionFunctionSum, &sum);
    ASSERT_TEST(count == 3, "ForEach count should match size");
    ASSERT_TEST(sum == 3, "ForEach sum should match count");

    HashMap_Destroy(&map, NULL, NULL);

    printf("  PASS\n");
    return 1;
}

int main(void)
{
    printf("=== HashMap tests ===\n");

    int failed = 0;
    failed += !test_create_invalid();
    failed += !test_happy_path();
    failed += !test_insert_errors();
    failed += !test_remove_errors();
    failed += !test_find_errors();
    failed += !test_size_edges();
    failed += !test_foreach_edges();
    failed += !test_collisions();
    failed += !test_destroy_with_callbacks();
    failed += !test_multiple_keys_size();

    if (failed)
    {
        printf("=== %d TEST(S) FAILED ===\n", failed);
        return 1;
    }

    printf("=== ALL PASSED ===\n");
    return 0;
}
