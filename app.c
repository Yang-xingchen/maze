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
typedef struct data {
    union {
        void * Object;
        int Integer;
        double Double;
        long Long;
    } data;
    enum {
        OBJECT,
        INTEGER,
        DOUBLE,
        LONG,
    } type;
}Data,* pData;
/**
 * 节点
 * */
typedef struct node {
    pData d;
    struct node * last;
    struct node * next;
}node, *pNode;
/**
 * 链表
 * */
typedef struct list {
    pNode head;
    unsigned int size;
    int (*add)(struct list *, pData, unsigned int);
    pData (*remove)(struct list *,unsigned int);
    pData (*get)(struct list *, unsigned int);
    void (*foreach)(struct list *, void (*consumer)(pData));
    int (*isNull)(struct list *);
    void (*free)(struct list *, void (*freeData)(pData));
}list, *pList;
/**
 * 队列
 * */
typedef struct queue {
    pList list;
    pNode tail;
    int (*offer)(struct queue *, pData);
    pData (*peek)(struct queue *);
    int (*isNull)(struct queue *);
    void (*free)(struct queue *, void (*freeData)(pData));
}queue, *pQueue;
/**
 * 堆栈
 * */
typedef struct stack {
    pList list;
    pData (*pop)(struct stack*);
    void (*push)(struct stack *,pData);
    void (*free)(struct stack *, void (*freeData)(pData));
}stack, *pStack;
/**
 * 迷宫节点，辅助走迷宫
 * */
typedef struct mazeNode {
    int end;
    pList points;
    pList link_node;
    pList in;
    pList out;
}mazeNode, *pMazeNode;

//显示移动
#define EAST            0x001
#define SOUTH           0x002
#define WEST            0x004
#define NORTH           0x008

//标记通路
#define EASE_ROAD       0x100
#define SOUTH_ROAD      0x200
#define WEST_ROAD       0x400
#define NORTH_ROAD      0x800

//其他显示
#define ROAD            0x000
#define WALK            0x010
#define START           0x011
#define END             0x012

//其他标记
#define FLAG            0x020
#define BLOCK_FLAG      0x040
#define BOUNDARY_FLAG   0x080


//method
#define ADD_BIT(bitset, bit)    (bitset |= bit)
#define REMOVE_BIT(bitset, bit) (bitset &= ~bit)
#define HAS_BIT(bitset, bit)    (bitset &  bit)

char* show[] = {
    "  ", "→ ", "↓ ", "↘ ",
    "← ", "↔ ", "↙ ", "er",
    "↑ ", "↗ ", "↕ ", "er",
    "↖ ", "er", "er", "er",
    "⬛", "S ", "E "
};

/**
 * 基础数据释放
 * data:释放的Data
 * */
void __Data_free(pData data){
    free(data);
}

/**
 * 包装Data
 * integer:包装的int值
 * return:创建的Data
 * */
pData toDataByInt(int integer){
    pData d = (pData)malloc(sizeof(Data));
    d->data.Integer = integer;
    d->type = INTEGER;
    return d;
}

/**
 * 包装一般对象
 * object:包装的对象
 * return:创建的Data
 * */
pData toDataByObject(void* object){
    pData d = (pData)malloc(sizeof(Data));
    d->data.Object = object;
    d->type = OBJECT;
    return d;
}

/**
 * 添加元素
 * data:添加的数据
 * index:索引，从0开始
 * return：是否失败
 * */
int _list_add(pList self, pData data, unsigned int index) {
    pNode node = (pNode)malloc(sizeof(node));
    node->d = data;
    if (0 == index) {
        node->last = NULL;
        node->next = self->head;
        self->head = node;
        self->size++;
        return 0;
    }
    pNode p = self->head;
    if(NULL == p) {
        return 1;
    }
    int i = 0;
    while(NULL != p->next) {
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
    self->size++;
    return 0;
}

/**
 * 删除元素
 * index:索引，从0开始
 * return:删除的值，为NULL表示失败;
 * */
pData _list_remove(pList self, unsigned int index) {
    if (NULL == self || NULL == self->head) {
        return NULL;
    }
    if (0 == index) {
        pNode n = self->head;
        self->head = n->next;
        if (NULL != self->head) {
            self->head->last = NULL;
        }
        pData d = n->d;
        n->d = NULL;
        n->next = NULL;
        free(n);
        self->size--;
        return d;
    }
    pNode p = self->head->next;
    int i = 0;
    while(NULL != p) {
        if (index == (++i)) {
            pData d = p->d;
            if (NULL != p->next) {
                p->next->last = p->last;
            }
            p->last->next = p->next;
            p->last = NULL;
            p->next = NULL;
            p->d = NULL;
            free(p);
            self->size--;
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
pData _list_get(pList list, unsigned int index) {
    int i = 0;
    pNode p = list->head;
    while(NULL != p) {
        if(index == (i++)) {
            return p->d;
        }
        p=p->next;
    }
    return NULL;
}

/**
 * 遍历链表
 * consumer:消费者函数，即遍历链表每个元素做的事
 * */
void _list_foreach(pList self, void (*consumer)(pData)) {
    pNode n = self->head;
    while(NULL != n) {
        consumer(n->d);
        n = n->next;
    }
}

/**
 * 链表是否为空
 * return:为空返回1,不为空返回0
 * */
int _list_isNull(pList self) {
    return self->head == NULL;
}

/**
 * 删除链表
 * freeData:元素删除方法
 * */
void _list_free(pList self, void (freeData)(pData data)) {
    while (0 != self->size && NULL != self->head) {
        freeData(self->remove(self, 0));
    }
    free(self);
}

void __base_data_free(pData data){
    free(data);
}

/**
 * 初始化链表
 * return:初始化的链表
 * */
pList initList() {
    pList l = (pList)malloc(sizeof(list));
    l->size = 0;
    l->head = NULL;
    l->add = _list_add;
    l->remove = _list_remove;
    l->get = _list_get;
    l->foreach = _list_foreach;
    l->isNull = _list_isNull;
    l->free = _list_free;
    return l;
}

/**
 * 入队
 * d:入队的数据
 * */
int _queue_offer(pQueue self, pData d) {
    pNode n = (pNode)malloc(sizeof(node));
    n->d = d;
    n->next = NULL;
    n->last = self->tail;
    if (NULL != self->tail) {
        self->tail->next = n;
    }
    self->tail = n;
    if (NULL == self->list->head) {
        self->list->head = n;
    }
    return 0;
}

/**
 * 出队
 * return:出队的数据
 * */
pData _queue_peek(pQueue self) {
    if (1 == self->list->size) {
        self->tail = NULL;
    }
    return self->list->remove(self->list, 0);
}

/**
 * 队列是否为空
 * return:为空返回1,不为空返回0
 * */
int _queue_isNull(pQueue self) {
    return self->list->isNull(self->list);
}

/**
 * 删除队列
 * freeData:元素删除方法
 * */
void _queue_free(pQueue self, void (freeData)(pData data)) {
    self->list->free(self->list, freeData);
    free(self);
}

/**
 * 初始化队列
 * return:初始化的队列
 * */
pQueue initQueue() {
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
void _stack_push(pStack self, pData data) {
    self->list->add(self->list, data, 0);
}

/**
 * 弹栈
 * return:弹出的数据
 * */
pData _stack_pop(pStack self) {
    pNode n = self->list->head;
    if (NULL == n) {
        return NULL;
    }
    self->list->head = n->next;
    if (NULL != self->list->head) {
        self->list->head->last = NULL;
    }
    pData d = n->d;
    n->d = NULL;
    n->last = NULL;
    n->next = NULL;
    free(n);
    return d;
}

/**
 * 删除堆栈
 * freeData:元素删除方法
 * */
void _stack_free(pStack self, void (freeData)(pData data)) {
    self->list->free(self->list, freeData);
    free(self);
}

/**
 * 初始化堆栈
 * */
pStack initStack() {
    pStack s = (pStack)malloc(sizeof(stack));
    s->list = initList();
    s->pop = _stack_pop;
    s->push = _stack_push;
    s->free = _stack_free;
    return s;
}

/**
 * 获取位置
 * row:行
 * column:列
 * return:位置代码
 * */
int __maze_getPoint(pMaze self, int row, int column) {
    return (row%self->row)*self->column+(column%self->column);
}

/**
 * 输出
 * */
void _maze_show(pMaze self) {
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            printf(show[self->maze[__maze_getPoint(self, i, j)]&0b11111]);
        }
        printf("\n");
    }
}

/**
 * 删除迷宫
 * */
void _maze_free(pMaze self) {
    free(self->maze);
    free(self);
}

/**
 * 移动
 * q:队列
 * point:位置
 * op:原位置
 * direction:方向
 * opposite:反方向
 * condition:条件
 * */
void __maze_move(pMaze self, pQueue q, 
                                int point,int op,
                                int direction,int opposite, 
                                int condition) {
    if (condition                                                       //初始条件
        // && (0 == (self->maze[op] & opposite) || END == self->maze[op])  //不走原方向
        && !HAS_BIT(self->maze[point], direction)                 //该方向没走过
        && WALK != self->maze[point]                                    //不为墙
        && START != self->maze[point]                                   //不为开始
        && END != self->maze[point]) {                                  //不为结束
            if (ROAD == self->maze[point]) {
                pData d = toDataByInt(point);
                q->offer(q, d);    
            }
            ADD_BIT(self->maze[point], direction);
            ADD_BIT(self->maze[point], FLAG);
    }
}

/**
 * 清除标记
 * q:队列
 * direction:方向
 * p:当前位置
 * np:方向位置
 * condition:条件
 * */
void __maze_removeFlag(pMaze self, pQueue q, int direction, int p, int np, int condition) {
    if (condition
        && HAS_BIT(self->maze[np], FLAG)
        && HAS_BIT(self->maze[p], direction) || START == self->maze[p]) {
            REMOVE_BIT(self->maze[np], FLAG);
            pData d = toDataByInt(np);
            q->offer(q, d);
    }
}

/**
 * 移动
 * */
void _maze_run(pMaze self) {
    pQueue q = initQueue();
    pData d = toDataByInt(self->end);
    q->offer(q, d);
    while(!q->isNull(q)) {
        d = q->peek(q);
        int point = d->data.Integer;
        free(d);
        int r = point / self->column;
        int c = point % self->column;
        __maze_move(self, q, __maze_getPoint(self, r+1, c), point, NORTH, SOUTH, r<(self->row-1));
        __maze_move(self, q, __maze_getPoint(self, r, c-1), point, EAST, WEST, c>0);
        __maze_move(self, q, __maze_getPoint(self, r-1, c), point, SOUTH, NORTH, r>0);
        __maze_move(self, q, __maze_getPoint(self, r, c+1), point, WEST, EAST, c<(self->column-1));
    }
    d = toDataByInt(self->start);
    q->offer(q, d);
    while(!q->isNull(q)) {
        d = q->peek(q);
        int point = d->data.Integer;
        free(d);
        int r = point / self->column;
        int c = point % self->column;
        __maze_removeFlag(self, q, NORTH, point, __maze_getPoint(self, r-1, c), r>0);
        __maze_removeFlag(self, q, WEST, point, __maze_getPoint(self, r, c-1), c>0);
        __maze_removeFlag(self, q, SOUTH, point, __maze_getPoint(self, r+1, c), r<(self->row-1));
        __maze_removeFlag(self, q, EAST, point, __maze_getPoint(self, r, c+1), c<(self->column-1));
    }
    q->free(q, __base_data_free);
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            int p = __maze_getPoint(self, i, j);
            if (HAS_BIT(self->maze[p], FLAG)) {
                self->maze[p] = ROAD;
            }
        }
    }
}

int __maze_flag(pMaze self, int r, int c){
    int point = __maze_getPoint(self, r, c);
    if (WALK != self->maze[point]) {
        self->maze[point] |= 0xF00;
        self->maze[point] |= FLAG;
        int en = 3;
        int es = 3;
        int wn = 3;
        int ws = 3;
        if (0 == r || 0 == c || WALK == self->maze[__maze_getPoint(self, r-1, c-1)]) {
            wn--;
        }
        if (0 == r || WALK == self->maze[__maze_getPoint(self, r-1, c)]) {
            self->maze[point] &= ~NORTH_ROAD;
            en--;
            wn--;
        }
        if (0 == r || c == (self->column-1) || WALK == self->maze[__maze_getPoint(self, r-1, c+1)]) {
            en--;
        }
        if (0 == c || WALK == self->maze[__maze_getPoint(self, r, c-1)]) {
            self->maze[point] &= ~WEST_ROAD;
            wn--;
            ws--;
        }
        if (c == (self->column-1) || WALK == self->maze[__maze_getPoint(self, r, c+1)]) {
            self->maze[point] &= ~EASE_ROAD;
            en--;
            es--;
        }
        if (r == (self->row-1) || 0 == c || WALK == self->maze[__maze_getPoint(self, r+1, c-1)]) {
            ws--;
        }
        if (r == (self->row-1) || WALK == self->maze[__maze_getPoint(self, r+1, c)]) {
            self->maze[point] &= ~SOUTH_ROAD;
            es--;
            ws--;
        }
        if (r == (self->row-1) || c == (self->column-1) || WALK == self->maze[__maze_getPoint(self, r+1, c+1)]) {
            es--;
        }
        self->maze[point] |= en == 3 ? BLOCK_FLAG : 0;
        self->maze[point] |= es == 3 ? BLOCK_FLAG : 0;
        self->maze[point] |= wn == 3 ? BLOCK_FLAG : 0;
        self->maze[point] |= ws == 3 ? BLOCK_FLAG : 0;
        printf("---%d %d %x\n", r, c, self->maze[point]>>6);
        self->print(self);
        return 1;
    }
    return 0;
}

int __maze_findNodePoint(pMaze self, pList points, pList linkPoints, int point, int condition){
    if (!condition) {
        return 0;
    }
    if (WALK == self->maze[point]){
        return 0;
    }
    if (__util_hasBit(self->maze[point], FLAG)) {
        return 0;
    }
    int r = point / self->column;
    int c = point % self->column;
    __maze_flag(self, r, c);
    pData d = toDataByInt(point);
    if (
        1
        // __util_hasBit(self->maze[point], EN_FLAG | ES_FLAG)
        // || __util_hasBit(self->maze[point], WS_FLAG | ES_FLAG)
        // || __util_hasBit(self->maze[point], WN_FLAG | WS_FLAG)
        // || __util_hasBit(self->maze[point], EN_FLAG | WN_FLAG)
        // || __util_hasBit(self->maze[point], E_FLAG | WN_FLAG)
        // || __util_hasBit(self->maze[point], E_FLAG | WS_FLAG)
        // || __util_hasBit(self->maze[point], S_FLAG | EN_FLAG)
        // || __util_hasBit(self->maze[point], S_FLAG | WN_FLAG)
        // || __util_hasBit(self->maze[point], W_FLAG | EN_FLAG)
        // || __util_hasBit(self->maze[point], W_FLAG | ES_FLAG)
        // || __util_hasBit(self->maze[point], N_FLAG | ES_FLAG)
        // || __util_hasBit(self->maze[point], N_FLAG | WS_FLAG)
    ) {
        linkPoints->add(linkPoints, d, linkPoints->size);
    } else {
        points->add(points, d, points->size);
    }
    int end;
    end |= __maze_findNodePoint(self, points, linkPoints, __maze_getPoint(self, r-1, c), r>0);
    end |= __maze_findNodePoint(self, points, linkPoints, __maze_getPoint(self, r, c-1), c>0);
    end |= __maze_findNodePoint(self, points, linkPoints, __maze_getPoint(self, r+1, c), r<self->row-1);
    end |= __maze_findNodePoint(self, points, linkPoints, __maze_getPoint(self, r, c+1), r<self->column-1);
    return end || END == self->maze[point];
}

void __mazeNode_free(pData d){
    pMazeNode mn = (pMazeNode)d->data.Object;
    mn->points->free(mn->points, __Data_free);
    mn->in->free(mn->in, __Data_free);
    mn->out->free(mn->out, __Data_free);
    mn->link_node->free(mn->link_node, __Data_free);
    free(mn);
    free(d);
}

int __maze_move2(pMaze self, pStack nodeStack, int point, int condition) {
    if (!condition || WALK == self->maze[point]) {
        return 0;
    }
    if (__util_hasBit(self->maze[point], BLOCK_FLAG)) {
        pList points = initList();
        pList in = initList();
        pList out = initList();
        pList link = initList();
        pMazeNode sunMazeNode = (pMazeNode)malloc(sizeof(mazeNode));
        sunMazeNode->points = points;
        sunMazeNode->in = in;
        sunMazeNode->out = out;
        sunMazeNode->link_node = link;
        pStack temp = initStack();
        int isIn = 0;
        while (!nodeStack->list->isNull(nodeStack->list)) {
            pData data = nodeStack->pop(nodeStack);
            pMazeNode mazeNode = (pMazeNode)data->data.Object;
            for(int i = 0; i < mazeNode->points->size; i++) {
                pData d = mazeNode->points->get(mazeNode->points, i);
                isIn |= d->data.Integer == point;
                points->add(points, d, 0);
            }
            for(int i = 0 ;i < mazeNode->link_node->size; i++) {
                link->add(points, mazeNode->link_node->get(mazeNode->link_node, i), 0);
            }
            for(int i = 0 ;i < mazeNode->in->size; i++) {
                in->add(points, mazeNode->in->get(mazeNode->in, i), 0);
            }
            for(int i = 0 ;i < mazeNode->out->size; i++) {
                out->add(points, mazeNode->out->get(mazeNode->out, i), 0);
            }
            temp->push(temp, data);
            if (isIn) {
                temp->free(temp, __mazeNode_free);
                pData d = toDataByObject(sunMazeNode);
                nodeStack->push(nodeStack, d);
            }
        }
        if (!isIn) {
            while (temp->list->isNull(temp->list)) {
                nodeStack->push(nodeStack, temp->pop(temp));
            }
        }
        return 0;
    }
    int end = END == self->maze[point];
    int r = point / self->column;
    int c = point % self->column;
    int flag = __util_hasBit(self->maze[point], 0b1111111111000000);
    __maze_flag(self, r, c);
    if (!__util_hasBit(self->maze[point], BLOCK_FLAG) && flag) {
        if (__maze_move2(self, nodeStack, __maze_getPoint(self, r-1, c), r>0)) {
            end = 1;
            self->maze[point] |= NORTH;
        }
        if (__maze_move2(self, nodeStack, __maze_getPoint(self, r, c-1), c>0)) {
            end = 1;
            self->maze[point] |= EAST;
        }
        if (__maze_move2(self, nodeStack, __maze_getPoint(self, r+1, c), r<self->row-1)) {
            end = 1;
            self->maze[point] |= SOUTH;
        }
        if (__maze_move2(self, nodeStack, __maze_getPoint(self, r, c+1), c<self->column-1)) {
            end = 1;
            self->maze[point] |= WEST;
        }
        return end;
    }
    pMazeNode mazeNode = (pMazeNode)malloc(sizeof(mazeNode));
    pList points = initList();
    pList linkPoint = initList();
    mazeNode->points = points;
    mazeNode->link_node = linkPoint;
    mazeNode->in = initList();
    mazeNode->out = initList();
    if (START != self->maze[point]) {
        pData inData = toDataByInt(point);
        mazeNode->in->add(mazeNode->in, inData, 0);
    }
    self->maze[point] &= ~0b1111111111000000;
    end = __maze_findNodePoint(self, points, linkPoint, point, 1);
    mazeNode->end = end;
    pData d = toDataByObject(mazeNode);
    nodeStack->push(nodeStack, d);
    pNode p = linkPoint->head;
    while(NULL != p){
        int link_point = p->d->data.Integer;
        int pr = point / self->column;
        int pc = point % self->column;
        if (!__util_hasBit(self->maze[link_point], NORTH_ROAD) && r>0) {
            if (__maze_move2(self, nodeStack, __maze_getPoint(self, r-1, c), r>0)) {
                end = 1;
                pData inData = toDataByInt(__maze_getPoint(self, r-1, c));
                mazeNode->out->add(mazeNode->out, inData, 0);
            }
        }
        if (!__util_hasBit(self->maze[link_point], WEST_ROAD) && c>0) {
            if (__maze_move2(self, nodeStack, __maze_getPoint(self, r, c-1), c>0)) {
                end = 1;
                pData inData = toDataByInt(__maze_getPoint(self, r, c-1));
                mazeNode->out->add(mazeNode->out, inData, 0);
            }
        }
        if (!__util_hasBit(self->maze[link_point], SOUTH_ROAD) && r<self->row-1) {
            if (__maze_move2(self, nodeStack, __maze_getPoint(self, r+1, c), r<self->row-1)) {
                end = 1;
                pData inData = toDataByInt(__maze_getPoint(self, r+1, c));
                mazeNode->out->add(mazeNode->out, inData, 0);
            }
        }
        if (!__util_hasBit(self->maze[link_point], EASE_ROAD) && c<self->column-1) {
            if (__maze_move2(self, nodeStack, __maze_getPoint(self, r, c+1), c<self->column-1)) {
                end = 1;
                pData inData = toDataByInt(__maze_getPoint(self, r, c+1));
                mazeNode->out->add(mazeNode->out, inData, 0);
            }
        }
        p = p->next;
    }
    return end;
}

void _maze_run2(pMaze self){
    // pList nodeList = initList();
    // __maze_toNode(self, nodeList, self->start);
    pStack nodeStack = initStack();
    __maze_move2(self, nodeStack, self->start, 1);
}

void __debug_show(pMaze self){
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            int point = __maze_getPoint(self, i, j);
            if (__util_hasBit(self->maze[point], BLOCK_FLAG)) {
                printf("b ");
            } else if(__util_hasBit(self->maze[point], 0b11111111000000)) {
                printf("%X", self->maze[point]>>6 & 0xff);
            } else {
                printf(show[self->maze[point] & 0b11111]);
            }
        }
        printf("\n");
    }
}

/**
 * 生成路径
 * seed:种子
 * */
void _maze_generateRoad(pMaze self, unsigned int seed) {
    srand(seed);
    int size = self->row * self->column ;
    for(int i = size>>4; i < size; i++) {
        self->maze[rand()%size] = ROAD;
    }
    do {
        self->start = rand()%size;
    } while(WALK != self->maze[self->start]);
    self->maze[self->start] = START;
    do {
        self->end = rand()%size;
    } while(WALK != self->maze[self->end]);
    self->maze[self->end] = END;
    return;
}

/**
 * 保存到文件
 * fileName:要保存的文件名
 * */
void _maze_save(pMaze self, char* fileName) {
    FILE *file = fopen(fileName, "w");
    fprintf(file, "%d %d\n", self->row, self->column);
    int size = self->row * self->column;
    for(int i = 0; i < size; i++) {
        fprintf(file, "%d ", self->maze[i]);
    }
    fclose(file);
}

/**
 * 通过文件创建迷宫
 * fileName:文件名
 * return:初始化的迷宫
 * */
pMaze initMazeByFile(char *fileName) {
    FILE *file = fopen(fileName,"r");
    pMaze ret = (pMaze)malloc(sizeof(maze));
    fscanf(file, "%d %d\n",&(ret->row), &(ret->column));
    int size = ret->row*ret->column;
    int *m = (int*)malloc(sizeof(int)*size);
    for(int i = 0; i < size; i++) {
        fscanf(file, "%d", &(m[i]));
        if (START == m[i]) {
            ret->start = i;
        } else if (END == m[i]) {
            ret->end = i;
        }
    }
    fclose(file);
    ret->maze = m;
    ret->print = _maze_show;
    ret->generate = _maze_generateRoad;
    ret->save = _maze_save;
    ret->free = _maze_free;
    ret->run = _maze_run;
    return ret;
}

/**
 * 创建迷宫
 * row:行数
 * column:列数
 * return:初始化的迷宫
 * */
pMaze initMaze(unsigned int row, unsigned int column) {
    pMaze ret = (pMaze)malloc(sizeof(maze));
    ret->row = row;
    ret->column = column;
    int size = row*column;
    int* m = (int*)malloc(sizeof(int)*size);
    for(int i = 0; i < size; i++) {
        m[i]=WALK;
    }
    ret->start = -1;
    ret->end = -1;
    ret->maze = m;
    ret->print = __debug_show;
    ret->generate = _maze_generateRoad;
    ret->save = _maze_save;
    ret->free = _maze_free;
    ret->run = _maze_run2;
    return ret;
}

/**
 * 获取种子
 * */
int __menu_inputSeed(){
    int i = 0;
    char ch = getchar();
    if ('\n' == ch) {
        return time(NULL);
    }
    char *in = (char *)malloc(sizeof(char) * 128);
    int seed = ch >= '0' && ch <= '9' ? ch - '0' : 0;
    int isNum = ch >= '0' && ch < '9' || ch == '-';
    in[0] = ch;
    for(int i = 1; i < 128; i++) {
        ch = getchar();
        if ('\n' == ch) {
            in[i] = '\0';
            break;
        }
        isNum = isNum && ch >= '0' && ch <='9';
        in[i] = ch;
        seed = seed * 10 + ch -'0';
    }
    if (!isNum) {
        int i = 0;
        seed = 0;
        while('\0' != in[i]){
            seed ^= in[i] << ((3*i)%(8*sizeof(int)));
            i++;
        }
    } else if ('-' == in[0]){
        seed *= -1;
    }
    free(in);
    return seed;
}

/**
 * 随机生成迷宫菜单
 * */
pMaze __menu_random() {
    int r = 0;
    int c = 0;
    printf("                              ==== 随机生成 ====\n");
    while(r <= 0) {
        printf("请输入行数:");
        scanf("%d", &r);
    }
    while(c <= 0) {
        printf("请输入列数:");
        scanf("%d", &c);
    }
    getchar();
    pMaze maze = initMaze(r, c);
    printf("请输入种子(留空则为随机种子):");
    int seed = __menu_inputSeed();
    maze->generate(maze, seed);
    printf("种子为:%d的迷宫为:\n", seed);
    maze->print(maze);
    char in='\0';
    while(in == '\0') {
        printf("请选择:\n");
        printf("1.开始移动(默认)\n");
        printf("2.保存到文件\n");
        printf("3.重新生成\n");
        in = getchar();
        while('\n' != in && '\n'!=getchar());
        switch (in) {
            case '\n':
            case '1':
                return maze;
            case '2':
                printf("请输入文件名:");
                char* fileName = (char*)malloc(sizeof(char)*64);
                scanf("%s", fileName);
                while('\n'!=getchar());
                maze->save(maze, fileName);
                printf("保存成功!\n");
                in = '\0';
                break;
            case '3':
                return __menu_random();
            default:
                printf("输入错误，请选择正确的选项!\n");
                in = '\0';
                break;
        }
    }
}

/**
 * 文件输入迷宫菜单
 * */
pMaze __menu_file() {
    printf("请输入文件名:");
    char* fileName = (char *)malloc(sizeof(char)*64);
    scanf("%s", fileName);
    while('\n'!=getchar());
    pMaze maze = initMazeByFile(fileName);
    free(fileName);
    printf("当前迷宫为:\n");
    maze->print(maze);
    char in = '\0';
    while('\0' == in) {
        printf("是否确认？(Y/n):");
        in = getchar();
        while('\n' != in && '\n'!=getchar());
        if ('n' == in || 'N' == in) {
            return __menu_file();
        } else if ('Y' == in ||'y' == in || '\n' == in) {
            return maze;
        } else {
            printf("输入错误，请选择正确的选项\n");
            in = '\0';
        }
    }
}

/**
 * 菜单
 * */
void menu() {
    printf("                              ==== 迷宫问题 ====\n");
    printf("1.随机生成迷宫\n");
    printf("2.文件读取迷宫\n");
    printf("0.退出\n");
    char in = getchar();
    while('\n' != in && '\n'!=getchar());
    pMaze maze = NULL;
    switch (in) {
        case '1':
            maze = __menu_random();
            break;
        case '2':
            maze = __menu_file();
            break;
        case '0':
            return;
        default:
            printf("输入错误，请选择正确的选项\n");
            menu();
            return;
    }
    if (NULL == maze) {
        menu();
        return;
    }
    maze->run(maze);
    printf("结果为:\n");
    maze->print(maze);
    printf("按下回车键返回主菜单\n");
    while('\n' != getchar());
    maze->free(maze);
    menu();
}

/**
 * 主方法
 * */
int main() {
    menu();
    return 0;
}