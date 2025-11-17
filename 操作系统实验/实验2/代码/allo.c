#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESS_NAME_LEN 32
#define MIN_SLICE 10
#define DEFAULT_MEM_SIZE 1024
#define DEFAULT_MEM_START 0

#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

/*---------------------------------- 数据结构 ----------------------------------*/

struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type *next;
};

struct allocated_block {
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};

struct free_block_type *free_block = NULL;
struct allocated_block *allocated_block_head = NULL;

int mem_size = DEFAULT_MEM_SIZE;
int ma_algorithm = MA_FF;
static int pid = 0;
int flag = 0;

/*---------------------------------- 函数声明 ----------------------------------*/

struct free_block_type* init_free_block(int mem_size);
void display_menu();
int set_mem_size();
void set_algorithm();

void rearrange(int algorithm);
void rearrange_FF();
void rearrange_BF();
void rearrange_WF();

typedef int (*cmp_func)(struct free_block_type*, struct free_block_type*);
void sort_free_list(cmp_func cmp);

int allocate_mem(struct allocated_block *ab);
int free_mem(struct allocated_block *ab);

struct allocated_block* find_process(int pid);
void dispose(struct allocated_block *free_ab);

int new_process();
int kill_process();
int display_mem_usage();

/*---------------------------------- 初始化空闲区 ----------------------------------*/

struct free_block_type* init_free_block(int mem_size){
    struct free_block_type *fb =
        (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if(!fb){
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}

/*---------------------------------- 菜单 ----------------------------------*/

void display_menu(){
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process\n");
    printf("4 - Terminate a process\n");
    printf("5 - Display memory usage\n");
    printf("0 - Exit\n");
}

/*-------------------------------- 安全输入辅助 --------------------------------*/

int read_int(const char *prompt, int *out){
    char buf[128];
    int val;
    if(prompt) printf("%s", prompt);
    if(!fgets(buf, sizeof(buf), stdin)) return 0;
    if(sscanf(buf, "%d", &val) != 1) return 0;
    *out = val;
    return 1;
}

int read_choice_char(char *out){
    char buf[8];
    if(!fgets(buf, sizeof(buf), stdin)) return 0;
    int i = 0;
    while(buf[i] && (buf[i]==' ' || buf[i]=='\t' || buf[i]=='\r' || buf[i]=='\n')) i++;
    if(!buf[i]) return 0;
    *out = buf[i];
    return 1;
}

/*-------------------------------- 设置内存 --------------------------------*/

int set_mem_size(){
    int size;
    if(flag != 0){
        printf("Cannot set memory size again!\n");
        return 0;
    }
    if(!read_int("Total memory size = ", &size)){
        printf("Invalid input.\n");
        return 0;
    }
    if(size > 0){
        mem_size = size;
        if(free_block) {
            free_block->size = mem_size;
            free_block->start_addr = DEFAULT_MEM_START;
            free_block->next = NULL;
        } else {
            free_block = init_free_block(mem_size);
        }
    } else {
        printf("Size must be positive.\n");
        return 0;
    }
    flag = 1;
    return 1;
}

/*-------------------------------- 设置算法 --------------------------------*/

void set_algorithm(){
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit\n");
    printf("\t3 - Worst Fit\n");
    if(!read_int("Select algorithm: ", &algorithm)){
        printf("Invalid input.\n");
        return;
    }

    if(algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;
    else
        printf("Invalid algorithm number, keeping current.\n");

    rearrange(ma_algorithm);
}

/*------------------------------ 空闲块排序 ------------------------------*/

void sort_free_list(cmp_func cmp){
    struct free_block_type *i, *j;
    for(i = free_block; i && i->next; i = i->next){
        for(j = i->next; j; j = j->next){
            if(cmp(i, j)){
                int addr = i->start_addr;
                int size = i->size;
                i->start_addr = j->start_addr;
                i->size = j->size;
                j->start_addr = addr;
                j->size = size;
            }
        }
    }
}

int cmp_ff(struct free_block_type* a, struct free_block_type* b){
    return a->start_addr > b->start_addr;
}
int cmp_bf(struct free_block_type* a, struct free_block_type* b){
    return a->size > b->size;
}
int cmp_wf(struct free_block_type* a, struct free_block_type* b){
    return a->size < b->size;
}

void rearrange_FF(){ sort_free_list(cmp_ff); }
void rearrange_BF(){ sort_free_list(cmp_bf); }
void rearrange_WF(){ sort_free_list(cmp_wf); }

/*---------------------------------- 查找进程 ----------------------------------*/

struct allocated_block* find_process(int pid){
    struct allocated_block *p = allocated_block_head;
    while(p){
        if(p->pid == pid) return p;
        p = p->next;
    }
    return NULL;
}

/*---------------------------------- 紧缩内存 ----------------------------------*/

void compact_memory() {
    int addr = 0;
    struct allocated_block *ab = allocated_block_head;

    // 将已分配块按链表顺序紧凑
    while(ab){
        ab->start_addr = addr;
        addr += ab->size;
        ab = ab->next;
    }

    // 释放旧空闲块
    struct free_block_type *fb = free_block;
    while(fb){
        struct free_block_type *t = fb->next;
        free(fb);
        fb = t;
    }

    // 生成单一空闲块
    free_block = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if(!free_block){
        fprintf(stderr, "malloc failed in compact_memory\n");
        exit(-1);
    }
    free_block->start_addr = addr;
    free_block->size = mem_size - addr;
    free_block->next = NULL;

    rearrange(ma_algorithm);
}

/*---------------------------------- 分配内存 ----------------------------------*/

int allocate_mem(struct allocated_block *ab){
    int req = ab->size;
    struct free_block_type *p = free_block, *prev = NULL;

    while(p){
        if(p->size >= req){
            ab->start_addr = p->start_addr;

            if(p->size == req || (p->size - req) < MIN_SLICE){
                ab->size = p->size;
                if(prev) prev->next = p->next;
                else free_block = p->next;
                free(p);
            } else {
                p->start_addr += req;
                p->size -= req;
            }

            rearrange(ma_algorithm);
            return 1;
        }
        prev = p;
        p = p->next;
    }

    // 如果分配失败，尝试紧缩
    int sum = 0;
    for(p = free_block; p; p = p->next) sum += p->size;

    if(sum >= req){
        compact_memory();
        return allocate_mem(ab); // 紧缩后再次尝试
    }

    return -1; // 内存不足
}

/*---------------------------------- 回收内存 ----------------------------------*/

int free_mem(struct allocated_block *ab){
    struct free_block_type *p = free_block, *prev = NULL;

    struct free_block_type *newb = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if(!newb){
        fprintf(stderr, "malloc failed\n");
        return 0;
    }

    newb->start_addr = ab->start_addr;
    newb->size = ab->size;
    newb->next = NULL;

    while(p && p->start_addr < newb->start_addr){
        prev = p;
        p = p->next;
    }

    newb->next = p;
    if(prev) prev->next = newb;
    else free_block = newb;

    // merge previous
    if(prev && prev->start_addr + prev->size == newb->start_addr){
        prev->size += newb->size;
        prev->next = newb->next;
        free(newb);
        newb = prev;
    }

    // merge next
    if(p && newb->start_addr + newb->size == p->start_addr){
        newb->size += p->size;
        newb->next = p->next;
        free(p);
    }

    rearrange(ma_algorithm);
    return 1;
}

/*---------------------------------- 删除链表节点 ----------------------------------*/

void dispose(struct allocated_block *free_ab){
    if(allocated_block_head == NULL || free_ab == NULL) return;

    if(free_ab == allocated_block_head){
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return;
    }

    struct allocated_block *pre = allocated_block_head;
    struct allocated_block *ab  = allocated_block_head->next;

    while(ab && ab != free_ab){
        pre = ab;
        ab = ab->next;
    }

    if(ab == NULL) return;

    pre->next = ab->next;
    free(ab);
}

/*---------------------------------- 创建进程 ----------------------------------*/

int new_process(){
    struct allocated_block *ab;
    int size, ret;

    ab = (struct allocated_block*)malloc(sizeof(struct allocated_block));
    if(!ab) exit(-1);

    ab->next = NULL;
    pid++;
    snprintf(ab->process_name, PROCESS_NAME_LEN, "PROCESS-%02d", pid);
    ab->pid = pid;

    if(!read_int("Memory for process: ", &size)){
        printf("Invalid input.\n");
        free(ab);
        return -1;
    }
    ab->size = size;

    ret = allocate_mem(ab);

    if(ret == 1){
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        return 1;
    } else {
        printf("Allocation failed!\n");
        free(ab);
        return -1;
    }
}

/*---------------------------------- 删除进程 ----------------------------------*/

int kill_process(){
    int tpid;
    if(!read_int("Kill Process, PID = ", &tpid)) return 0;

    struct allocated_block *ab = find_process(tpid);
    if(ab){
        free_mem(ab);
        dispose(ab);
    } else {
        printf("PID not found!\n");
    }
    return 1;
}

/*---------------------------------- 显示内存 ----------------------------------*/

int display_mem_usage(){
    struct free_block_type *fbt = free_block;
    struct allocated_block *ab = allocated_block_head;

    printf("\n------------------ Memory Usage ------------------\n");

    printf("Free Memory:\n  start_addr        size\n");
    while(fbt){
        printf("%10d %12d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }

    printf("\nUsed Memory:\n");
    printf("PID   ProcessName   start_addr   size\n");
    while(ab){
        printf("%3d   %-12s   %10d   %5d\n",
            ab->pid, ab->process_name, ab->start_addr, ab->size);
        ab = ab->next;
    }

    printf("--------------------------------------------------\n");
    return 0;
}

/*---------------------------------- rearrange 总调用 ----------------------------------*/
void rearrange(int algorithm){
    switch(algorithm){
        case MA_FF: rearrange_FF(); break;
        case MA_BF: rearrange_BF(); break;
        case MA_WF: rearrange_WF(); break;
        default: break;
    }
}

/*---------------------------------- 主函数 ----------------------------------*/

int main(){
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size);

    while(1){
        display_menu();
        printf("Select: ");
        if(!read_choice_char(&choice)){
            printf("Input error or EOF — exiting.\n");
            break;
        }

        switch(choice){
            case '1': set_mem_size(); break;
            case '2': set_algorithm(); flag = 1; break;
            case '3': new_process(); flag = 1; break;
            case '4': kill_process(); flag = 1; break;
            case '5': display_mem_usage(); flag = 1; break;
            case '0': 
                // 清理内存
                while(allocated_block_head) dispose(allocated_block_head);
                while(free_block){
                    struct free_block_type *t = free_block->next;
                    free(free_block);
                    free_block = t;
                }
                exit(0);
            default: printf("Unknown choice.\n"); break;
        }
    }
    return 0;
}
