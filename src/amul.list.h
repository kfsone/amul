#ifndef AMUL_LIST_H
#define AMUL_LIST_H

struct SingleLinkedNode {
    struct SingleLinkedNode *next;
};

struct DoubleLinkedNode {
    struct DoubleLinkedNode *next;
    struct DoubleLinkedNode *prev;
};

#endif
