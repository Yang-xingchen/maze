#include<stdio.h>
#include<malloc.h>
#include<time.h>
#include<stdlib.h>
/**
 * 迷宫
 * */
typedef struct maze{
    int* maze;
    unsigned int row;
    unsigned int column;
    unsigned int start;
    unsigned int end;
    void (*print)(struct maze *);
    void (*generate)(struct maze *, unsigned int);
    void (*save)(struct maze *, char *);
    void (*free)(struct maze *);
    void (*run)(struct maze *);
}maze, *pMaze;
/**
 * 数据类型
 * */
typedef union data
{
    void * Object;
    int Integer;
    double Double;
    long Long;
}Data,* pData;
/**
 * 节点
 * */
typedef struct node{
    pData d;
    struct node * last;
    struct node * next;
}node, *pNode;
/**
 * 链表
 * */
typedef struct list
{
    pNode head;
    int (*add)(struct list *, pData, unsigned int);
    pData (*remove)(struct list *,unsigned int);
    pData (*get)(struct list *, unsigned int);
    unsigned int (*size)(struct list *);
    void (*foreach)(struct list *, void (*consumer)(pData));
    int (*isNull)(struct list *);
    void (*free)(struct list *);
}list, *pList;
/**
 * 队列
 * */
typedef struct queue{
    pList list;
    pNode tail;
    int (*offer)(struct queue *, pData);
    pData (*peek)(struct queue *);
    int (*isNull)(struct queue *);
    void (*free)(struct queue *);
}queue, *pQueue;
/**
 * 堆栈
 * */
typedef struct stack
{
    pList list;
    pData (*pop)(struct stack*);
    void (*push)(struct stack *,pData);
    void (*free)(struct stack *);
}stack, *pStack;


int EAST = 1;
int SOUTH = 1<<1;
int WEST = 1<<2;
int NORTH = 1<<3;
int WALK = 16;
int ROAD = 0;
int START = 17;
int END = 18;

char* show[] = {
    "  ", "→ ", "↓ ", "↘ ",
    "← ", "↔ ", "↙ ", "err",
    "↑ ", "↗ ", "↕ ", "err",
    "↖ ","err","err","err",
    "⬛","S ","E "
};

/**
 * 添加元素
 * data:添加的数据
 * index:索引，从0开始
 * return：是否失败
 * */
int _list_add(pList self, pData data, unsigned int index){
    pNode node = (pNode)malloc(sizeof(node));
    node->d = data;
    if (0 == index) {
        node->last = NULL;
        node->next = self->head;
        self->head = node;
        return 0;
    }
    pNode p = self->head;
    if(NULL == p){
        return 1;
    }
    int i = 0;
    while(NULL != p->next){
        if (index == (++i)) {
            break;
        }
        p = p->next;
    }
    if (index != i) {
        return 1;
    }
    if (NULL != p->next) {
        p->next->last = node;
    }
    node->next = p->next;
    p->next = node;
    node->last = p;
    return 0;
}

/**
 * 删除元素
 * index:索引，从0开始
 * return:删除的值，为NULL表示失败;
 * */
pData _list_remove(pList self, unsigned int index){
    if (NULL == self || NULL == self->head) {
        return NULL;
    }
    if (0 == index) {
        pNode n = self->head;
        self->head = n->next;
        self->head->last = NULL;
        pData d = n->d;
        n->d = NULL;
        n->next = NULL;
        free(n);
        return d;
    }
    pNode p = self->head->next;
    int i = 0;
    while(NULL != p){
        if (index == (++i)) {
            pData d = p->d;
            if (NULL != p->next) {
                p->next->last = p->last;
            }
            p->last->next = p->next;
            p->last = NULL;
            p->next = NULL;
            p->d = NULL;
            return d;
        }
    }
    return NULL;
}

/**
 * 获取数据
 * index:索引，从0开始
 * return:该索引的值，为NULL表示越界
 * */
pData _list_get(pList list, unsigned int index){
    int i = 0;
    pNode p = list->head;
    while(NULL != p){
        if(index == (i++)){
            return p->d;
        }
        p=p->next;
    }
    return NULL;
}

/**
 * 获取链表节点的数量
 * return:节点的数量
 * */
unsigned int _list_size(pList self){
    unsigned int size = 0;
    pNode p = self->head;
    while(NULL != p){
        size++;
        p = p->next;
    }
    return size;
}

/**
 * 遍历链表
 * consumer:消费者函数，即遍历链表每个元素做的事
 * */
void _list_foreach(pList self, void (*consumer)(pData)){
    pNode n = self->head;
    while(NULL != n){
        consumer(n->d);
        n = n->next;
    }
}

/**
 * 链表是否为空
 * return:为空返回1,不为空返回0
 * */
int _list_isNull(pList self){
    return self->head == NULL;
}

/**
 * 删除链表
 * */
void _list_free(pList self){
    while(NULL != self->remove(self, 0));
    free(self);
}

/**
 * 初始化链表
 * return:初始化的链表
 * */
pList initList(){
    pList l = (pList)malloc(sizeof(list));
    l->head = NULL;
    l->add = _list_add;
    l->remove = _list_remove;
    l->get = _list_get;
    l->size = _list_size;
    l->foreach = _list_foreach;
    l->isNull = _list_isNull;
    l->free = _list_free;
    return l;
}

/**
 * 入队
 * d:入队的数据
 * */
int _queue_offer(pQueue self, pData d){
    self->list->add(self->list, d, 0);
    if (1 == self->list->size(self->list)) {
        self->tail = self->list->head;
    }
    return 0;
}

/**
 * 出队
 * return:出队的数据
 * */
pData _queue_peek(pQueue self){
    pNode node = self->list->head;
    pData data = node->d;
    node->d = NULL;
    self->list->head = self->list->head->next;
    if (NULL == self->list->head) {
        self->tail = NULL;
    }
    free(node);
    return data;}

/**
 * 队列是否为空
 * return:为空返回1,不为空返回0
 * */
int _queue_isNull(pQueue self){
    return self->list->isNull(self->list);
}

void _queue_free(pQueue self){
    self->list->free(self->list);
    free(self);
}

/**
 * 初始化队列
 * return:初始化的队列
 * */
pQueue initQueue(){
    pQueue q = (pQueue)malloc(sizeof(queue));
    q->list = initList();
    q->tail = NULL;
    q->offer = _queue_offer;
    q->peek = _queue_peek;
    q->isNull = _queue_isNull;
    q->free = _queue_free;
    return q;
}

/**
 * 压栈
 * data:数据
 * */
void _pushStack(pStack self, pData data){
    self->list->add(self->list, data, 0);
}

/**
 * 弹栈
 * return:弹出的数据
 * */
pData _popStack(pStack self){
    pNode n = self->list->head;
    if (NULL == n) {
        return NULL;
    }    
    self->list->head = n->next;
    self->list->head->last = NULL;
    pData d = n->d;
    n->d = NULL;
    n->last = NULL;
    n->next = NULL;
    free(n);
    return d;
}

/**
 * 删除堆栈
 * */
void _freeStack(pStack self){
    self->list->free(self->list);
    free(self);
}

/**
 * 初始化堆栈
 * */
pStack initStack(){
    pStack s = (pStack)malloc(sizeof(stack));
    s->list = initList();
    s->pop = _popStack;
    s->push = _pushStack;
    s->free = _freeStack;
    return s;
}

/**
 * 获取位置
 * row:行
 * column:列
 * return:位置代码
 * */
int _getPoint(pMaze self, int row, int column){
    return (row%self->row)*self->column+(column%self->column);
}

/**
 * 获取位置信息
 * row:行
 * column:列
 * return:该位置的代码，参考全局变量show
 * */
int _getByXY(pMaze self, int row, int column){
    return self->maze[_getPoint(self, row, column)];
}

/**
 * 输出
 * */
void _print(pMaze self){
    for(int i = 0; i < self->row; i++)
    {
        for(int j = 0; j < self->column; j++)
        {
            printf(show[_getByXY(self, i ,j)%19]);
        }
        printf("\n");
    }    
}

/**
 * 删除迷宫
 * */
void _freeMaze(pMaze self){
    free(self->maze);
    free(self);
}

/**
 * 移动
 * q:队列
 * r:行
 * c:列
 * direction:方向
 * opposite:反方向
 * condition:条件
 * */
void _moveMaze(pMaze self, pQueue q, 
                                int r, int c, 
                                int direction,int opposite, 
                                int condition){
    int point = _getPoint(self, r, c);
    if (condition                                                                                       //初始条件
        && 0 == (self->maze[point]&direction)                               //该方向没走过
        && 0 == (self->maze[point]&opposite)                                   //新位置不到两条路
        && WALK != self->maze[point]                                                    //不为墙
        && START != self->maze[point]                                                   //不为开始
        && END != self->maze[point]) {                                                  //不为结束
            self->maze[point] |= direction;
            pData d = (pData)malloc(sizeof(Data));
            d->Integer = point;
            q->offer(q, d);    
    }
}

/**
 * 移动
 * */
void _runMaze(pMaze self){
    pQueue q = initQueue();
    pData d = (pData)malloc(sizeof(Data));
    d->Integer = self->end;
    q->offer(q, d);
    while(!q->isNull(q)){
        d = q->peek(q);
        int point = d->Integer;
        free(d);
        int r = point / self->column;
        int c = point % self->column;
        _moveMaze(self, q, r-1, c, SOUTH, NORTH, r>0);
        _moveMaze(self, q, r, c-1, EAST, WEST, c>0);
        _moveMaze(self, q, r+1, c, NORTH, SOUTH, r<(self->row-1));
        _moveMaze(self, q, r, c+1, WEST, EAST, c<(self->column-1));
    }    
}

/**
 * 生成路径
 * seed:种子
 * */
void _generateRoad(pMaze self, unsigned int seed){
    int size = self->column*self->row;
    int r = (self->row>>1)+(self->row>>2)+(self->row>>4);
    int c = (self->column>>1)+(self->column>>2)+(self->column>>4);
    srand(seed);
    while(r>0 && c>0) {
        if (r>0) {
            int row;
            int startColumn;
            do
            {
                row = rand()%self->row;
                startColumn = rand()%self->column;
            } while (ROAD==_getByXY(self, row+1, startColumn) 
                || ROAD == _getByXY(self, row>0?row:(self->row-1), startColumn));            
            int l = rand()%(self->column>>1)+(self->column>>2);
            for(int j = 0; j < l; j++)
            {
                int stop = 0;
                int point = _getPoint(self, row, startColumn+j);
                while(ROAD == self->maze[point]){
                    if (!rand()&3) {
                        break;
                    }
                    if (!rand()&3) {
                        stop = 1;
                    }                    
                    row = rand()%self->row;
                    startColumn = rand()%self->column;
                    point = _getPoint(self, row, startColumn+j);
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
            } while (ROAD == _getByXY(self, startRow, column+1) 
                || ROAD == _getByXY(self, startRow, column>0?column:(self->column-1)));
            int l = rand()%(self->row>>1)+self->row>>2;
            for(int j = 0; j < l; j++)
            {
                int stop = 0;
                int point = _getPoint(self, startRow+j, column);
                while(ROAD == self->maze[point]){
                    if (!rand()&3) {
                        break;
                    }
                    if (!rand()&3) {
                        stop = 1;
                    }                    
                    column = rand()%self->column;
                    startRow = rand()%self->row;
                    point = _getPoint(self, startRow+j, column);
                }            
                self->maze[point] = ROAD;
            }
            c--;  
        }      
    }
    while(1){
        int r = rand()%self->row;
        int c = rand()%self->column;
        if (ROAD == _getByXY(self, r, c)) {
            self->maze[_getPoint(self, r, c)] = START;
            self->start = _getPoint(self, r, c);
            break;
        }        
    }    
    while(1){
        int r = rand()%self->row;
        int c = rand()%self->column;
        if (ROAD == _getByXY(self, r, c)) {
            self->maze[_getPoint(self, r, c)] = END;
            self->end = _getPoint(self, r, c);
            break;
        }        
    }    
}

/**
 * 保存到文件
 * fileName:要保存的文件名
 * */
void _saveToFile(pMaze self, char* fileName){
    FILE *file = fopen(fileName, "w");
    fprintf(file, "%d %d\n", self->row, self->column);
    int size = self->row * self->column;
    for(int i = 0; i < size; i++)
    {
        fprintf(file, "%d ", self->maze[i]);
    }
    fclose(file);
}

/**
 * 通过文件创建迷宫
 * fileName:文件名
 * return:初始化的迷宫
 * */
pMaze initMazeByFile(char *fileName){
    FILE *file = fopen(fileName,"r");
    pMaze ret = (pMaze)malloc(sizeof(maze));
    fscanf(file, "%d %d\n",&(ret->row), &(ret->column));
    int size = ret->row*ret->column;
    int *m = (int*)malloc(sizeof(int)*size);
    for(int i = 0; i < size; i++)
    {
        fscanf(file, "%d", &(m[i]));
        if (START == m[i]) {
            ret->start = i;
        }else if (END == m[i]) {
            ret->end = i;
        }
    }
    fclose(file);
    ret->maze = m;
    ret->print = _print;
    ret->generate = _generateRoad;
    ret->save = _saveToFile;
    ret->free = _freeMaze;
    ret->run = _runMaze;
    return ret;
}

/**
 * 创建迷宫
 * row:行数
 * column:列数
 * return:初始化的迷宫
 * */
pMaze initMaze(unsigned int row, unsigned int column){
    pMaze ret = (pMaze)malloc(sizeof(maze));
    ret->row = row;
    ret->column = column;
    int size = row*column;
    int* m = (int*)malloc(sizeof(int)*size);
    for(int i = 0; i < size; i++)
    {
        m[i]=WALK;
    }
    ret->start = -1;
    ret->end = -1;
    ret->maze = m;
    ret->print = _print;
    ret->generate = _generateRoad;
    ret->save = _saveToFile;
    ret->free = _freeMaze;
    ret->run = _runMaze;
    return ret;
}

/**
 * 主方法
 * */
int main(){
    pMaze maze = initMaze(20, 40);
    maze->generate(maze, time(NULL));
    maze->print(maze);
    maze->run(maze);
    printf("--------------------------------------------------------------------------------\n");
    maze->print(maze);
    return 0;
}