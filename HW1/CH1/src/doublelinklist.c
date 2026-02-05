#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *prev;
    struct Node *next;
} Node;

Node *create_node(const char *str) {
    Node *node = malloc(sizeof(Node));
    if (!node) return NULL;

    node->data = malloc(strlen(str) + 1);
    if (!node->data) {
        free(node);
        return NULL;
    }

    strcpy(node->data, str);
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void insert(Node **head, const char *str) {
    Node *new_node = create_node(str);
    if (!new_node) return;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = new_node;
    new_node->prev = curr;
}

Node *find(Node *head, const char *str) {
    while (head != NULL) {
        if (strcmp(head->data, str) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

int delete(Node **head, const char *str) {
    Node *target = find(*head, str);
    if (!target) {
        return 0;  //not found
    }

    if (target->prev) {
        target->prev->next = target->next;
    } else {
        //deleting the head
        *head = target->next;
    }

    if (target->next) {
        target->next->prev = target->prev;
    }

    free(target->data);
    free(target);
    return 1;
}

void print_list(Node *head) {
    printf("List: ");
    while (head != NULL) {
        printf("\"%s\" ", head->data);
        head = head->next;
    }
    printf("\n");
}

void print_list_reverse(Node *head) {
    if (!head) {
        printf("Reverse: (empty)\n");
        return;
    }

    // go to tail
    Node *tail = head;
    while (tail->next) {
        tail = tail->next;
    }

    printf("Reverse: ");
    while (tail) {
        printf("\"%s\" ", tail->data);
        tail = tail->prev;
    }
    printf("\n");
}

void free_list(Node *head) {
    while (head != NULL) {
        Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

int main(void) {
    Node *list = NULL;

    insert(&list, "apple");
    insert(&list, "banana");
    insert(&list, "cherry");

    print_list(list);
    print_list_reverse(list);

    delete(&list, "banana");

    print_list(list);
    print_list_reverse(list);

    free_list(list);
    return 0;
}

