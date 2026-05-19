#include "genqueue.h"
#include <stdlib.h>

struct Queue
{
    size_t m_capacity;
    size_t m_head;
    size_t m_tail;
    size_t m_numberOfItems;
    void **m_items;
};

#define RING_INC(number, base) (number) = ((number) + 1) % base

Queue *QueueCreate(size_t _size)
{
    if (_size == 0) { return NULL; }
    
    Queue *queue = malloc(sizeof(Queue));
    if (queue == NULL) { return NULL; }

    queue->m_capacity = _size;
    queue->m_numberOfItems = 0;
    queue->m_head = 0;
    queue->m_tail = 0;
    queue->m_items = malloc(_size * sizeof(void*));

    if (queue->m_items == NULL)
    {
        free(queue);
        return NULL;
    }

    return queue;
}


void QueueDestroy(Queue** _queue, DestroyItem _itemDestroy)
{
    if (_queue == NULL || *_queue == NULL) { return; }

    Queue *q = *_queue;
    if (_itemDestroy && !QueueIsEmpty(q))
    {
        size_t index = q->m_head;
        for (size_t i = 0 ; i < q->m_numberOfItems ; ++i)
        {
            _itemDestroy(q->m_items[index]);
            index = (index + 1) % q->m_capacity;
        }
        
    }

    free(q->m_items);
    free(q);
    *_queue = NULL;
}


QueueResult QueueInsert(Queue* _queue,void* _item)
{
    if (_queue == NULL) { return QUEUE_UNINITIALIZED_ERROR; }
    if (_item == NULL) { return QUEUE_NULL_PTR_ERROR; }
    if (_queue->m_numberOfItems == _queue->m_capacity) { return QUEUE_OVERFLOW_ERROR; }

    _queue->m_items[_queue->m_tail] = _item;
    RING_INC(_queue->m_tail, _queue->m_capacity);
    _queue->m_numberOfItems += 1;

    return QUEUE_SUCCESS;
}


QueueResult QueueRemove(Queue* _queue,void** _item)
{
    if (_queue == NULL) { return QUEUE_UNINITIALIZED_ERROR; }
    if (_item == NULL) { return QUEUE_NULL_PTR_ERROR; }
    if (_queue->m_numberOfItems == 0) { return QUEUE_SIZE_ZERO_ERROR; }

    *_item = _queue->m_items[_queue->m_head];
    RING_INC(_queue->m_head, _queue->m_capacity);
    _queue->m_numberOfItems -= 1;


    return QUEUE_SUCCESS;
}


size_t QueueIsEmpty(Queue* _queue)
{
    if (_queue == NULL) { return (size_t)-1; }
    if (_queue->m_numberOfItems == 0) { return 1; }
    return 0;
}



size_t QueueForEach(Queue* _queue, ActionFunction _action, void* _context)
{
    if (_queue == NULL || _action == NULL) { return 0; }
    if (_queue->m_numberOfItems == 0) { return 0; }

    size_t queueIndex = _queue->m_head;
    size_t i;
    for (i = 0 ; i < _queue->m_numberOfItems ; ++i)
    {
        int shouldContinue = _action(_queue->m_items[queueIndex], _context);
        RING_INC(queueIndex,  _queue->m_capacity);

        if (!shouldContinue)
        {
            ++i;
            break; 
        }
    }
    return i;
}

