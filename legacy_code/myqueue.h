//
// Created by 陆清 on 3/31/20.
//

#ifndef H4D1_MYQUEUE_H
#define H4D1_MYQUEUE_H
#include <stdlib.h>

struct Node{
    struct Node * next=NULL;
    int client_fd;
};
typedef struct Node node_t;

node_t *head=NULL;
node_t *tail=NULL;

void enqueue(int client_fd) {
    node_t * newNode = (node_t *)malloc(sizeof(node_t));
    newNode->client_fd=client_fd;
    newNode->next=NULL;
    if(tail==NULL){
        head=newNode;
    }else{
        tail->next=newNode;
    }
    tail=newNode;
}

int dequeue() {
    if(head==NULL){
        return 0;
    }else{
        int result=head->client_fd;
        Node * tmp=head;
        head=head->next;
        if(head==NULL){
            tail=NULL;
        }
        free(tmp);
        return result;
    }
}

#endif //H4D1_MYQUEUE_H
