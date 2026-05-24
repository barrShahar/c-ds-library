#include "../genstack.h"
#include "../../GenVector/vector.h"
#include "stdio.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#define TYPE int
#define UNUSED(x) (void)(x)
#define LENGTH(array) (sizeof(array) / sizeof(array[0]))

/*
  For testomg Memory Leaks run:
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_s
*/
void FreePtr(void* _ptr)
{
    free(_ptr);
}

int PrintSumIntegersAction(void* _number, size_t _index, void* _sumPtr)
{
    int* numberPtr = (int*)_number;
    int* sumPtr = (int*)_sumPtr;
    *sumPtr += *numberPtr;
    printf("%ld: %d  \n", _index, *sumPtr);
    return 1; // Keep going, dont break the foreach loop
}

int PrintIntegerAction(void* _number, size_t _index, void* _context)
{
    UNUSED(_context); // To silent a warning

    int* numberPtr = (int*)_number;
    printf("%ld: %d  \n", _index, *numberPtr);
    return 1; // Keep going, dont break the foreach loop
}

Stack *TestStackSetUp(size_t _initialCapacity, size_t _blockSize)
{
    Stack* stack = StackCreate(_initialCapacity, _blockSize);
    assert(stack != NULL);
    return stack;
}

void TestTearDown(Stack* _stack, void (*_elementDestroy)(void* _item))
{
    StackDestroy(&_stack, _elementDestroy);
    assert(_stack == NULL);
}

void VectorObjectDestroy(void* _item)
{
    VectorDestroy((Vector**)_item, NULL);
}


bool TestStackCreate()
{
    printf(" * Test Stack Create\n");
    printf("Test create stack with 0 capacity and 0 block size\n");
    Stack* stack = StackCreate(0, 0);
    assert(stack == NULL);

    printf("Test create stack with 10 capacity and 10 block size\n");
    stack = StackCreate(10, 10);
    assert(stack != NULL);

    // Clean up
    printf("Clean up stack\n");
    StackDestroy(&stack, NULL);
    assert(stack == NULL);

    printf("[Passed] Test Stack Create\n==\n");
    return true;
}

bool TestPushErrorCases()
{
    printf(" * Test Push Error Cases\n");

    printf("Test overflow error\n");
    Stack* stack = StackCreate(5, 0);
    assert(stack != NULL);

    size_t i;
    for (i = 0 ; i < 5 ; ++i)
    {
        assert(StackPush(stack, &i) == STACK_SUCCESS);
    }

    assert(StackPush(stack, &i) == STACK_OVERFLOW_ERROR);

    printf("Test unitialized stack\n");
    Stack* unitializedStack = NULL;
    assert(StackPush(unitializedStack, &i) == STACK_UNINITIALIZED_ERROR);

    printf("Test null item\n");
    void* nullItem = NULL;
    assert(StackPush(stack, nullItem) == STACK_NULL_PTR);

    // Clean up
    TestTearDown(stack, NULL);
    printf("[Passed] Test Push Error Cases\n==\n");

    return true;
}

bool TestPushToConstStack()
{
    printf(" * Test Push to Constant Stack\n");
    const size_t initialCapacity = 5;
    Stack* nonDynamicStack = StackCreate(initialCapacity, 0);
    assert(nonDynamicStack != NULL);

    size_t i;
    for (i = 0 ; i < initialCapacity ; ++i)
    {
        assert(StackPush(nonDynamicStack, &i) == STACK_SUCCESS);
    }

    assert(StackPush(nonDynamicStack, &i) == STACK_OVERFLOW_ERROR);

    // Clean up
    TestTearDown(nonDynamicStack, NULL);
    
    printf("[Passed] Test Constant Stack\n==\n");

    return true;
}

bool TestStackGrowth()
{
    printf(" * Test Stack Growth\n");
    const size_t initialCapacity = 5;
    const size_t blockSize = 5;

    Stack* dynamicStack = StackCreate(initialCapacity, blockSize);
    assert(dynamicStack != NULL);

    size_t i;
    for (i = 0 ; i < initialCapacity ; ++i)
    {
        assert(StackPush(dynamicStack, &i) == STACK_SUCCESS);
    }

    assert(StackCapacity(dynamicStack) == initialCapacity);

    assert(StackPush(dynamicStack, &i) == STACK_SUCCESS);
    assert(StackCapacity(dynamicStack) == initialCapacity + blockSize);
    
    TestTearDown(dynamicStack, NULL);
    printf("[Passed] Test Stack Growth\n==\n");

    return true;
}

bool TestStackPop()
{
    printf(" * Test Stack Pop\n");
    const size_t initialCapacity = 5;
    Stack* stack = StackCreate(initialCapacity, 0);
    assert(stack != NULL);

    size_t const arraySize = 5;
    size_t array[arraySize];
    // Initialize array with values from 0 to 4
    for (size_t i = 0; i < arraySize; ++i)
    {
        array[i] = i;
    }

    // Push the array to the stack
    for (size_t i = 0 ; i < 5 ; ++i)
    {
        assert(StackPush(stack, array + i) == STACK_SUCCESS);
    }

    // Pop the array from the stack
    for (size_t j = 0 ; j < initialCapacity ; ++j)
    {
        size_t poppedValue = 0;
        size_t* ptrPoppedValue = &poppedValue;
        // Pop the value from the stack
        assert(StackPop(stack, (void**) &ptrPoppedValue) == STACK_SUCCESS);
        // Check if the popped value is the correct value
        assert(*ptrPoppedValue == array[arraySize - j - 1]);
    }

    // Clean up
    TestTearDown(stack, NULL);
    printf("[Passed] Test Stack Pop\n==\n");
    return true;
}    

bool TestStackPopErrorCases()
{
    printf(" * Test Stack Pop Error Cases\n");
    Stack* stack = StackCreate(5, 0);
    assert(stack != NULL);
    
    size_t i = 0;
    size_t* ptrItem = &i;
    printf("Test unitialized stack\n");
    Stack* unitializedStack = NULL;
    assert(StackPop(unitializedStack, (void**) &ptrItem) == STACK_UNINITIALIZED_ERROR);

    printf("Test null item\n");
    void* nullItem = NULL;
    assert(StackPop(stack, nullItem) == STACK_NULL_PTR);

    Stack* emptyStack = TestStackSetUp(2, 2);

    printf("Test empty stack\n");
    assert(StackPop(stack, (void**) &ptrItem) == STACK_SIZE_ZERO_ERROR);

    // Clean up
    TestTearDown(emptyStack, NULL);
    TestTearDown(stack, NULL);
    printf("[Passed] Test Stack Pop Error Cases\n==\n");
    return true;
}

bool TestStackTop()
{
    printf(" * Test Stack Top\n");
    Stack* stack = StackCreate(5, 0);
    assert(stack != NULL);
    
    size_t i = 0;
    size_t topValue;
    for (i = 0 ; i < 5 ; ++i)
    {
        assert(StackPush(stack, &i) == STACK_SUCCESS);
        size_t* ptrTopValue = &topValue;
        assert(StackTop(stack, (void**) &ptrTopValue) == STACK_SUCCESS);
        assert(*ptrTopValue == i);
    }

    // Clean up
    TestTearDown(stack, NULL);
    printf("[Passed] Test Stack Top\n==\n");
    return true;
}

bool TestStackTopErrorCases()
{
    printf(" * Test Stack Top Error Cases\n");
    printf("Test unitialized stack\n");
    Stack* unitializedStack = NULL;
    void* ptrUnitializedTopValue = NULL;
    assert(StackTop(unitializedStack, ptrUnitializedTopValue) == STACK_UNINITIALIZED_ERROR);

    printf("Test null pValue\n");
    Stack* emptyStack = TestStackSetUp(2, 2);
    assert(StackTop(emptyStack, NULL) == STACK_NULL_PTR);

    printf("Test null item on an empty stack\n");
    assert(StackTop(emptyStack, (void**) NULL) == STACK_NULL_PTR);
    Stack* stack = StackCreate(1, 0);
    assert(stack != NULL);

    printf("Test null item on a non-empty stack\n");
    size_t Item = 1;
    size_t* ptrItem = &Item;
    assert(StackPush(stack, &ptrItem) == STACK_SUCCESS);
    assert(StackTop(stack, NULL) == STACK_NULL_PTR);

    printf("Test empty stack\n");
    assert(StackTop(emptyStack, (void**) &ptrItem) == STACK_SIZE_ZERO_ERROR);


    // Clean up
    TestTearDown(emptyStack, NULL);
    TestTearDown(stack, NULL);

    printf("[Passed] Test Stack Top Error Cases\n==\n");
    return true;
    
}

bool TestStackPrint()
{
    printf(" * Test Stack Print\n");
    Stack* stack = StackCreate(5, 0);
    assert(stack != NULL);
    
    printf("Test print null stack\n");
    assert(StackPrint(NULL, NULL, NULL) == 0);
    printf("Test print null action\n");
    assert(StackPrint(stack, NULL, NULL) == 0);
    printf("Test print empty stack\n");
    assert(StackPrint(stack, PrintIntegerAction, NULL) == 0);

    printf("Test print full stack\n");
    for (size_t i = 0 ; i < 5 ; ++i)
    {
        size_t* ptrItem = malloc(sizeof(size_t));
        *ptrItem = i;
        assert(StackPush(stack, ptrItem) == STACK_SUCCESS);
    }
    assert(StackPrint(stack, PrintIntegerAction, NULL) == 5);

    printf("Test full stack with a null action\n");
    assert(StackPrint(stack, NULL, NULL) == 0);

    printf("Test print full stack with a sum action\n");
    int sum = 0;
    int* ptrSum = &sum;
    assert(StackPrint(stack, PrintSumIntegersAction, ptrSum) == 5);
    assert(sum == 10);


    // Clean up
    TestTearDown(stack, FreePtr);
    printf("[Passed] Test Stack Print\n==\n");
    return true;
}

int main()
{
    bool (*testFunctions[])() = {TestStackCreate, 
        TestPushErrorCases, 
        TestPushToConstStack, 
        TestStackGrowth,
        TestStackPop,
        TestStackPopErrorCases,
        TestStackTop,
        TestStackTopErrorCases,
        TestStackPrint
    };

    size_t count = 0;
    for (size_t i = 0 ; i < LENGTH(testFunctions) ; ++i)
    {
        testFunctions[i]();
        count++;
    }

    printf("============================\n");
    printf("Passed %ld out of %ld tests\n", count, LENGTH(testFunctions));

    return 0;
}