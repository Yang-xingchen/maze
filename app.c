#include<stdio.h>
#include<malloc.h>
#include<time.h>
#include<stdlib.h>
/**
 * 迷宫
 * */
typedef struct maze{
    int* maze;
    int row;
    int column;
    void (*print)(struct maze *);
    void (*generate)(struct maze *, int);
}maze, *pMaze;
/**
 * 队列节点
 * */
typedef struct queueNode{
    void *data;
    struct queueNode * last;
    struct queueNode * next;
}queueNode, *pQueueNode;
/**
 * 队列
 * */
typedef struct queue{
    pQueueNode head;
    pQueueNode tail;
    int (*enQueue)(struct queue *, void *);
    void* (*outQueue)(struct queue *);
    void (*freeQueue)(struct queue *);
}queue, *pQueue;

int EAST = 1;
int SOUTH = 1<<1;
int WEST = 1<<2;
int NORTH = 1<<3;
int WALK = 16;
int ROAD = 0;
int START = 17;
int END = 18;

char* show[] = {
    "  ", "→", "↓", "↘",
    "←", "↔", "↙", "",
    "↑", "↗", "↕", "",
    "↖","","","",
    "⬛","S","E"
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
 * 获取位置信息
 * self:自身
 * row:行
 * column:列
 * return:该位置的代码，参考全局变量show
 * */
int _getByXY(pMaze self, int row, int column){
    return self->maze[row*self->column+column];
}

/**
 * 输出
 * self:自身
 * */
void _print(pMaze self){
    for(int i = 0; i < self->row; i++)
    {
        for(int j = 0; j < self->column; j++)
        {
            printf(show[_getByXY(self, i ,j)]);
        }
        printf("\n");
    }    
}

/**
 * 生成路径
 * */
void _generateRoad(pMaze self, int seed){
    int size = self->column*self->row;
    int r = (self->row>>1)+(self->row>>2)+(self->row>>4);
    int c = (self->column>>1)+(self->column>>2)+(self->column>>4);
    srand(seed);
    while(r>0 && c>0)
    {
        if (r>0) {
            int row;
            int startColumn;
            do
            {
                row = rand()%self->row;
                startColumn = rand()%self->column;
            } while (ROAD==self->maze[((row+1)%self->row)*self->column + startColumn%self->column]);            
            int l = rand()%(self->column>>1)+(self->column>>2);
            for(int j = 0; j < l; j++)
            {
                int stop = 0;
                int point = row*self->column + (startColumn+j)%self->column;
                while(ROAD == self->maze[point]){
                    if (!rand()&2) {
                        break;
                    }
                    if (!rand()&2) {
                        stop = 1;
                    }                    
                    row = rand()%self->row;
                    startColumn = rand()%self->column;
                    point = row*self->column + (startColumn+j)%self->column;
                }            
                self->maze[point] = ROAD;
            }
            r--;
        }
        if (c>0) {
            int column;
            int startRow;
            do
            {
                column = rand()%self->column;
                startRow = rand()%self->row;
            } while (ROAD == self->maze[(startRow%self->row)*self->column + column+1]);
            int l = rand()%(self->row>>1)+self->row>>2;
            for(int j = 0; j < l; j++)
            {
                int stop = 0;
                int point = ((startRow+j)%self->row)*self->column + column;
                while(ROAD == self->maze[point]){
                    if (!rand()&2) {
                        break;
                    }
                    if (!rand()&2) {
                        stop = 1;
                    }                    
                    column = rand()%self->column;
                    startRow = rand()%self->row;
                    point = ((startRow+j)%self->row)*self->column + column;
                }            
                self->maze[point] = ROAD;
            }
            c--;  
        }      
    }
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
    maze->print = _print;
    maze->generate = _generateRoad;
    return maze;
}

/**
 * 主方法
 * */
int main(){
    pMaze maze = initMaze(20, 35);
    maze->generate(maze, time(NULL));
    maze->print(maze);
    return 0;
}