#include<stdio.h>
#include<malloc.h>
typedef struct maze{
    int* maze;
    int row;
    int column;
}maze, *pMaze;
typedef struct queueNode{
    void *data;
    struct queueNode * last;
    struct queueNode * next;
}queueNode, *pQueueNode;
typedef struct queue{
    pQueueNode head;
    pQueueNode tail;
    int (*enQueue)(struct queue *self, void* data);
    void* (*outQueue)(struct queue *self);
    void (*freeQueue)(struct queue *self);
}queue, *pQueue;

int EAST = 1;
int SOUTH = 1<<1;
int WEST = 1<<2;
int NORTH = 1<<3;
int WALK = 16;
int START = 17;
int END = 18;

char* show[] = {
    " ", "→", "↓", "↘",
    "←", "↔", "↙", "",
    "↑", "↗", "↕", "",
    "↖","","","",
    "■","S","E"
};

/**
 * 入队
 * self:自身
 * data:入队的数据
 * return：是否失败
 * */
int _enQueue(pQueue self, void* data){
    if (NULL == self) {
        return 1;
    }
    pQueueNode node = (pQueueNode)malloc(sizeof(queueNode));
    node->data = data;
    node->last = self->tail;
    node->next = NULL;
    if (NULL != self->tail) {
        self->tail->next = node;
    }
    self->tail = node;
    if (NULL == self->head) {
        self->head = node;
    }
    return 0;
}

/**
 * 出队
 * self:自身
 * return:出队的值，为NULL表示失败;
 * */
void* _outQueue(pQueue self){
    if (NULL == self || NULL == self->head) {
        return NULL;
    }
    pQueueNode node = self->head;
    void* data = node->data;
    self->head = self->head->next;
    free(node);
    return data;
}

/**
 * 删除队列
 * self:自身
 * */
void _freeQueue(pQueue self){
    while(NULL != self->outQueue);
    free(self);
}

/**
 * 初始化队列
 * */
pQueue initQueue(){
    pQueue q = (pQueue)malloc(sizeof(queue));
    q->head = NULL;
    q->tail = NULL;
    q->enQueue = _enQueue;
    q->outQueue = _outQueue;
    q->freeQueue = _freeQueue;
    return q;
}

/**
 * 创建迷宫
 * */
pMaze initMaze(int row, int column){
    pMaze maze = (pMaze)malloc(sizeof(maze));
    maze->row = row;
    maze->column = column;
    int size = row*column;
    int* m = (int*)malloc(sizeof(int)*size);
    for(int i = 0; i < size; i++)
    {
        m[i]=WALK;
    }
    maze->maze = m;
    return maze;
}

/**
 * 主方法
 * */
int main(){
    
    return 0;
}