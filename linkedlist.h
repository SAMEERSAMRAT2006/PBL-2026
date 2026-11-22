// linkedlist.h
// DSA: Singly linked list written from scratch (template class).
// OOPS: Templates, constructor/destructor, deleted copy operations.
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node *next;
        explicit Node(const T &d) : data(d), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    int count;

public:
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}

    ~LinkedList() { clear(); }

    // Copying a list would share raw pointers, so it is disabled.
    LinkedList(const LinkedList &) = delete;
    LinkedList &operator=(const LinkedList &) = delete;

    // Insert at the end in O(1) using the tail pointer.
    void push_back(const T &value) {
        Node *n = new Node(value);

        if (head == nullptr)
            head = tail = n;
        else {
            tail->next = n;
            tail = n;
        }

        count++;
    }

    int size() const { return count; }

    // Delete every node.
    void clear() {
        while (head != nullptr) {
            Node *temp = head;
            head = head->next;
            delete temp;
        }

        tail = nullptr;
        count = 0;
    }

    // Traverse from head to tail and call f on every element.
    template <typename Func>
    void forEach(Func f) const {
        for (Node *cur = head; cur != nullptr; cur = cur->next)
            f(cur->data);
    }
};

#endif
