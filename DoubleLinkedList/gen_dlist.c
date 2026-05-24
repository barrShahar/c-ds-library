#include "gen_dlist.h"
#include <stdlib.h>

typedef struct Node
{
    struct Node* m_next;
    struct Node* m_prev;
    void* m_data;
} Node;

struct List
{
    Node m_head;
    Node m_tail;
};

// Macros 
#define LIST_BEGIN(list) ((list)->m_head.m_next)
#define LIST_END(list) ((list)->m_tail.m_next)
#define LIST_IS_EMPTY(list) (LIST_BEGIN(list) == LIST_END(list))
#define LIST_ITR_IS_TAIL(node) ((node) == (node)->m_next)
#define LIST_ITR_IS_HEAD(node) ((node)->m_prev == (node)->m_prev->m_prev)

// Helper functions declarations 
static void ListConnectBetweenNodes(Node* _lValue, Node* _rValue);

List* ListCreate(void)
{
    List* list = malloc(sizeof(List));
    if (list == NULL)
    {
        return NULL;
    }

    // Sentinel head
    list->m_head.m_prev = &list->m_head;
    list->m_head.m_data = NULL;

    // Sentinel tail
    list->m_tail.m_next = &list->m_tail;
    list->m_tail.m_data = NULL;

    // Connect between sentinel head and sentinel tail
    ListConnectBetweenNodes(&list->m_head, &list->m_tail);

    return list;
}

void ListDestroy(List** _pList, void (*_elementDestroy)(void* _item))
{
    if (_pList == NULL || *_pList == NULL) { return; }
    List* list = *_pList;

    Node* it = LIST_BEGIN(list);
    while (it != LIST_END(list))
    {
        Node* next = it->m_next;
        if (_elementDestroy)
        {
            _elementDestroy(it->m_data);
        }
        free(it);
        it = next;
    }

    free(list);
    *_pList = NULL;

    return;
}

ListItr ListPushHead(List* _list, void* _item)
{
    if (_list == NULL || _item == NULL) { return NULL; }

    Node* newHead = malloc(sizeof(Node));
    if (!newHead) 
    {
        return NULL;
    }

    newHead->m_data = _item;
    ListConnectBetweenNodes(newHead, _list->m_head.m_next);
    ListConnectBetweenNodes(&_list->m_head, newHead);

    return (ListItr)newHead;
}

ListItr ListPushTail(List* _list, void* _item)
{
    if (_list == NULL || _item == NULL) { return NULL; }

    Node* toPushBack = malloc(sizeof(Node));
    if (!toPushBack) 
    {
        return NULL;
    }

    toPushBack->m_data = _item;
    ListConnectBetweenNodes(LIST_END(_list)->m_prev, toPushBack);
    ListConnectBetweenNodes(toPushBack, LIST_END(_list));

    return (ListItr)toPushBack;
}

void* ListPopHead(List* _list)
{
    if (_list == NULL || LIST_IS_EMPTY(_list)) { return NULL; }

    Node* toRemove = LIST_BEGIN(_list);
    void* poppedData = toRemove->m_data;
    ListConnectBetweenNodes(toRemove->m_prev, toRemove->m_next);

    free(toRemove);
    return poppedData;
}

void* ListPopTail(List* _list)
{
    if (_list == NULL || LIST_IS_EMPTY(_list)) { return NULL; }

    Node* toRemove = _list->m_tail.m_prev;
    void* poppedData = toRemove->m_data;
    ListConnectBetweenNodes(toRemove->m_prev, toRemove->m_next);

    free(toRemove);
    return poppedData;
}

ListItr ListItrBegin(const List* _list)
{
    return _list ? LIST_BEGIN(_list) : NULL;
}

ListItr ListItrEnd(const List* _list)
{
    return _list ? LIST_END(_list) : NULL;
}

ListItr ListItrNext(ListItr _itr)
{
    Node* node = (Node*)_itr;
    return node ? (ListItr)node->m_next : NULL;
}

ListItr ListItrPrev(ListItr _itr)
{
    if (_itr == NULL) { return NULL; }
    Node* node = (Node*)_itr;
    return LIST_ITR_IS_HEAD(node) ? (ListItr)node : (ListItr)node->m_prev;
}

void* ListItrGet(ListItr _itr)
{
    Node* node = (Node*)_itr;
    return node ? node->m_data : NULL; // tail->data is NULL
}

void* ListItrSet(ListItr _itr, void* _element)
{
    if (_itr == NULL || _element == NULL || LIST_ITR_IS_TAIL((Node*)_itr)) { return NULL; }
    Node* node = (Node*)_itr;
    void* oldData = node->m_data;

    node->m_data = _element;
    return oldData;
}

ListItr ListItrInsertBefore(ListItr _itr, void* _element)
{
    if (_itr == NULL || _element == NULL) { return NULL; }

    Node* newNodeToInsert = malloc(sizeof(Node));
    if (newNodeToInsert == NULL)
    {
        return NULL;
    }
    newNodeToInsert->m_data = _element;

    Node* insertBeforePos = (Node*)_itr;
    Node* nodeBefore = insertBeforePos->m_prev;

    ListConnectBetweenNodes(nodeBefore, newNodeToInsert);
    ListConnectBetweenNodes(newNodeToInsert, insertBeforePos);

    return (ListItr)newNodeToInsert;
}

void* ListItrRemove(ListItr _itr)
{
    if (_itr == NULL || LIST_ITR_IS_TAIL((Node*)_itr)) { return NULL; }
    Node* node = (Node*)_itr;

    void* data = node->m_data;

    node->m_prev->m_next = node->m_next;
    node->m_next->m_prev = node->m_prev;

    free(node);
    return data;
}

size_t ListSize(const List* _list)
{
    if (!_list) { return 0; }

    size_t listSize = 0;
    for (Node* it = LIST_BEGIN(_list) ; it != LIST_END(_list) ; it = it->m_next, ++listSize);

    return listSize;
}

size_t ListIsEmpty(List* _list)
{
    return _list ? LIST_BEGIN(_list) == LIST_END(_list) : 1;    
}

ListItr ListItrForEach(ListItr _begin, ListItr _end, ListActionFunction _action, void* _context)
{
    if (_begin == NULL || _end == NULL || _action == NULL) { return NULL; }
    Node* it = (Node*)_begin;
    for ( ; it != (Node*)_end ; it = it->m_next)
    {
        if (!_action(it->m_data, _context))
        {
            break;
        }
    }

    return it;
}

// Statics

static void ListConnectBetweenNodes(Node* _lValue, Node* _rValue)
{
    _lValue->m_next = _rValue;
    _rValue->m_prev = _lValue;
}

ListItr ListFind(List* _list, void* _target, int (*Compare)(void* _toFind, void* _currItem))
{
    if (_list == NULL || _target == NULL || Compare == NULL) { return NULL; }
    
    Node* end = LIST_END(_list);
    for (Node* it = LIST_BEGIN(_list) ; it != end ; it = it->m_next)
    {
        if (_target == it->m_data ||
            (Compare && Compare(_target, it->m_data)))
        {
            return it;
        }
    }

    return NULL;
}