内存检查方法
===

## 简介

使用valgrind工具检查内存泄露、内存越界等问题

## 安装

```shell
sudo apt install valgrind
```

## 使用valgrind

```shell
cd examples/hello_world
maixcdk build

# 输出简要日志
valgrind ./build/hello_world

# 输出更详细日志
# --tool=memcheck：指定工具为memcheck, memcheck是默认工具，该参数可以省略
# --leak-check=full：详细的显示每个单独的内存泄露信息
# --log-file=valgrind.log：将内存泄露检查结果输出到指定文件, valgrind.log为自定义的文件名
valgrind --tool=memcheck --leak-check=full --log-file=valgrind.log ./build/hello_world
```

## 观察valgrind输出日志的方法

### 观察内存泄露摘要信息
内存泄露时可以看到类似如下日志
```shell
LEAK SUMMARY:
   definitely lost: 48 bytes in 3 blocks.   # 一定泄露的内存，例如一级指针指向的内存没有free
   indirectly lost: 32 bytes in 2 blocks.   # 间接泄露的内存，例如二级指针指向的内存没有free，间接导致二级指针指向的一级指针指向的内存没有free
     possibly lost: 96 bytes in 6 blocks.
   still reachable: 64 bytes in 4 blocks.
        suppressed: 0 bytes in 0 blocks.
```

### 观察内存异常详细信息
内存操作有多种异常，可以看到类似如下日志，详情见[官方文档](https://valgrind.org/docs/manual/mc-manual.html)查看具体含义。
```shell
8 bytes in 1 blocks are definitely lost in loss record 1 of 14  # 找到内存泄露的位置
   at 0x........: malloc (vg_replace_malloc.c:...)
   by 0x........: mk (leak-tree.c:11)
   by 0x........: main (leak-tree.c:39)

88 (8 direct, 80 indirect) bytes in 1 blocks are definitely lost in loss record 13 of 14
   at 0x........: malloc (vg_replace_malloc.c:...)
   by 0x........: mk (leak-tree.c:11)
   by 0x........: main (leak-tree.c:25)
```

## 遇到无法解决的问题

见[官方文档](https://valgrind.org)