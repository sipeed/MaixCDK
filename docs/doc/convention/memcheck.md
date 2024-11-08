---
title: Memory Check Method
---

## Introduction

Use the **Valgrind** tool to check for memory leaks, out-of-bounds memory access, and other memory-related issues.

## Installation

```shell
sudo apt install valgrind
```

## Using Valgrind

```shell
cd examples/hello_world
maixcdk build

# Output a brief log
valgrind ./build/hello_world

# Output a detailed log
# --tool=memcheck: Specifies the tool as memcheck. Memcheck is the default tool, so this option can be omitted.
# --leak-check=full: Displays detailed information for each memory leak.
# --log-file=valgrind.log: Outputs the memory leak check result to the specified file. Here, valgrind.log is a custom filename.
valgrind --tool=memcheck --leak-check=full --log-file=valgrind.log ./build/hello_world
```

## Analyzing Valgrind Output

### Checking Memory Leak Summary

In case of memory leaks, you may see logs similar to the following:

```shell
LEAK SUMMARY:
   definitely lost: 48 bytes in 3 blocks.   # Memory definitely leaked (e.g., memory allocated by a pointer not freed)
   indirectly lost: 32 bytes in 2 blocks.   # Indirectly leaked memory (e.g., a double pointer's memory not freed, causing indirect leakage)
     possibly lost: 96 bytes in 6 blocks.
   still reachable: 64 bytes in 4 blocks.
        suppressed: 0 bytes in 0 blocks.
```

### Checking Detailed Memory Errors

There are various types of memory errors that Valgrind can detect. Here is an example log. For more details, refer to the [official documentation](https://valgrind.org/docs/manual/mc-manual.html).

```shell
8 bytes in 1 blocks are definitely lost in loss record 1 of 14  # Identifies where memory is leaked
   at 0x........: malloc (vg_replace_malloc.c:...)
   by 0x........: mk (leak-tree.c:11)
   by 0x........: main (leak-tree.c:39)

88 (8 direct, 80 indirect) bytes in 1 blocks are definitely lost in loss record 13 of 14
   at 0x........: malloc (vg_replace_malloc.c:...)
   by 0x........: mk (leak-tree.c:11)
   by 0x........: main (leak-tree.c:25)
```

## Troubleshooting Unresolved Issues

If you encounter issues that cannot be resolved, refer to the [official documentation](https://valgrind.org).

