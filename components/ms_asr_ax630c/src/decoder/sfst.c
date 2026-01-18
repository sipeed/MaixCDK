#include "sfst.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

//载入sfst文件，可选是否以mmap形式
int sfst_open(char* sfst_name, char* sym_name, int is_mmap, uint8_t** _sfst_buf, uint32_t** _sym_buf, size_t* _sfst_size, size_t* _sym_size)
{
    // sfst select mmap or directly read to memory
    if(is_mmap == 0) {  //直接读入缓存
        FILE* fp = fopen(sfst_name, "r");
        if(!fp) {
            printf("open %s failed\n", sfst_name);
            return -1;
        }
        fseek(fp,0L,SEEK_END);
        size_t sfst_size = ftell(fp);
        uint8_t* sfst_buf = (uint8_t*)malloc(sfst_size);
        if(sfst_buf==NULL) {
            printf("alloc buf %ld bytes failed\n", (long int)sfst_size);
            return -1;
        }
        fseek(fp,0L,SEEK_SET);
        size_t read_cnt = fread(sfst_buf, 1, sfst_size, fp);
        if (read_cnt != sfst_size) {
            printf("fread %ld bytes but get %ld bytes\n", (long int)sfst_size, (long int)read_cnt);
            return -1;
        }
        fclose(fp);
        *_sfst_size = (size_t)sfst_size;
        //判读magic
        char* magic = (char*)sfst_buf;
        sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
        if(magic[0]=='S' && magic[1]=='F' && magic[2]=='S' && magic[3]=='T' && (sfst_head->version == 1)) {
            *_sfst_buf = sfst_buf;
        } else {
            printf("sfst head magic or version not right, %c%c%c%c != SFST, or %d != 1\n", \
                magic[0], magic[1], magic[2], magic[3], sfst_head->version);
            return -1;
        }
    } else {
        int fd, nread;
        struct stat sb;
        if((fd = open(sfst_name, O_RDONLY)) < 0){
            printf("mmap open %s failed\n", sfst_name);
            return -1;
        } 
        if((fstat(fd, &sb)) == -1 ){
            printf("fstat failed\n");
            return -1;
        }   
        uint8_t* sfst_buf = (uint8_t*)mmap(\
            NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); //MAP_SHARED, MAP_PRIVATE
        if((void*)sfst_buf ==(void*) -1){
            printf("mmap failed\n");
            close(fd); 
            return -1;
        } 
        close(fd); 
        *_sfst_size = (size_t)sb.st_size;
        //判读magic
        char* magic = (char*)sfst_buf;
        sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
        if(magic[0]=='S' && magic[1]=='F' && magic[2]=='S' && magic[3]=='T' && (sfst_head->version == 1)) {
            *_sfst_buf = sfst_buf;
        } else {
            printf("sfst head magic or version not right, %c%c%c%c != SFST, or %d != 1\n", \
                magic[0], magic[1], magic[2], magic[3], sfst_head->version);
            return -1;
        }
    }

    //symout file mmap to reduce memory usage
    {
        int fd, nread;
        struct stat sb;
        if((fd = open(sym_name, O_RDONLY)) < 0){
            printf("mmap open %s failed\n", sym_name);
            goto free_sfst;
        } 
        if((fstat(fd, &sb)) == -1 ){
            printf("fstat failed\n");
            goto free_sfst;
        }   
        uint32_t* sym_buf = (uint32_t*)mmap(\
            NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); //MAP_SHARED, MAP_PRIVATE
        if((void*)sym_buf ==(void*) -1){
            printf("mmap failed\n");
            close(fd); 
            goto free_sfst;
        } 
        close(fd); 
        *_sym_size = (size_t)sb.st_size;
        *_sym_buf = sym_buf;
    }
    return 0;

free_sfst:
    if(is_mmap == 0) {
        free(*_sfst_buf);
    } else {
        munmap(*_sfst_buf, *_sfst_size); 
    }
    return -1;
}
    
//释放sfst缓存或者ummap
void sfst_close(int is_mmap, uint8_t* sfst_addr, uint32_t* sym_addr, size_t sfst_size, size_t sym_size)
{
    if(is_mmap == 0){
        free(sfst_addr);
    } else {
        munmap(sfst_addr, sfst_size); 
    }
    munmap(sym_addr, sym_size); 
    return;
}

/*
typedef struct {
    uint32_t state_in;
    uint32_t start_oft;
    uint32_t end_oft;
    uint32_t cur_oft;
}sfst_iter_t;
*/
//iter state's arcs
int sfst_iter_state_init(uint8_t* sfst_buf, uint32_t state_in, sfst_iter_t* sfst_iter)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    if(state_in >= sfst_head->state_cnt) {
        printf("state in %d >= state cnt %d invalid\n", \
            state_in, sfst_head->state_cnt);
        return -1;
    }
    uint32_t* sfst_buf_u32 = (uint32_t*)sfst_buf;
    uint32_t start_oft = sfst_buf_u32[sfst_head->state_oft/4 + state_in];
    uint32_t end_oft;
    if(state_in == (sfst_head->state_cnt-1)){
        end_oft = sfst_head->arc_oft+sfst_head->arc_cnt*ARC_SIZE;
    } else {
        end_oft = sfst_buf_u32[sfst_head->state_oft/4 + state_in+1];
    }
    sfst_iter->state_in = state_in;
    sfst_iter->start_oft= sfst_buf+start_oft;
    sfst_iter->end_oft  = sfst_buf+end_oft;
    sfst_iter->cur_oft  = sfst_buf+start_oft;
    return 0;
}




#if ARC_SIZE==5
/*
S0S1S2 S3|C0|T0 T1	5byte  S3:2|C0:3|T0:3  S:26bit
S表示state_out, 总共分配 24+2 = 26bit = 67M个 (测试lg8 state数小于40M，lg7 state数小于6M)
T表示sym_in,  总共分配 8+3 = 11bit = 2048个
C表示cost，总共分配3bit，最大8个状态，0,1,2,3,4,5,6,7   0.5压缩，即最大截断到14
*/
//a603000000: 10879910,0,0.0

#define STATE_BITS 26
#define COST_BITS  3
#define SYMIN_BITS 11
#define COST_SCALE (1/0.5)

#define SYMIN_LOW  (SYMIN_BITS-8)
#define STATE_MOD  ((1<<(STATE_BITS-24))-1)
#define COST_SHIFT (STATE_BITS-24)
#define COST_MOD   ((1<<COST_BITS)-1)

static inline void _parse_arc(uint8_t* data, uint32_t* state_out, uint32_t* sym_in, float* cost)
{
    *state_out = \
        ((uint32_t)data[0])|(((uint32_t)data[1])<<8)|(((uint32_t)data[2])<<16)|(((uint32_t)(data[3]&STATE_MOD))<<24);
        //((uint32_t)data[0])|(((uint32_t)data[1])<<8)|(((uint32_t)data[2])<<16)|(((uint32_t)(data[3]&0x03))<<24);
    //int qcost = (data[3]>>2)&0x07;
    //*cost= qcost*2.0;  //0~7 -> 0.0~14.0 
    int qcost = (data[3]>>COST_SHIFT)&(COST_MOD);
    *cost= qcost*COST_SCALE;  //0~15 
    *sym_in=  (data[3]>>5) + (((int)data[4])<<3);
    //printf("%02x%02x%02x%02x%02x: %d,%d,%.1f\n", data[0], data[1], data[2], data[3], data[4], *state_out, *sym_in, *cost);
    return;
}
#elif ARC_SIZE==6
/*
# 6字节版
S0S1S2 S3|T0 T1 C	6byte  S3:2|T0:6  C:8bit
state 总共26bit, 64M=0.64亿状态 （考虑到最大4GB大小，64*6=3.84GB，够用）
sym_in总共14bit, 16384种输入
cost 8bit量化
*/
#define STATE_BITS 28
#define SYMIN_BITS 12
#define COST_SCALE (1/32.0)

#define SYMIN_LOW  (SYMIN_BITS-8)
#define STATE_MOD  ((1<<(STATE_BITS-24))-1)
#define COST_SHIFT (STATE_BITS-24)
#define COST_MOD   ((1<<COST_BITS)-1)

static inline void _parse_arc(uint8_t* data, uint32_t* state_out, uint32_t* sym_in, float* cost)
{
    *state_out = \
        ((uint32_t)data[0])|(((uint32_t)data[1])<<8)|(((uint32_t)data[2])<<16)|(((uint32_t)(data[3]&STATE_MOD))<<24);
    *sym_in=  (data[3]>>COST_SHIFT) + (((int)data[4])<<SYMIN_LOW);
    //int qcost = (data[3]>>2)&0x07;
    //*cost= qcost*2.0;  //0~7 -> 0.0~14.0 
    int qcost = ((int)(data[5]));
    *cost= qcost*COST_SCALE; //(1/32.0);  //0~15 
    //printf("%02x%02x%02x%02x%02x: %d,%d,%.1f\n", data[0], data[1], data[2], data[3], data[4], *state_out, *sym_in, *cost);
    return;
}

#elif ARC_SIZE==9
/*
# 6字节版
S0S1S2 S3|T0 T1 C	6byte  S3:2|T0:6  C:8bit
state 总共26bit, 64M=0.64亿状态 （考虑到最大4GB大小，64*6=3.84GB，够用）
sym_in总共14bit, 16384种输入
cost 8bit量化
*/
static inline void _parse_arc(uint8_t* data, uint32_t* state_out, uint32_t* sym_in, float* cost)
{
    *state_out = \
        ((uint32_t)data[0])|(((uint32_t)data[1])<<8)|(((uint32_t)data[2])<<16)|(((uint32_t)(data[3]&0x03))<<24);
    *sym_in=  (data[3]>>2) + (((int)data[4])<<6);
    //int qcost = (data[3]>>2)&0x07;
    //*cost= qcost*2.0;  //0~7 -> 0.0~14.0 
    uint8_t* p= (uint8_t*)(cost);
    p[0] = data[5];
    p[1] = data[6];
    p[2] = data[7];
    p[3] = data[8];
    //printf("%02x%02x%02x%02x%02x: %d,%d,%.1f\n", data[0], data[1], data[2], data[3], data[4], *state_out, *sym_in, *cost);
    return;
}

#endif

/*
typedef struct {
    uint8_t* arc_addr;
    uint32_t state_out;
    uint32_t sym_in;
    float    cost;
}sfst_arc_t;
*/
int sfst_iter_state(sfst_iter_t* sfst_iter, sfst_arc_t* sfst_arc)
{
    uint8_t* data = sfst_iter->cur_oft;
    if(data >= sfst_iter->end_oft) return 0;    //end

    _parse_arc(data, &sfst_arc->state_out, &sfst_arc->sym_in, &sfst_arc->cost);
    sfst_arc->arc_addr = sfst_iter->cur_oft;
    sfst_arc->sym_out  = 0xffffffff; //don't fill correct sym_out, as it is not used at this time

    sfst_iter->cur_oft += ARC_SIZE;
    return 1;
}

//从arc地址查询idx
uint32_t sfst_get_arc_idx(uint8_t* sfst_buf, uint8_t* arc_addr)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    uint32_t idx = (arc_addr - (uint8_t*)(sfst_buf+sfst_head->arc_oft))/ARC_SIZE;
    return idx;
}


//返回对应arc_idx的arc_buf
uint8_t sfst_first_arc_data[5] = {0, 0, 0, 0, 0}; //dummy init arc, point to state 0
sfst_arc_t sfst_first_arc = {sfst_first_arc_data, 0, 0, 0};

int sfst_get_arc(uint8_t* sfst_buf, uint32_t* sym_buf, uint32_t arc_idx, sfst_arc_t* sfst_arc)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    if(arc_idx == INIT_ARC_IDX) {
        sfst_arc->arc_addr = sfst_first_arc_data;
        sfst_arc->state_out= 0;
        sfst_arc->sym_in   = 0;
        sfst_arc->sym_out  = 0xffffffff;
        sfst_arc->cost     = 0;
        return 0;
    }
    if(arc_idx >= sfst_head->arc_cnt) {
        printf("arc_idx %d >= arc_cnt %d invalid\n", \
            arc_idx, sfst_head->arc_cnt);
        return -1;
    }

    sfst_arc->arc_addr = sfst_buf + sfst_head->arc_oft + arc_idx*ARC_SIZE;
    _parse_arc(sfst_arc->arc_addr, &sfst_arc->state_out, &sfst_arc->sym_in, &sfst_arc->cost);
    
    if(sym_buf != NULL) sfst_arc->sym_out  = sym_buf[arc_idx];
    else sfst_arc->sym_out = 0xffffffff;
    return 0;
}


//查找某状态的终止cost 
float sfst_get_finalcost(uint8_t* sfst_buf, uint32_t state_in)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    if(state_in >= sfst_head->state_cnt) {
        printf("state in %d >= state cnt %d invalid\n", \
            state_in, sfst_head->state_cnt);
        return 0.0;
    }
    
    uint32_t oft;
    float cost;
    if(state_in < sfst_head->state_cnt-1) { //非最后一个，查找下一个状态的前一项
        uint32_t* p = (uint32_t*)sfst_buf;
        oft = p[sfst_head->state_oft/4 + state_in + 1] - ARC_SIZE; // -1就是前一项
    } else { //最后一个，直接查看最后一项是否为final状态
        oft = sfst_head->arc_oft+ (sfst_head->arc_cnt-1)*ARC_SIZE;
    }

    sfst_arc_t sfst_arc;
    _parse_arc(sfst_buf+oft, &sfst_arc.state_out, &sfst_arc.sym_in, &sfst_arc.cost);
    //printf("state_in=%d, state_out=%d, sym_in=0x%x, sym_out=0x%x, cost=%f\n", sfst_arc->state_in, sfst_arc->state_out, sfst_arc->sym_in, sfst_arc->sym_out, sfst_arc->cost);
    if((state_in == sfst_arc.state_out) && (sfst_arc.sym_in == SYM_FINAL)) { //该状态有final状态
        cost = sfst_arc.cost;
    } else {
        cost = COST_INF;
    }
    return cost;
}

//获取状态数
uint32_t sfst_get_state_cnt(uint8_t* sfst_buf)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    return sfst_head->state_cnt;
}    

//获取总arc数
uint32_t sfst_get_arc_cnt(uint8_t* sfst_buf)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    return sfst_head->arc_cnt;
}  


//打印当前sfst的状态偏移表
int sfst_print_state(uint8_t* sfst_buf)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    printf("Dump state-oft table\n");
    printf("state_cnt=%d, state_oft=%d; arc_cnt=%d, arc_oft=%d\n", \
        sfst_head->state_cnt, sfst_head->state_oft, sfst_head->arc_cnt, sfst_head->arc_oft);
    printf("state  oft(uint32_t unit)\n");
    printf("==============================================\n");
    uint32_t* oft_table = (uint32_t*)(sfst_buf + sfst_head->state_oft);
    int i = 0;
    for(; i+3 < sfst_head->state_cnt; i+=4) {
        printf("%6d:%-6d, %6d:%-6d, %6d:%-6d, %6d:%-6d,\n", \
            i,oft_table[i], i+1,oft_table[i+1], i+2,oft_table[i+2], i+3,oft_table[i+3]);
    }
    for(; i < sfst_head->state_cnt; i++) {
        printf("%6d:%-6d, ", i,oft_table[i]);
    }
    printf("\n");
    return 0;
}

//打印当前state的arc表
int sfst_print_state_arc(uint8_t* sfst_buf, uint32_t* sym_buf, uint32_t state_in)
{
    sfst_head_t* sfst_head = (sfst_head_t*)sfst_buf;
    if(state_in >= sfst_head->state_cnt) {
        printf("state in %d >= state cnt %d invalid\n", \
            state_in, sfst_head->state_cnt);
        return -1;
    }
    printf("stat %d==============================================\n", state_in);
    sfst_iter_t sfst_iter;
    int res = sfst_iter_state_init(sfst_buf, state_in, &sfst_iter);
    if(res != 0) return -1;
    sfst_arc_t arc;
    while(sfst_iter_state(&sfst_iter, &arc)) { 
        if(arc.state_out == state_in) continue; //skip final arc  
        int idx = sfst_get_arc_idx(sfst_buf, sfst_iter.cur_oft)-1;
        printf("%7d,%7d,%7d,%7d,  %.6f\n", \
            state_in, arc.state_out, arc.sym_in, sym_buf[idx], arc.cost);

    }
    printf("\n");
    return 0;
}    




