#include <stdio.h>
#include "../bin_tree.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int g_test_failures = 0;
#define ARRAY_SIZE 10
#define LENGTH(array) sizeof(array)/sizeof(array[0])
#define FOR(i, N) for (size_t (i) = 0 ; i < (N) ; ++i)

#define SOFT_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, message); \
            g_test_failures++; \
        } \
    } while (0)


// Test functions
static bool TestBSTreeCreate(void);
static bool  TestBSTreeDestroy(void);
// static bool TestBSTreeInsert(void);
static bool TestBSTreeItrBegin(void);
static bool TestBSTreeItrEnd(void);
// static BSTreeItr TestBSTreeItrNext(BSTreeItr _it);
// static BSTreeItr TestBSTreeItrPrev(BSTreeItr _it);
// static void* TestBSTreeItrRemove(BSTreeItr _it);
// static void* TestBSTreeItrGet(BSTreeItr _it);
// static BSTreeItr TestBSTreeForEach(const BSTree* _tree, BSTreeTraversalMode _mode, ActionFunction _action, void* _context);

// helpers
static int IntegerComparator(void* lValue, void* rValue);
static void DestroyInt(void* _integer);
static BSTree* BuildLocalTreeInteger(int array[], size_t length);
static void PrintInteger(void* _item);
static void PrintTree(BSTree* _tree, void (*_printer)(void* _item));
static BSTree* BuildHeapMemoryTreeInteger(int array[], size_t length);


static bool TestBSTreeCreate(void)
{
    printf("************ TestBSTreeCreate ************\n");
    // Null input
    BSTree* nullTree = BSTreeCreate(NULL);
    assert(nullTree == NULL);
    
    int array[] = {30, 20, 10, 25, 45};
/********* 
    30
   /  \
  20   45
 /  \
10  25
*********/

    BSTree* tree = BuildLocalTreeInteger(array, LENGTH(array));
    if (!tree)
    {
        return false;
    }
    
    if (!tree)
    {
        return false;
    }
    PrintTree(tree, PrintInteger);

    BSTreeDestroy(&tree, NULL);
    return true;
}

static bool  TestBSTreeDestroy()
{
    printf("************ TestBSTreeCreate ************\n");
    int array[] = {30, 20, 10, 25, 45};
    
    BSTree* tree = BuildHeapMemoryTreeInteger(array, LENGTH(array));
    BSTreeDestroy(&tree, DestroyInt);
    return true;
}

static bool TestBSTreeItrBegin(void)
{
    printf("************ TestBSTreeItrBegin ************\n");
    BSTree* tree = BSTreeCreate(IntegerComparator);
    BSTreeItr sentinelNode = BSTreeItrEnd(tree);
    BSTreeItr emptyTreeIterator = BSTreeItrBegin(tree);
    assert(emptyTreeIterator == sentinelNode);

    int array[] = {30, 20, 10, 25, 45};
    int* expectedBegin = &array[0];
    FOR(i,LENGTH(array))
    {
        
        assert(BSTreeInsert(tree, &array[i]));
        BSTreeItr itBegin = BSTreeItrBegin(tree);
        int* beginItem = BSTreeItrGet(itBegin);
        FOR(j, i+1)
        {
            if (*expectedBegin > array[j])
            {
                expectedBegin = &array[j];
            }
        }
        assert(beginItem == expectedBegin);
    }

    return true;
}

static bool TestBSTreeItrEnd(void)
{
    printf("************ TestBSTreeItrEnd ************\n");
    BSTree* tree = BSTreeCreate(IntegerComparator);
    BSTreeItr sentinelNode = BSTreeItrBegin(tree);
    const BSTreeItr itEnd = BSTreeItrEnd(tree);
    assert(itEnd == sentinelNode);

    int array[] = {30, 20, 10, 25, 45};
    int* expectedEnd = itEnd;
    FOR(i,LENGTH(array))
    {
        
        assert(BSTreeInsert(tree, &array[i]));
        BSTreeItr itEnd = BSTreeItrEnd(tree);
        assert(itEnd == expectedEnd);
    }

    return true;
}


int main(void)
{
    printf("Hello tests\n");
    size_t test_passed = 0;
    size_t test_failed = 0;
    bool (*tests[])(void) = {TestBSTreeCreate, 
        TestBSTreeDestroy, 
        TestBSTreeItrBegin
       ,TestBSTreeItrEnd};

    FOR(i, LENGTH(tests))
    {
        if (tests[i]())
        {
            ++test_passed;
        }
        else
        {
            ++test_failed;
        }
    }
    printf("Tests passed: %zu, Tests failed: %zu\n", test_passed, test_failed);
    return 0;
}

static int IntegerComparator(void* lValue, void* rValue)
{
    int* left = (int*) lValue;
    int* right = (int*) rValue;

    return *left - *right;
}


static BSTree* BuildLocalTreeInteger(int array[], size_t length)
{
    BSTree* tree = BSTreeCreate(IntegerComparator);
    
    FOR(i, length)
    {
        BSTreeItr it = BSTreeInsert(tree, &array[i]);
        if (!it)
        {
            assert(0);
        }
    }

    return tree;
}

static BSTree* BuildHeapMemoryTreeInteger(int array[], size_t length)
{
    BSTree* tree = BSTreeCreate(IntegerComparator);
    
    FOR(i, length)
    {
        int* item = malloc(sizeof(int));
        assert(item);
        *item = array[i];
        BSTreeItr it = BSTreeInsert(tree, item);
        if (!it)
        {
            assert(0);
        }
    }

    return tree;
}

static void PrintInteger(void* _item)
{
    printf("%d ", *(int*)_item);
}

static void PrintTree(BSTree* _tree, void (*_printer)(void* _item))
{
    printf("************ PrintTree ************\n");
    BSTreeItr it = BSTreeItrBegin(_tree);
    while (it != BSTreeItrEnd(_tree))
    {
        _printer(BSTreeItrGet(it));
        it = BSTreeItrNext(it);
    }
    printf("\n");
}

static void DestroyInt(void* _integer)
{
    free((int*)_integer);
}