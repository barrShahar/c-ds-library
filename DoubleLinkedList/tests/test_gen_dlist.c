#include <stdio.h>
#include <assert.h>
#include "../gen_dlist.h"

/* ── helpers ── */

static int sum_action(void* _element, void* _context)
{
    *(int*)_context += *(int*)_element;
    return 1;
}

static int stop_at_5(void* _element, void* _context)
{
    (void)_context;
    return *(int*)_element != 5;
}

static int compareInts(void* a, void* b)
{
    return *(int*)a == *(int*)b;
}

/* ── tests ── */

static void test_list_find(void)
{
    printf("test_list_find\n");
    List* list = ListCreate();

    int target = 42;

    ListItr expectedNull = ListFind(list, &target, compareInts);

    assert(expectedNull == NULL);
    int a = 1, b = 2, c = 3;

    ListPushHead(list, &a);
    ListPushHead(list, &b);
    ListPushHead(list, &c);
    assert(ListSize(list) == 3);

    expectedNull = ListFind(list, &target, compareInts);
    assert(expectedNull == NULL);

    int numbers[] = {5, 42, 13};
    for (size_t i = 0 ; i < sizeof(numbers)/sizeof(int) ; ++i)
    {
        ListPushHead(list, &numbers[i]);
    }

    ListItr it = ListFind(list, &target, compareInts);
    void* expectedToBe42 = ListItrGet(it);
    assert(compareInts((void*)&target, expectedToBe42));
    for (size_t i = 0 ; i < sizeof(numbers)/sizeof(int) ; ++i)
    {
        void* target = (void*)&numbers[i];
        it = ListFind(list, target, compareInts);
        assert(ListItrGet(it) == target);
    }
    
    ListDestroy(&list, NULL);
    printf("  PASS\n");


}

static void test_create_destroy(void)
{
    printf("test_create_destroy\n");

    List* list = ListCreate();
    assert(list != NULL);
    assert(ListIsEmpty(list));
    assert(ListSize(list) == 0);

    ListDestroy(&list, NULL);
    assert(list == NULL);

    /* double destroy must not crash */
    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_push_pop_head(void)
{
    printf("test_push_pop_head\n");

    List* list = ListCreate();
    int a = 1, b = 2, c = 3;

    ListPushHead(list, &a);
    ListPushHead(list, &b);
    ListPushHead(list, &c);
    assert(ListSize(list) == 3);
    assert(!ListIsEmpty(list));

    int* out = (int*)ListPopHead(list);
    assert(*out == 3);
    out = (int*)ListPopHead(list);
    assert(*out == 2);
    out = (int*)ListPopHead(list);
    assert(*out == 1);

    assert(ListIsEmpty(list));
    assert(ListPopHead(list) == NULL);   /* pop empty */

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_push_pop_tail(void)
{
    printf("test_push_pop_tail\n");

    List* list = ListCreate();
    int a = 10, b = 20, c = 30;

    ListPushTail(list, &a);
    ListPushTail(list, &b);
    ListPushTail(list, &c);
    assert(ListSize(list) == 3);

    int* out = (int*)ListPopTail(list);
    assert(*out == 30);
    out = (int*)ListPopTail(list);
    assert(*out == 20);
    out = (int*)ListPopTail(list);
    assert(*out == 10);

    assert(ListIsEmpty(list));
    assert(ListPopTail(list) == NULL);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_iterators(void)
{
    printf("test_iterators\n");

    List* list = ListCreate();
    int vals[4] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++) { ListPushTail(list, &vals[i]); }

    ListItr it = ListItrBegin(list);
    ListItr prevBegin = ListItrPrev(it);
    assert(it == prevBegin);

    ListItr end = ListItrEnd(list);
    int expected = 1;
    while (it != end)
    {
        assert(*(int*)ListItrGet(it) == expected++);
        it = ListItrNext(it);
    }
    assert(expected == 5);

    /* traverse backwards */
    it = ListItrPrev(ListItrEnd(list));
    expected = 4;
    while (it != ListItrBegin(list))
    {
        assert(*(int*)ListItrGet(it) == expected--);
        it = ListItrPrev(it);
    }
    assert(*(int*)ListItrGet(it) == 1);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_itr_get_set(void)
{
    printf("test_itr_get_set\n");

    List* list = ListCreate();
    int a = 7, b = 99;

    ListPushHead(list, &a);
    ListItr it = ListItrBegin(list);
    assert(*(int*)ListItrGet(it) == 7);

    int* old = (int*)ListItrSet(it, &b);
    assert(*old == 7);
    assert(*(int*)ListItrGet(it) == 99);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_insert_before_remove(void)
{
    printf("test_insert_before_remove\n");

    List* list = ListCreate();
    int a = 1, b = 3, mid = 2;

    ListPushTail(list, &a);
    ListPushTail(list, &b);

    /* insert 2 before the node holding 3 */
    ListItr it = ListItrNext(ListItrBegin(list));   /* points at 3 */
    ListItrInsertBefore(it, &mid);
    assert(ListSize(list) == 3);

    int expected[] = {1, 2, 3};
    int idx = 0;
    for (ListItr i = ListItrBegin(list); i != ListItrEnd(list); i = ListItrNext(i))
    {
        assert(*(int*)ListItrGet(i) == expected[idx++]);
    }

    /* remove the middle node (holds 2) */
    it = ListItrNext(ListItrBegin(list));
    int* removed = (int*)ListItrRemove(it);
    assert(*removed == 2);
    assert(ListSize(list) == 2);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_foreach_full(void)
{
    printf("test_foreach_full\n");

    List* list = ListCreate();
    int vals[4] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++) { ListPushTail(list, &vals[i]); }

    int total = 0;
    ListItr stopped = ListItrForEach(ListItrBegin(list), ListItrEnd(list), sum_action, &total);
    assert(stopped == ListItrEnd(list));
    assert(total == 10);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_foreach_early_stop(void)
{
    printf("test_foreach_early_stop\n");

    List* list = ListCreate();
    int vals[5] = {1, 5, 2, 3, 4};

    for (int i = 0; i < 5; i++) { ListPushTail(list, &vals[i]); }

    /* stop_at_5 returns 0 when it sees 5 */
    ListItr stopped = ListItrForEach(ListItrBegin(list), ListItrEnd(list), stop_at_5, NULL);
    assert(*(int*)ListItrGet(stopped) == 5);

    ListDestroy(&list, NULL);
    printf("  PASS\n");
}

static void test_null_safety(void)
{
    printf("test_null_safety\n");

    assert(ListSize(NULL) == 0);
    assert(ListIsEmpty(NULL));
    assert(ListPushHead(NULL, NULL) == NULL);
    assert(ListPushTail(NULL, NULL) == NULL);
    assert(ListPopHead(NULL) == NULL);
    assert(ListPopTail(NULL) == NULL);
    assert(ListItrBegin(NULL) == NULL);
    assert(ListItrEnd(NULL) == NULL);
    assert(ListItrNext(NULL) == NULL);
    assert(ListItrPrev(NULL) == NULL);
    assert(ListItrGet(NULL) == NULL);
    assert(ListItrSet(NULL, NULL) == NULL);
    assert(ListItrInsertBefore(NULL, NULL) == NULL);
    assert(ListItrRemove(NULL) == NULL);

    printf("  PASS\n");
}

int main(void)
{
    printf("=== gen_dlist tests ===\n");
    test_create_destroy();
    test_push_pop_head();
    test_push_pop_tail();
    test_iterators();
    test_itr_get_set();
    test_insert_before_remove();
    test_foreach_full();
    test_foreach_early_stop();
    test_null_safety();
    test_list_find();
    printf("=== ALL PASSED ===\n");
    return 0;
}
