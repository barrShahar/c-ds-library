#include "bin_tree.h"
#include <stdlib.h>

/*
 havnt been tested enough yet
*/
typedef struct Node
{
    struct Node* m_papa;
    struct Node* m_left;
    struct Node* m_right;
    void* m_data;
} Node;

struct BSTree
{
    Node sentinel;
    int (*comparator)(void* _left, void* _right);
};

// Helpers
static void BSTreeDestroyPostOrder(Node* _head, void (*_destroyer)(void*));
static Node* BSTreeCreateNewNode(Node* _papa, void* _item, Node* _sentinel);
static BSTreeItr BSTreeMostLeftSon(BSTreeItr _it);
static BSTreeItr BSTreeMostRightSon(BSTreeItr _it);
static void BSTreeSeparateAndReplace(Node* _papa, Node* _son, Node* replacement);
static void SwapData(Node* _lVal, Node* _rVal);
static void* BSTreeItrRemoveRec(BSTreeItr _it);
static BSTreeItr BSTreeForEachPre(Node* _head, ActionFunction _action, void* _context);
static BSTreeItr BSTreeForEachInOrder(Node* _head, ActionFunction _action, void* _context);
static BSTreeItr BSTreeForEachPost(Node* _head, ActionFunction _action, void* _context);
// static BSTreeItr BSTreeInsertRec(Node* _head, void* _item, int (*comparator)(void* _left, void* _right));

// Macros
#define IS_LEAF(node) ((node)->m_left == (node)->m_right)
#define IS_SENTINEL(node) ((node)->m_data == NULL)
#define BST_ROOT(tree) ((tree)->sentinel.m_left)
#define BST_END(tree) ((tree)->sentinel.m_papa)
#define MAX_NUMBER(a,b) (((a)>(b))?(a):(b))


BSTree* BSTreeCreate(Comparator _less)
{
    if (_less == NULL) { return NULL; }
    BSTree* bstTree = malloc(sizeof(BSTree));
    if (bstTree == NULL) { return NULL; }

    // Initial sentinel sentinel
    Node* this = &bstTree->sentinel;
    bstTree->sentinel.m_papa = this;
    bstTree->sentinel.m_left = this;
    bstTree->sentinel.m_right = this;
    bstTree->sentinel.m_data = NULL;

    bstTree->comparator = _less;
    return bstTree;
}

void  BSTreeDestroy(BSTree** _tree, void (*_destroyer)(void*))
{
    if (_tree == NULL || *_tree == NULL) { return; }
    BSTree* tree = *_tree;
    BSTreeDestroyPostOrder(BST_ROOT(tree), _destroyer);

    free(tree);
    _tree = NULL;

    return;
}

BSTreeItr BSTreeInsert(BSTree* _tree, void* _item)
{
    if (_tree == NULL || _item == NULL) { return NULL; }

    Node* sentinel = BST_END(_tree);
    Node* papa = sentinel;
    Node* curr = sentinel->m_left;
    Comparator compare = _tree->comparator;

    while (curr != sentinel)
    {
        void* currData = curr->m_data;
        papa = curr;

        // No duplicates keys are allowed
        if (_tree->comparator(_item, currData) == 0)
        {
            return NULL;
        }

        curr = (compare(_item, currData) < 0) ? curr->m_left : curr->m_right;
    }

    Node* newNode = BSTreeCreateNewNode(papa, _item, sentinel);
    if (newNode == NULL) { return NULL; }

    if (papa == sentinel || compare(_item, papa->m_data) < 0)
    {
       papa->m_left = newNode;
    }
    else
    {
        papa->m_right = newNode;
    }

    return newNode;
}

BSTreeItr BSTreeItrBegin(const BSTree* _tree)
{
    if (_tree == NULL) { return NULL; }
    return BSTreeMostLeftSon(BST_ROOT(_tree));
}

BSTreeItr BSTreeItrEnd(const BSTree* _tree)
{
    if (_tree == NULL) { return NULL; }
    return BST_END(_tree);
}

BSTreeItr BSTreeItrNext(BSTreeItr _it)
{
    if (_it == NULL) { return NULL; }

    Node* it = (Node*) _it; // Alias

    if (IS_SENTINEL(it)) { return BSTreeMostLeftSon(it->m_left); }

    // If there is a right son, return the most left subtree of that node as next
    if (!IS_SENTINEL(it->m_right))
    {
        return BSTreeMostLeftSon(it->m_right);
    }

    // No right child: climb until we arrive from the left
    Node* curr = it;
    Node* papa = curr->m_papa;

    while (!IS_SENTINEL(papa))
    {
        if (papa->m_left == curr)
        {
            return papa;
        }

        curr = papa;
        papa = papa->m_papa;
    }

    return papa; // No next, return sentinel
}

BSTreeItr BSTreeItrPrev(BSTreeItr _it)
{
    if (_it == NULL) { return NULL; }

    Node* it = (Node*) _it; // Alias

    if (IS_SENTINEL(it)) { return BSTreeMostRightSon(it->m_left); }

    // If there is a left son, return the most right subtree of that node as next
    if (!IS_SENTINEL(it->m_left))
    {
        BSTreeMostRightSon(it);
    }

    // No left child: climb until we arrive from the right
    Node* curr = it;
    Node* papa = curr->m_papa;

    while (!IS_SENTINEL(papa))
    {
        if (papa->m_right == curr)
        {
            return papa;
        }

        curr = papa;
        papa = papa->m_papa;
    }

    return papa; // No next, return sentinel
}

int BSTreeGetHeight(Node* _head)
{
    if (IS_SENTINEL(_head))
    {
        return -1;
    }

    return 1 + MAX_NUMBER(BSTreeGetHeight(_head->m_left), BSTreeGetHeight(_head->m_right));
}

void* BSTreeItrRemove(BSTreeItr _it)
{
    if (_it == NULL || IS_SENTINEL((Node*)_it)) { return NULL; }

    return BSTreeItrRemoveRec(_it);
}

void* BSTreeItrGet(BSTreeItr _it)
{
    Node* it = (Node*)_it;
    return it ? it->m_data : NULL;
}

BSTreeItr BSTreeForEach(const BSTree* _tree, BSTreeTraversalMode _mode, ActionFunction _action, void* _context)
{
    if (_tree == NULL || _action == NULL) { return NULL; }

    switch (_mode)
    {
    case BSTREE_TRAVERSAL_PREORDER:
        return BSTreeForEachPre(BST_ROOT(_tree), _action, _context);

    case BSTREE_TRAVERSAL_INORDER:
        return BSTreeForEachInOrder(BST_ROOT(_tree), _action, _context);

    case BSTREE_TRAVERSAL_POSTORDER:
        return BSTreeForEachPost(BST_ROOT(_tree), _action, _context);

    default:
        return NULL;
    }
}


/* ---------- Static functions ---------- */

static void BSTreeDestroyPostOrder(Node* _head, void (*_destroyer)(void*))
{
    if (IS_SENTINEL(_head)) { return; }

    BSTreeDestroyPostOrder(_head->m_left, _destroyer);
    BSTreeDestroyPostOrder(_head->m_right, _destroyer);

    void* data = _head->m_data;
    if (_destroyer)
    {
        _destroyer(data);
    }

    free(_head);
    return;
}

static Node* BSTreeCreateNewNode(Node* _papa, void* _item, Node* _sentinel)
{
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) { return NULL; }

    newNode->m_papa = _papa;
    newNode->m_left = _sentinel;
    newNode->m_right = _sentinel;
    newNode->m_data = _item;

    return newNode;
}

static BSTreeItr BSTreeMostLeftSon(BSTreeItr _it)
{
    Node* curr = (Node*) _it; // Alias
    Node* next = curr->m_left;

    while (!IS_SENTINEL(next))
    {
        curr = next;
        next = next->m_left;
    }

    return curr;
}

static BSTreeItr BSTreeMostRightSon(BSTreeItr _it)
{
    Node* curr = (Node*) _it; // Alias
    Node* next = curr->m_right;

    while (!IS_SENTINEL(next))
    {
        curr = next;
        next = next->m_right;
    }

    return curr;
}

static void BSTreeSeparateAndReplace(Node* _papa, Node* _son, Node* replacement)
{
    if (_papa->m_left == _son)
    {
        _papa->m_left = replacement;
    }
    else
    {
        _papa->m_right = replacement;
    }
}

static void SwapData(Node* _lVal, Node* _rVal)
{
    void* tmp = _lVal->m_data;
    _lVal->m_data = _rVal->m_data;
    _rVal->m_data = tmp;
}

static BSTreeItr BSTreeForEachPre(Node* _head, ActionFunction _action, void* _context)
{
    if (IS_SENTINEL(_head)) { return _head; }

    BSTreeItr retVal;

    if (!_action(_head->m_data, _context)) { return _head; }
    retVal = BSTreeForEachPre(_head->m_left, _action, _context);
    retVal = BSTreeForEachPre(_head->m_right, _action, _context);

    return retVal;
}

static BSTreeItr BSTreeForEachInOrder(Node* _head, ActionFunction _action, void* _context)
{
    if (IS_SENTINEL(_head)) { return _head; }

    BSTreeItr retVal;

    retVal = BSTreeForEachPre(_head->m_left, _action, _context);
    if (!_action(_head->m_data, _context)) { return _head; }
    retVal = BSTreeForEachPre(_head->m_right, _action, _context);
    return retVal;
}

static BSTreeItr BSTreeForEachPost(Node* _head, ActionFunction _action, void* _context)
{
    if (IS_SENTINEL(_head)) { return _head; }

    BSTreeItr retVal;

    retVal = BSTreeForEachPre(_head->m_left, _action, _context);
    retVal = BSTreeForEachPre(_head->m_right, _action, _context);
    if (!_action(_head->m_data, _context)) { return _head; }

    return retVal;
}

static void* BSTreeItrRemoveRec(BSTreeItr _it)
{
    Node* it = (Node*)_it; // Alias

    if (IS_LEAF(it))
    {
        Node* papa = it->m_papa;
        Node* sentinel = it->m_left; // Get sentinel from the leaf

        BSTreeSeparateAndReplace(papa, it, sentinel);
        
        void* data = it->m_data;
        free(it);
        return data;
    }

    if (BSTreeGetHeight(it->m_left) < BSTreeGetHeight(it->m_right))
    {
        SwapData(it, it->m_right);
        return BSTreeItrRemove(it->m_right);
    }
    else
    {
        SwapData(it, it->m_left);
        return BSTreeItrRemove(it->m_left);
    }
}
