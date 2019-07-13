#ifndef AMUL_H_AMUL_LIST_H
#define AMUL_H_AMUL_LIST_H

struct SingleLinkedNode {
    struct SingleLinkedNode *next;
};

struct DoubleLinkedNode {
    struct DoubleLinkedNode *next;
    struct DoubleLinkedNode *prev;
};

#endif