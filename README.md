# MY ALLOC

This module provides functionalities for custom memory allocation, including multiple allocation strategies (First-Fit, Next-Fit, Best-Fit), architecture-specific memory alignment, and debugging support.

## Project Tree:

```
/my-alloc
│
├── /inc
│   └── libmemalloc.h
│
├── /src
│   └── libmemalloc.c
│
├── /test
│   └── unit_test.c
│
├── .gitattributes
├── .gitignore
├── LICENSE
├── README.md
└── makefile
```

# Memory Allocator Algorithms Overview

This document provides a detailed explanation of the algorithms employed in the custom memory allocator module. Understanding these algorithms is crucial for appreciating how the allocator manages memory efficiently, minimizes fragmentation, and optimizes performance.

## Table of Contents

1. [Allocation Strategies](#allocation-strategies)
    - [First-Fit](#first-fit)
    - [Next-Fit](#next-fit)
    - [Best-Fit](#best-fit)
2. [Block Management](#block-management)
    - [Split Block](#split-block)
    - [Merge Blocks](#merge-blocks)
3. [FitBlock Process](#fitblock-process)
4. [Rationale for Algorithm Selection](#rationale-for-algorithm-selection)
5. [Summary](#summary)
6. [References](#references)

# Allocation Strategies

Memory allocation strategies determine how the allocator searches for free memory blocks to satisfy allocation requests. The chosen strategy impacts allocation speed, memory utilization, and fragmentation levels. The custom allocator implements three primary strategies:

## First-Fit

### Description:
The First-Fit algorithm scans the free list from the beginning and selects the first block that is large enough to satisfy the allocation request.

### Implementation:

```c
int MEM_findFirstFit(mem_allocator_t *allocator, size_t size, block_header_t **fit_block) 
{
    int ret = 0u;

    block_header_t *current = NULL;

    if (allocator == NULL || fit_block == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    current = allocator->free_list;

    while (current) 
    {
        if (current->free && current->size >= size + sizeof(block_header_t)) {
            *fit_block = current;
            goto end_of_function;
        }

        current = current->next;
    }

    ret = ENOMEM;

end_of_function:
    return ret;
}
```

### Advantages:

- Speed:
    - Generally faster than Best-Fit as it stops searching upon finding the first suitable block.
  
- Simplicity:
    - Easy to implement and understand.
  
### Disadvantages:

- Fragmentation:
    - Can lead to higher fragmentation over time, as small unusable gaps accumulate.

## Next-Fit
###  Description:

The Next-Fit algorithm resumes the search from where the last allocation occurred, rather than starting from the beginning each time.

### Implementation:

```c
int MEM_findNextFit(mem_allocator_t *allocator, size_t size, block_header_t **fit_block) 
{
    int ret                     = 0u;

    block_header_t *current     = NULL;
    block_header_t *start       = NULL;

    if (allocator == NULL || fit_block == NULL)
    {
        ret = EINVAL;
        goto end_of_function;
    }

    current     = allocator->last_allocated;
    start       = current;

    do 
    {
        if (current->free && current->size >= size + sizeof(block_header_t)) 
        {
            *fit_block                  = current;
            allocator->last_allocated   = current;

            goto end_of_function;
        }

        current = current->next ? current->next : (block_header_t *)allocator->heap;
    } while (current != start);


    ret = ENOMEM;

end_of_function:
    return ret;
}
```

### Advantages:

Reduced Fragmentation: More evenly distributes allocations across the heap, potentially reducing fragmentation compared to First-Fit.
Fairness: Prevents starvation of memory blocks by not always starting from the beginning.
Disadvantages:

Slightly Slower Than First-Fit: May traverse more blocks if the next suitable block is farther from the last allocation point.

## Best-Fit
### Description:

The Best-Fit algorithm searches the entire free list to find the smallest block that can satisfy the allocation request, aiming to minimize wasted space.

### Implementation:

```c
int MEM_findBestFit(mem_allocator_t *allocator, size_t size, block_header_t **best_fit) 
{
    int ret                 = 0u;

    block_header_t *current = NULL;

    if (allocator == NULL || best_fit == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    *best_fit   = NULL;
    current     = allocator->free_list;

    while (current) 
    {
        if (current->free && current->size >= size + sizeof(block_header_t)) 
        {
            if (*best_fit == NULL || current->size < (*best_fit)->size) 
            {
                *best_fit = current;
            }
        }

        current = current->next;
    }

    if (*best_fit == NULL) 
    {
        ret = ENOMEM;
    }

end_of_function:
    return ret;
}
```

### Advantages:

Minimized Fragmentation: By selecting the smallest sufficient block, Best-Fit reduces the likelihood of large unusable gaps.
Efficient Memory Utilization: Helps in utilizing memory more effectively.
Disadvantages:

Slower Allocation: Must traverse the entire free list to find the best fit, which can be time-consuming for large free lists.
Potential for Small Fragments: May create many small free blocks, which could eventually lead to increased fragmentation.

# Block Management

Efficient memory allocation and deallocation require effective management of memory blocks. The allocator employs Block Splitting and Block Merging to optimize memory usage and reduce fragmentation.

## Split Block
### Description:

Block Splitting involves dividing a larger free memory block into two smaller blocks when only a portion of the block is needed for an allocation. This ensures that the remaining memory is not wasted.

### Implementation:

```c
int MEM_splitBlock(mem_allocator_t *allocator, block_header_t *block, size_t size) 
{
    int ret                     = 0u;

    size_t aligned_size         = 0u;

    uint8_t *new_block_addr     = NULL;
    block_header_t *new_block   = NULL;

    if (allocator == NULL || block == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    aligned_size = ALIGN(size);

    if (block->size >= aligned_size + sizeof(block_header_t) + ARCH_ALIGNMENT) 
    {
        new_block_addr      = (uint8_t *)block + sizeof(block_header_t) + aligned_size;
        new_block           = (block_header_t *)new_block_addr;

        new_block->size     = block->size - sizeof(block_header_t) - aligned_size;
        new_block->free     = 1u;
        new_block->file     = NULL;
        new_block->line     = 0u;
        new_block->var_name = NULL;
        new_block->next     = block->next;
        new_block->prev     = block;

        block->size         = aligned_size + sizeof(block_header_t);
        block->free         = 0u;

        if (new_block->next) 
        {
            new_block->next->prev = new_block;
        }

        block->next = new_block;

        if (allocator->free_list == block) 
        {
            allocator->free_list = new_block;
        }

        MEM_printd("MEM_splitBlock: Split block. New block at %p with size %zu bytes.\n",
                   (void *)new_block, new_block->size);
    } 
    else 
    {
        block->free = 0u;
        MEM_printd("MEM_splitBlock: Block at %p not split. Marked as allocated.\n", (void *)block);
    }

end_of_function:
    return ret;
}
```

###  Purpose and Benefits:

Efficient Memory Utilization: Prevents wastage by allocating only the required memory and retaining the unused portion for future allocations.
Flexibility: Allows the allocator to handle varying allocation sizes dynamically.
Considerations:

Minimum Block Size: Ensures that the remaining free block after splitting is large enough to be useful (ARCH_ALIGNMENT in this case).

## Merge Blocks
### Description:

Block Merging (or Coalescing) combines adjacent free memory blocks into a single larger block. This process reduces fragmentation by eliminating small unusable gaps between allocations.

### Implementation:

```c
int MEM_mergeBlocks(mem_allocator_t *allocator, block_header_t *block) 
{
    int ret = 0u;

    block_header_t *next_block = NULL;

    if (allocator == NULL || block == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    next_block = (block_header_t *)((uint8_t *)block + block->size);
    if ((uint8_t *)next_block < allocator->heap + HEAP_SIZE && next_block->free) 
    {
        block->size += next_block->size;
        block->next = next_block->next;

        if (next_block->next) 
        {
            next_block->next->prev = block;
        }

        MEM_printd("MEM_mergeBlocks: Merged with next block. New size: %zu bytes.\n", block->size);
    }

    if (block->prev && block->prev->free) 
    {
        block->prev->size += block->size;
        block->prev->next = block->next;

        if (block->next) 
        {
            block->next->prev = block->prev;
        }

        block = block->prev;

        MEM_printd("MEM_mergeBlocks: Merged with previous block. New size: %zu bytes.\n", block->size);
    }

    block->file     = NULL;
    block->line     = 0u;
    block->var_name = NULL;

end_of_function:
    return ret;
}
```

### Purpose and Benefits:

Reduced Fragmentation: By merging adjacent free blocks, the allocator maintains larger contiguous free spaces, which are more likely to satisfy future allocation requests.
Improved Memory Availability: Larger free blocks increase the chances of fulfilling large memory allocation requests without needing to expand the heap.
Considerations:

Adjacency: Only adjacent free blocks can be merged. The allocator must ensure that the memory layout maintains block adjacency correctly.
Update Pointers: After merging, the free list pointers (next and prev) must be updated to reflect the new block structure accurately.

# FitBlock Process

The FitBlock process refers to the strategy employed to select an appropriate free block that can accommodate a memory allocation request. Depending on the chosen allocation strategy (First-Fit, Next-Fit, Best-Fit), the allocator traverses the free list differently to find the most suitable block.

## Workflow:

### Receive Allocation Request: 
The allocator receives a request to allocate a specific size of memory.

### Select Allocation Strategy:
Based on the strategy (e.g., First-Fit), the allocator decides how to traverse the free list.

### Traverse Free List:

  - First-Fit: 
      - Starts from the beginning and selects the first block that fits.
  
  - Next-Fit:
      - Starts from the last allocated block and continues searching forward.
  
  - Best-Fit: 
      - Traverses the entire free list to find the smallest block that fits.

### Select Block:

If a suitable block is found, proceed to allocate it.
If no block fits, return an allocation failure.

### Split Block (if necessary): 
If the selected block is larger than needed, split it into an allocated block and a smaller free block.

### Update Metadata: 
Mark the allocated block as used and update relevant metadata (e.g., file name, line number).

### Return Pointer: 
Provide the user with a pointer to the allocated memory.

## Example Implementation for First-Fit:

```c
void *MEM_allocatorMalloc(mem_allocator_t *allocator, size_t size, const char *file, int line, const char *var_name, allocation_strategy_t strategy) 
{
    int ret                 = 0u;

    void *user_ptr          = NULL;

    block_header_t *block   = NULL;

    if (allocator == NULL) 
    {
        errno       = EINVAL;
        user_ptr    = NULL;

        goto end_of_function;
    }

    if (size == 0u) 
    {
        errno       = EINVAL;
        user_ptr    = NULL;

        goto end_of_function;
    }

    size_t aligned_size = ALIGN(size);

    switch (strategy) 
    {
        case FIRST_FIT:
            ret = MEM_findFirstFit(allocator, aligned_size, &block);
            break;
        case NEXT_FIT:
            ret = MEM_findNextFit(allocator, aligned_size, &block);
            break;
        case BEST_FIT:
            ret = MEM_findBestFit(allocator, aligned_size, &block);
            break;
        default:
            fprintf(stderr, "MEM_allocatorMalloc: Unknown allocation strategy.\n");
            errno = EINVAL;
            return NULL;
    }

    if (ret != 0 || block == NULL) 
    {
        fprintf(stderr, "MEM_allocatorMalloc: No sufficient free block to allocate %zu bytes for variable '%s' (in %s:%d)\n", 
                size, var_name, file, line);

        errno       = EINVAL;
        user_ptr    = NULL;

        goto end_of_function;
    }

    ret = MEM_splitBlock(allocator, block, aligned_size);
    if (ret != 0u) 
    {
        fprintf(stderr, "MEM_allocatorMalloc: Failed to split block for variable '%s' (in %s:%d)\n", 
                var_name, file, line);

        errno       = EINVAL;
        user_ptr    = NULL;

        goto end_of_function;
    }

    user_ptr = (void *)((uint8_t *)block + sizeof(block_header_t));

    block->file = file;
    block->line = line;
    block->var_name = var_name;

    MEM_printd("MEM_allocatorMalloc: Allocated %zu bytes for variable '%s' at %p (in %s:%d) using strategy %d.\n", 
               size, var_name, user_ptr, file, line, strategy);

end_of_function:
    return user_ptr;
}

```

# Rationale for Algorithm Selection

Choosing the appropriate memory allocation strategy is pivotal for balancing allocation speed, memory utilization, and fragmentation. Here's why each algorithm is utilized in the custom memory allocator:

### First-Fit
Use Case: Ideal for scenarios where allocation speed is a priority, and fragmentation is less of a concern.
Reasoning: Its simplicity and speed make it suitable for applications with frequent allocations and deallocations where the overhead of searching is minimal.

### Next-Fit
Use Case: Beneficial in environments where allocations are distributed more uniformly across the heap.
Reasoning: By continuing the search from the last allocation point, Next-Fit can reduce clustering at the beginning of the heap, potentially lowering fragmentation compared to First-Fit.

### Best-Fit
Use Case: Suitable for applications where memory fragmentation needs to be minimized, and allocation speed is less critical.
Reasoning: Best-Fit optimizes memory usage by selecting the smallest suitable block, thereby conserving larger blocks for future allocations and reducing wasted space.

### Block Splitting and Merging
Use Case: Essential for all allocation strategies to maintain an efficient free list and minimize fragmentation.

### Reasoning:
Splitting: Allows the allocator to utilize large free blocks effectively by dividing them as needed, ensuring that memory is not wasted.
Merging: Combines adjacent free blocks to form larger blocks, which is crucial for accommodating larger allocation requests and preventing small, unusable gaps.

# Summary

The custom memory allocator employs a combination of allocation strategies and block management techniques to efficiently manage memory within a predefined heap. By implementing First-Fit, Next-Fit, and Best-Fit strategies, the allocator offers flexibility in balancing allocation speed and memory utilization. Additionally, the mechanisms for splitting and merging blocks ensure that memory fragmentation is minimized, enhancing the overall performance and reliability of the allocator.

Understanding these algorithms allows developers to optimize their use of the allocator based on the specific needs and characteristics of their applications, ensuring efficient and effective memory management.

# References
[The Garbage Collection Handbook: The art of automatic memory management](https://gchandbook.org)
