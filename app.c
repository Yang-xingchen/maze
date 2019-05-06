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
    } data;
    enum {
        OBJECT,
        INTEGER,
        DOUBLE,
        LONG,
    } type;
    int (*equal)(struct data*, struct data*, int (*obj_equal)(void*, void*));
    void (*free)(struct data*);
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
    int (*consist)(struct list *, pData, int (*obj_equal)(void*, void*));
    void (*free)(struct list *);
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
    void (*free)(struct queue *);
}queue, *pQueue;
/**
 * 堆栈
 * */
typedef struct stack {
    pList list;
    pData (*pop)(struct stack*);
    void (*push)(struct stack *,pData);
    int (*isNull)(struct stack *);
    void (*free)(struct stack *);
}stack, *pStack;
/**
 * 迷宫节点，辅助走迷宫
 * */
typedef struct mazeNode {
    int end;
    int lock;
    pList points;
    pList linkPoint;
    pList in;
    pList out;
}mazeNode, *pMazeNode;

int __maze_isWalk(pMaze self, int r, int c, int direction);

//显示移动
#define EAST            0x00001
#define SOUTH           0x00002
#define WEST            0x00004
#define NORTH           0x00008

//其他显示
#define ROAD            0x00010
#define WALK            0x00020
#define START           0x00040
#define END             0x00080

//标记通路
#define EAST_ROAD       0x00100
#define SOUTH_ROAD      0x00200
#define WEST_ROAD       0x00400
#define NORTH_ROAD      0x00800

//掩码
#define DIRETCION       0x0000F
#define DIRETCION_ROAD  0x00F00

//其他标记
#define FLAG            0x01000
#define BLOCK_FLAG      0x02000
#define BOUNDARY_FLAG   0x04000
#define TO_END          0x08000
#define BLOCK_IN        0x10000
#define BLOCK_OUT       0x20000
#define LOCK            0x40000

//method
#define ADD_BIT(bitset, bit)                            ((bitset) |= (bit))
#define ADD_BIT_CONDITION(bitset, bit, condition)       (ADD_BIT(bitset, (condition) ? (bit) : 0))
#define REMOVE_BIT(bitset, bit)                         ((bitset) &= ~(bit))
#define REMOVE_BIT_CONDITION(bitset, bit, condition)    (REMOVE_BIT(bitset, (condition) ? (bit) : 0))
#define HAS_BIT(bitset, bit)                            ((bitset) & (bit))

char* show[] = {
    "  ", "→ ", "↓ ", "↘ ",
    "← ", "↔ ", "↙ ", "⇓ ",
    "↑ ", "↗ ", "↕ ", "⇒ ",
    "↖ ", "⇑ ", "⇐ ", "* "
};
char* ROAD_SHOW = "  ";
char* WALK_SHOW = "⬛";
char* START_SHOW = "S ";
char* END_SHOW = "E ";

/**
 * 基础数据释放
 * data:释放的Data
 * */
void __Data_free(pData data){
    free(data);
}

/**
 * 元素比较
 * other:相比的元素
 * obj_equal:无用参数，请使用NULL
 * return:是否相同
 * */
int __Data_equalByInt(pData self, pData other, int (*obj_equal)(void*, void*)){
    if (NULL == other) {
        return 0;
    }
    if (INTEGER == other->type) {
        return self->data.Integer == other->data.Integer;
    }
    return 0;
}

/**
 * 元素比较
 * other:相比的元素
 * obj_equal:比较方法，可为NULL（比较地址）
 * return:是否相同
 * */
int __Data_equalByObj(pData self, pData other, int (*obj_equal)(void*, void*)){
    if (NULL == other) {
        return 0;
    }
    if (OBJECT == other->type) {
        if (NULL != obj_equal){
            return obj_equal(self->data.Object, other->data.Object);
        } else {
            return self->data.Object == other->data.Object;
        }
    }
    return 0;
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
    d->equal = __Data_equalByInt;
    d->free = __Data_free;
    return d;
}

/**
 * 包装一般对象
 * object:包装的对象
 * return:创建的Data
 * */
pData toDataByObject(void* object, void (*free)(pData)){
    pData d = (pData)malloc(sizeof(Data));
    d->data.Object = object;
    d->type = OBJECT;
    d->equal = __Data_equalByObj;
    d->free = free;
    return d;
}

/**
 * 添加元素
 * data:添加的数据
 * index:索引，从0开始
 * return：是否失败
 * */
int _list_add(pList self, pData data, unsigned int index) {
    pNode n = (pNode)malloc(sizeof(node));
    n->d = data;
    if (0 == index) {
        n->last = NULL;
        n->next = self->head;
        if (NULL!=self->head) {
            self->head->last = n;
        }
        self->head = n;
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
        free(n);
        return 1;
    }
    if (NULL != p->next) {
        p->next->last = n;
    }
    n->next = p->next;
    p->next = n;
    n->last = p;
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
 * 存在某元素
 * d:存在的元素
 * obj_equal:比较方式，为NULL使用默认方式
 * return:是否存在
 * */
int _list_consist(pList self, pData d, int (*obj_equal)(void*, void*)){
    pNode n = self->head;
    while(NULL != n) {
        if (d->equal(d, n->d, obj_equal)) {
            return 1;
        }
        n = n->next;
    }
    return 0;
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
 * */
void _list_free(pList self) {
    while (!self->isNull(self)) {
        pData d = self->remove(self, 0);
        d->free(d);
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
    l->consist = _list_consist;
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
    self->list->size++;
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
 * */
void _queue_free(pQueue self) {
    self->list->free(self->list);
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
    self->list->size--;
    free(n);
    return d;
}

/**
 * 堆栈是否为空
 * return:为空返回1,不为空返回0
 * */
int _stack_isNull(pStack self) {
    return self->list->isNull(self->list);
}

/**
 * 删除堆栈
 * */
void _stack_free(pStack self) {
    self->list->free(self->list);
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
    s->isNull = _stack_isNull;
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
 * 获取显示图形
 * code:代码
 * return:显示的图形
 * */
char* __maze_getShow(int code) {
    if (HAS_BIT(code, WALK)) {
        return WALK_SHOW;
    } else if(HAS_BIT(code, START)) {
        return START_SHOW;
    } else if(HAS_BIT(code, END)) {
        return END_SHOW;
    } else if(HAS_BIT(code, ROAD)) {
        if (HAS_BIT(code, DIRETCION)) {
            return show[code & DIRETCION];
        } else {
            return ROAD_SHOW;
        }
    } else {
        return "er";
    }
}

/**
 * 输出
 * */
void _maze_show(pMaze self) {
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            printf(__maze_getShow(self->maze[__maze_getPoint(self, i, j)]));
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

int __maze_isWalk(pMaze self, int r, int c, int direction) {
    int result = 0;
    result |= (HAS_BIT(direction, EAST) && self->column-1 == c);
    result |= (HAS_BIT(direction, SOUTH) && self->row-1 == r);
    result |= (HAS_BIT(direction, WEST) && 0 == c);
    result |= (HAS_BIT(direction, NORTH) && 0 == r);
    if (result) return result;
    result |= direction == EAST && HAS_BIT(self->maze[__maze_getPoint(self, r, c+1)], WALK);
    result |= direction == SOUTH && HAS_BIT(self->maze[__maze_getPoint(self, r+1, c)], WALK);
    result |= direction == WEST && HAS_BIT(self->maze[__maze_getPoint(self, r, c-1)], WALK);
    result |= direction == NORTH && HAS_BIT(self->maze[__maze_getPoint(self, r-1, c)], WALK);
    result |= direction == (EAST | SOUTH) && HAS_BIT(self->maze[__maze_getPoint(self, r+1, c+1)], WALK);
    result |= direction == (EAST | NORTH) && HAS_BIT(self->maze[__maze_getPoint(self, r-1, c+1)], WALK);
    result |= direction == (WEST | SOUTH) && HAS_BIT(self->maze[__maze_getPoint(self, r+1, c-1)], WALK);
    result |= direction == (WEST | NORTH) && HAS_BIT(self->maze[__maze_getPoint(self, r-1, c-1)], WALK);
    return result;
}

void __debug_show(pMaze self){
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            int point = __maze_getPoint(self, i, j);
            if (HAS_BIT(self->maze[point], TO_END)) {
                printf("TE");
            } else {
                printf(__maze_getShow(self->maze[point]));
            }
        }
        printf("\n");
    }
}
void __debug_show2(pMaze self){
    for(int i = 0; i < self->row; i++) {
        for(int j = 0; j < self->column; j++) {
            int point = __maze_getPoint(self, i, j);
            if (HAS_BIT(self->maze[point], BLOCK_IN)) {
                printf("I%c", HAS_BIT(self->maze[point], BLOCK_FLAG)?(HAS_BIT(self->maze[point], TO_END)?'E':'B'):' ');
            } else if(HAS_BIT(self->maze[point], BLOCK_OUT)){
                printf("O%c", HAS_BIT(self->maze[point], BLOCK_FLAG)?(HAS_BIT(self->maze[point], TO_END)?'E':'B'):' ');
            } else if (HAS_BIT(self->maze[point], BOUNDARY_FLAG)) {
                printf("F%c", HAS_BIT(self->maze[point], BLOCK_FLAG)?(HAS_BIT(self->maze[point], TO_END)?'E':'B'):' ');
            } else if(HAS_BIT(self->maze[point], BLOCK_FLAG)){
                printf(" %c", HAS_BIT(self->maze[point], TO_END)?'E':'B');
            } else {
                if (HAS_BIT(self->maze[point], DIRETCION_ROAD) 
                    || HAS_BIT(self->maze[point], 0xF0)) {
                    int code = self->maze[point]>>8 & DIRETCION;
                    ADD_BIT(code, self->maze[point] &0xF0);
                    printf(__maze_getShow(code));
                } else {
                    printf("--");
                }
            }
        }
        printf("\n");
    }
}

void __mazeNode_free(pData d){
    pMazeNode mn = (pMazeNode)d->data.Object;
    mn->points->free(mn->points);
    mn->in->free(mn->in);
    mn->out->free(mn->out);
    mn->linkPoint->free(mn->linkPoint);
    free(mn);
    free(d);
}

pMazeNode initMazeNode(){
    pMazeNode mn = (pMazeNode)malloc(sizeof(mazeNode));
    mn->end = 0;
    mn->lock = 0;
    mn->in = initList();
    mn->linkPoint = initList();
    mn->out = initList();
    mn->points = initList();
    return mn;
}

int __maze_flag(pMaze self, int r, int c){
    int point = __maze_getPoint(self, r, c);
    if (WALK != self->maze[point]) {
        int e = __maze_isWalk(self, r, c, EAST);
        int s = __maze_isWalk(self, r, c, SOUTH);
        int w = __maze_isWalk(self, r, c, WEST);
        int n = __maze_isWalk(self, r, c, NORTH);
        int es = __maze_isWalk(self, r, c, EAST | SOUTH);
        int en = __maze_isWalk(self, r, c, EAST | NORTH);
        int ws = __maze_isWalk(self, r, c, WEST | SOUTH);
        int wn = __maze_isWalk(self, r, c, WEST | NORTH);
        ADD_BIT(self->maze[point], FLAG);
        ADD_BIT_CONDITION(self->maze[point], BLOCK_FLAG, !(e | es | s));
        ADD_BIT_CONDITION(self->maze[point], BLOCK_FLAG, !(e | en | n));
        ADD_BIT_CONDITION(self->maze[point], BLOCK_FLAG, !(w | ws | s));
        ADD_BIT_CONDITION(self->maze[point], BLOCK_FLAG, !(w | wn | n));
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | EAST_ROAD, !e & en & es);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | EAST_ROAD, !e & en & s);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | EAST_ROAD, !e & n & es);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | SOUTH_ROAD, !s & ws & es);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | SOUTH_ROAD, !s & ws & e);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | SOUTH_ROAD, !s & w & es);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | WEST_ROAD, !w & wn & ws);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | WEST_ROAD, !w & wn & s);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | WEST_ROAD, !w & n & ws);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | NORTH_ROAD, !n & wn & en);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | NORTH_ROAD, !n & wn & e);
        ADD_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG | NORTH_ROAD, !n & w & en);
        REMOVE_BIT_CONDITION(self->maze[point], BOUNDARY_FLAG, !HAS_BIT(self->maze[point], BLOCK_FLAG));
        return 1;
    }
    return 0;
}

int __maze_findNodePoint(pMaze self, pMazeNode mazeNode, int point, int condition){
    if (!condition || WALK == self->maze[point] || HAS_BIT(self->maze[point], FLAG)) {
        return 0;
    }
    int end = 0;
    if (HAS_BIT(self->maze[point], END)){
        mazeNode->out->add(mazeNode->out, toDataByInt(point), 0);
        ADD_BIT(self->maze[point], BLOCK_OUT);
        end = 1;
    } else if (HAS_BIT(self->maze[point], START)){
        mazeNode->in->add(mazeNode->in, toDataByInt(point), 0);
        ADD_BIT(self->maze[point], BLOCK_IN);
    }
    int r = point / self->column;
    int c = point % self->column;
    __maze_flag(self, r, c);
    mazeNode->points->add(mazeNode->points, toDataByInt(point), 0);
    if (HAS_BIT(self->maze[point], BOUNDARY_FLAG)) {
        mazeNode->linkPoint->add(mazeNode->linkPoint, toDataByInt(point), 0);
        end |= __maze_findNodePoint(self, mazeNode, 
                    __maze_getPoint(self, r, c+1), 
                    !HAS_BIT(self->maze[point], EAST_ROAD) && c<(self->column-1));
        end |= __maze_findNodePoint(self, mazeNode, 
                    __maze_getPoint(self, r+1, c), 
                    !HAS_BIT(self->maze[point], SOUTH_ROAD) && r<(self->row-1));
        end |= __maze_findNodePoint(self, mazeNode, 
                    __maze_getPoint(self, r, c-1), 
                    !HAS_BIT(self->maze[point], WEST_ROAD) && c>0);
        end |= __maze_findNodePoint(self, mazeNode, 
                    __maze_getPoint(self, r-1, c), 
                    !HAS_BIT(self->maze[point], NORTH_ROAD) && r>0);
        return end;
    }
    end |= __maze_findNodePoint(self, mazeNode, __maze_getPoint(self, r, c+1), c<(self->column-1));
    end |= __maze_findNodePoint(self, mazeNode, __maze_getPoint(self, r+1, c), r<(self->row-1));
    end |= __maze_findNodePoint(self, mazeNode, __maze_getPoint(self, r, c-1), c>0);
    end |= __maze_findNodePoint(self, mazeNode, __maze_getPoint(self, r-1, c), r>0);
    return end;
}

int __maze_move(pMaze, pList, int, int);

void __maze_move_flagToEnd(pMaze self, pMazeNode mazeNode){
    for(pNode n = mazeNode->points->head; NULL != n && mazeNode->end; n = n->next){
        ADD_BIT(self->maze[n->d->data.Integer], TO_END);
    }
}

void __maze_move_block_toNext(pMaze self, pMazeNode mazeNode, 
                    pList nodeList, 
                    int r, int c, int oldPoint,
                    int condition1 ,int condition2){
    if (!condition1 || !condition2) {
        return;
    }
    if (__maze_move(self, nodeList, __maze_getPoint(self, r, c), condition2)) {
        __maze_move_flagToEnd(self, mazeNode);
        mazeNode->end = 1;
        pData inData = toDataByInt(oldPoint);
        mazeNode->out->add(mazeNode->out, inData, 0);
        ADD_BIT(self->maze[oldPoint], BLOCK_OUT);
    }
}

int __maze_move_block(pMaze self, pMazeNode mazeNode, pList nodeList){
    for(pNode p = mazeNode->linkPoint->head; NULL != p; p = p->next){
        int link_point = p->d->data.Integer;
        int r = link_point / self->column;
        int c = link_point % self->column;
        __maze_move_block_toNext(self, mazeNode, nodeList, 
                r, c+1, link_point, 
                HAS_BIT(self->maze[link_point], EAST_ROAD), c<(self->column-1));
        __maze_move_block_toNext(self, mazeNode, nodeList, 
                r+1, c, link_point, 
                HAS_BIT(self->maze[link_point], SOUTH_ROAD), r<(self->row-1));
        __maze_move_block_toNext(self, mazeNode, nodeList, 
                r, c-1, link_point, 
                HAS_BIT(self->maze[link_point], WEST_ROAD), c>0);
        __maze_move_block_toNext(self, mazeNode, nodeList, 
                r-1, c, link_point, 
                HAS_BIT(self->maze[link_point], NORTH_ROAD), r>0);
    }
    return mazeNode->end;
}

int __maze_move_isBlock(pMaze self, pList nodeList, int point){
    int r = point / self->column;
    int c = point % self->column;
    pMazeNode mazeNode = initMazeNode();
    pData inData = toDataByInt(point);
    mazeNode->in->add(mazeNode->in, inData, 0);
    ADD_BIT(self->maze[point], BLOCK_IN);
    REMOVE_BIT(self->maze[point], FLAG);
    int end = __maze_findNodePoint(self, mazeNode, point, 1);
    mazeNode->end = end;
    pData d = toDataByObject(mazeNode, __mazeNode_free);
    nodeList->add(nodeList, d, 0);
    if (end) {
        __maze_move_flagToEnd(self, mazeNode);
    } else {
        mazeNode->lock = 1;
        if (__maze_move_block(self, mazeNode, nodeList)) {
            __maze_move_flagToEnd(self, mazeNode);
        }
        mazeNode->lock = 0;
    }
    return mazeNode->end;
}

int __maze_move_inExistBlock(pMaze self, int point, pList nodeList){
    pData pointData = toDataByInt(point);
    pMazeNode mazeNode = NULL;
    for(pNode node = nodeList->head; NULL != node; node = node->next){
        mazeNode = (pMazeNode)node->d->data.Object;
        pList points = mazeNode->linkPoint;
        if (points->consist(points, pointData, NULL)) {
            if (mazeNode->lock) {
                return 0;
            }
            if (mazeNode->end) {
                mazeNode->in->add(mazeNode->in, pointData, 0);
                ADD_BIT(self->maze[point], BLOCK_IN);
                return 1;
            }
            mazeNode->lock = 1;
            mazeNode->end = __maze_move_block(self, mazeNode, nodeList);
            mazeNode->lock = 0;
            ADD_BIT_CONDITION(self->maze[point], BLOCK_IN, mazeNode->end);
            return mazeNode->end;
        }
    }
    return 0;
}

int __maze_move_road_toNext(pMaze self, pList nodeList, 
        int point, int np,
        int direction, int opposite, 
        int condition){
    if (HAS_BIT(self->maze[np], opposite)) {
        return 0;
    }
    if (!HAS_BIT(self->maze[point], direction) && __maze_move(self, nodeList, np, condition)) {
        ADD_BIT_CONDITION(self->maze[point], direction, !HAS_BIT(self->maze[np], opposite));
        ADD_BIT(self->maze[point], TO_END);
        return 1;
    }
    return 0;
}

int __maze_move_isRoad(pMaze self, pList nodeList, int point){
    if (HAS_BIT(self->maze[point], LOCK)) {
        return 0;
    }
    int end = 0;
    int r = point / self->column;
    int c = point % self->column;
    ADD_BIT(self->maze[point], LOCK);
    while(1){
        if (__maze_move_road_toNext(self, nodeList, point, __maze_getPoint(self, r, c+1), EAST, WEST, c<(self->column-1))) {
            end = 1;
            continue;
        }
        if (__maze_move_road_toNext(self, nodeList, point, __maze_getPoint(self, r+1, c), SOUTH, NORTH, r<(self->row-1))) {
            end = 1;
            continue;
        }
        if (__maze_move_road_toNext(self, nodeList, point, __maze_getPoint(self, r, c-1), WEST, EAST, c>0)) {
            end = 1;
            continue;
        }
        if (__maze_move_road_toNext(self, nodeList, point, __maze_getPoint(self, r-1, c), NORTH, SOUTH, r>0)) {
            end = 1;
            continue;
        }
        break;
    }
    REMOVE_BIT(self->maze[point], LOCK);
    return end;
}

int __maze_move(pMaze self, pList nodeList, int point, int condition) {
    if (!condition || HAS_BIT(self->maze[point], WALK)) {
        return 0;
    }
    if (HAS_BIT(self->maze[point], END | TO_END)) {
        return 1;
    }
    if (HAS_BIT(self->maze[point], BOUNDARY_FLAG)) {
        return __maze_move_inExistBlock(self, point, nodeList);
    }
    __maze_flag(self, point / self->column, point % self->column);
    if (HAS_BIT(self->maze[point], BLOCK_FLAG)) {
        return __maze_move_isBlock(self, nodeList, point);
    }
    return __maze_move_isRoad(self, nodeList, point);
}

void __maze_run_move(pMaze self, pQueue q, int point, int op, int direction, int opposite, int condition){
    if (condition
        && !HAS_BIT(self->maze[op], opposite)
        && !HAS_BIT(self->maze[point], direction)
        && !HAS_BIT(self->maze[point], WALK)
        && HAS_BIT(self->maze[point], TO_END)
    ) {
        if (HAS_BIT(self->maze[point], ROAD)){
            q->offer(q, toDataByInt(point));
        }
        ADD_BIT(self->maze[point], direction);
        ADD_BIT(self->maze[point], FLAG);
    }
}

void __maze_run_removeFlag(pMaze self, pQueue q, int direction, int p, int np, int condition) {
    if (condition
        && HAS_BIT(self->maze[p], FLAG)
        && HAS_BIT(self->maze[p], direction)) {
            REMOVE_BIT(self->maze[p], FLAG);
            q->offer(q, toDataByInt(np));
    }
}

void __printInt(pData d){
    printf("%d ", d->data.Integer);
}
void __printMN(pMazeNode n){
    printf("---%d---", n);
    printf("\npoints:");
    n->points->foreach(n->points, __printInt);
    printf("\nlinkPoint:");
    n->linkPoint->foreach(n->linkPoint, __printInt);
    printf("\nin:");
    n->in->foreach(n->in, __printInt);
    printf("\nout:");
    n->out->foreach(n->out, __printInt);
    printf("\n------------\n");
}

void __maze_run_node(pMaze self, pList nodeList){
    for(pNode ln = nodeList->head; NULL != ln; ln = ln->next){
        pMazeNode mazeNode = (pMazeNode)ln->d->data.Object;
        if (!mazeNode->end) {
            continue;
        }
        // __printMN(mazeNode);
        for(pNode pointNode = mazeNode->points->head; NULL!=pointNode; pointNode = pointNode->next){
            REMOVE_BIT(self->maze[pointNode->d->data.Integer], FLAG);
        }
        pQueue q = initQueue();
        for(pNode toNode = mazeNode->out->head; NULL != toNode; toNode = toNode->next){
            int point = toNode->d->data.Integer;
            q->offer(q, toDataByInt(point));
            REMOVE_BIT_CONDITION(self->maze[point], DIRETCION, !HAS_BIT(self->maze[point], END));
            ADD_BIT_CONDITION(self->maze[point], (self->maze[point]>>8 && DIRETCION), !HAS_BIT(self->maze[point], END));
        }
        while(!q->isNull(q)){
            pData data = q->peek(q);
            int point = data->data.Integer;
            int r = point / self->column;
            int c = point % self->column;
            if (!mazeNode->points->consist(mazeNode->points, data, NULL)) {
                __Data_free(data);
                continue;
            }
            __Data_free(data);
            __maze_run_move(self, q, __maze_getPoint(self, r, c-1), point, EAST, WEST, c>0);
            __maze_run_move(self, q, __maze_getPoint(self, r-1, c), point, SOUTH, NORTH, r>0);
            __maze_run_move(self, q, __maze_getPoint(self, r, c+1), point, WEST, EAST, c<(self->column-1));
            __maze_run_move(self, q, __maze_getPoint(self, r+1, c), point, NORTH, SOUTH, r<(self->row-1));
        }
        for(pNode inNode = mazeNode->in->head; NULL != inNode; inNode = inNode->next){
            q->offer(q, toDataByInt(inNode->d->data.Integer));
        }
        while(!q->isNull(q)){
            pData data = q->peek(q);
            int point = data->data.Integer;
            int r = point / self->column;
            int c = point % self->column;
            __Data_free(data);
            __maze_run_removeFlag(self, q, EAST, point, __maze_getPoint(self, r, c+1), c<(self->column-1));
            __maze_run_removeFlag(self, q, SOUTH, point, __maze_getPoint(self, r+1, c), r<(self->row-1));
            __maze_run_removeFlag(self, q, WEST, point, __maze_getPoint(self, r, c-1), c>0);
            __maze_run_removeFlag(self, q, NORTH, point, __maze_getPoint(self, r-1, c), r>0);
        }
        q->free(q);
        for(pNode pointNode = mazeNode->points->head; NULL!=pointNode; pointNode = pointNode->next){
            if (HAS_BIT(self->maze[pointNode->d->data.Integer], FLAG)) {
                REMOVE_BIT(self->maze[pointNode->d->data.Integer] , DIRETCION);
                ADD_BIT(self->maze[pointNode->d->data.Integer], ROAD);
            }
        }
    }
}

void _maze_run(pMaze self){
    pList nodeList = initList();
    __maze_move(self, nodeList, self->start, 1);
    printf("-------------\n");
    __debug_show2(self);
    printf("-------------\n");
    __debug_show(self);
    __maze_run_node(self, nodeList);
    printf("*************\n");
    __debug_show2(self);
    printf("*************\n");
    __debug_show(self);
    nodeList->free(nodeList);
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
    } while(HAS_BIT(self->maze[self->start], WALK));
    self->maze[self->start] = START;
    do {
        self->end = rand()%size;
    } while(HAS_BIT(self->maze[self->end], WALK));
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
    ret->print = _maze_show;
    ret->generate = _maze_generateRoad;
    ret->save = _maze_save;
    ret->free = _maze_free;
    ret->run = _maze_run;
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