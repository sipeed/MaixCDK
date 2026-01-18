#include "dict.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

/***************************局部函数***********************************/
static int check_suffix(char* name, char* dst)
{
    int name_len = strlen(name);
    char *p = strchr(name, '.');
    if(p==NULL) {
        return -1;
    }
    return strcmp(p+1, dst);
}

/***************************文本形式打开，需要全部载入内存***********************************/
int dict_opentxt(char* dict_name, dict_t* dict)
{
    FILE* fp = fopen(dict_name, "r");
    if(!fp) {
        printf("open %s failed\n", dict_name);
        return -1;
    }
    fseek(fp,0L,SEEK_END);
    size_t dict_size = ftell(fp);
    char* dict_buf = (char*)malloc(dict_size);
    if(dict_buf==NULL) {
        printf("alloc dict buf %ld bytes failed\n", (long int)dict_size);
        goto free_dict_buf;
    }
    fseek(fp,0L,SEEK_SET);
    size_t read_cnt = fread(dict_buf, 1, dict_size, fp);
    if (read_cnt != dict_size) {
        printf("fread %ld bytes but get %ld bytes\n", (long int)dict_size, (long int)read_cnt);
        goto free_dict_buf;
    }
    
    //替换\n到0, 这样可以直接取字符串，注意，文件需要最后一行是空行
    uint32_t cnt = 0;
    for(uint32_t i = 0; i < dict_size; i++) {
        if(dict_buf[i] == '\n') {
            dict_buf[i] = 0;
            cnt += 1;
        }
    }
    //建立索引
    uint32_t* idx_buf = (uint32_t*)malloc(cnt*sizeof(uint32_t));
    if(idx_buf==NULL) {
        printf("alloc idx buf %u bytes failed\n", cnt*sizeof(uint32_t));
        goto free_dict_buf;
    }
    idx_buf[0] = 0;
    uint32_t _cnt = 0;
    for(uint32_t i = 0; i < dict_size-1; i++) {
        if(dict_buf[i] == 0) {
            _cnt += 1;
            idx_buf[_cnt] = i+1;    //记录偏移
        }
    }
    //存储入结构体
    dict->type     = DICT_TXT;
    dict->cnt      = cnt;
    dict->idx_buf  = idx_buf;
    dict->dict_buf = dict_buf;
    return 0;

free_dict_buf:
    free(dict_buf);
    fclose(fp);
    return -1;
}

void dict_closetxt(dict_t* dict)
{
    free(dict->idx_buf);
    free(dict->dict_buf);
    return;
}





/***************************bin形式打开，可以mmap减少内存占用***********************************/
int dict_openbin(char* dict_name, dict_t* dict)
{
    int fd, nread;
    struct stat sb;
    if((fd = open(dict_name, O_RDONLY)) < 0){
        printf("mmap open %s failed\n", dict_name);
        return -1;
    } 
    if((fstat(fd, &sb)) == -1 ){
        printf("fstat failed\n");
        return -1;
    }   
    uint32_t* dict_buf = (uint32_t*)mmap(\
        NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); //MAP_SHARED, MAP_PRIVATE
    if((void*)dict_buf ==(void*) -1){
        printf("mmap failed\n");
        close(fd); 
        return -1;
    } 
    close(fd); 

    //判读magic
    char* magic = (char*)dict_buf;
    if(magic[0]=='D' && magic[1]=='I' && magic[2]=='C' && magic[3]=='T' ) {
        dict_head_t* head = (dict_head_t*)dict_buf;
        //存储入结构体
        //索引部分每4字节表示对应下标的word的oft(相对dict_buf起始处，而不是总buf)
        dict->type     = DICT_BIN;
        dict->cnt      = head->word_cnt;
        dict->idx_buf  = (uint32_t*)((char*)dict_buf + head->idx_oft);  
        dict->dict_buf = (char*)dict_buf + head->word_oft;
        dict->size     = (size_t)sb.st_size;
        dict->bin_buf  = (char*)dict_buf;
        printf("get word_cnt = %d, idx_oft=%d, word_oft=%d\n", head->word_cnt, head->idx_oft, head->word_oft);
        return 0;
    } else {
        printf("sfst head magic not right, %c%c%c%c != SFST\n", \
            magic[0], magic[1], magic[2], magic[3]);
        return -1;
    }
}
void dict_closebin(dict_t* dict)
{
    munmap(dict->bin_buf, dict->size); 
    return;
}

/***************************字典操作***********************************/
int dict_open(char* dict_name, dict_t* dict)
{
    if(check_suffix(dict_name, "txt") == 0) {
        return dict_opentxt(dict_name, dict);
    } else if(check_suffix(dict_name, "bin") == 0) {
        return dict_openbin(dict_name, dict);
    } else {
        printf("unsupport format! dict only support txt&bin\n");
        return -1;
    }
}
void dict_close(dict_t* dict)
{
    if(dict->type == DICT_TXT) {
        dict_closetxt(dict);
    } else if(dict->type == DICT_BIN){
        dict_closebin(dict);
    } else {
        printf("unsupport format! dict only support txt&bin\n");
    }
    return ;
}

char* dict_get(dict_t* dict, uint32_t idx)
{
    uint32_t oft = dict->idx_buf[idx];
    return dict->dict_buf+oft;
}

void dict_dump(dict_t* dict)
{
    for(uint32_t i = 0; i < dict->cnt; i++) {
        printf("%-8d: %s\n", i, dict_get(dict, i));
    }
    printf("\n");
    return;
}


