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
    int start;
    int end;
    void (*print)(struct maze *);
    void (*generate)(struct maze *, int);
    void (*save)(struct maze *, char *);
    void (*free)(struct maze *);
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
    int (*offer)(struct queue *, void *);
    void* (*peek)(struct queue *);
    void (*free)(struct queue *);
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
    "⬛","S ","E "
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
    while(NULL != self->peek);
    free(self);
}

/**
 * 初始化队列
 * */
pQueue initQueue(){
    pQueue q = (pQueue)malloc(sizeof(queue));
    q->head = NULL;
    q->tail = NULL;
    q->offer = _enQueue;
    q->peek = _outQueue;
    q->free = _freeQueue;
    return q;
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
 * self:自身
 * row:行
 * column:列
 * return:该位置的代码，参考全局变量show
 * */
int _getByXY(pMaze self, int row, int column){
    return self->maze[_getPoint(self, row, column)];
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
            printf(show[_getByXY(self, i ,j)%19]);
        }
        printf("\n");
    }    
}

/**
 * 删除迷宫
 * self:自身
 * */
void _freeMaze(pMaze self){
    free(self->maze);
    free(self);
}

/**
 * 生成路径
 * self:自身
 * seed:种子
 * */
void _generateRoad(pMaze self, int seed){
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
 * self:自身
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
    return ret;
}

/**
 * 创建迷宫
 * row:行数
 * column:列数
 * return:初始化的迷宫
 * */
pMaze initMaze(int row, int column){
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
    return ret;
}

/**
 * 主方法
 * */
int main(){
    pMaze maze = initMaze(20, 40);
    maze->generate(maze, time(NULL));
    maze->print(maze);
    maze->save(maze, "maze");
    maze->free(maze);
    printf("\n\n\n");
    pMaze maze2 = initMazeByFile("maze");
    maze2->print(maze2);
    return 0;
}