/**
 *  @ingroup    MemoryManagement
 *  @addtogroup MemoryAllocator MEM_alloc
 *  @{
 *
 *  @package    MEM_alloc
 *  @brief      This module provides functionalities for custom memory allocation,
 *              including multiple allocation strategies (First-Fit, Next-Fit, Best-Fit),
 *              architecture-specific memory alignment, and debugging support.
 *
 *  @file       libmemalloc.c
 *  @author     Rafael V. Volkmer (Rafael.v.volkmer@gmail.com)
 * 
 *  @date       14.10.2024
 *
 *  @details    
 *              The Memory Allocator module manages dynamic memory allocation within a predefined
 *              heap space. It supports various allocation strategies to optimize memory usage
 *              and reduce fragmentation. The module ensures proper memory alignment based on
 *              the target architecture, enhancing performance and compatibility. Additionally,
 *              it includes debugging features that track allocation sources, facilitating easier
 *              debugging and memory management.
 *
 *  @note       
 *              - Ensure that the allocator is initialized before performing any allocation or deallocation.
 *              - The module is designed for single-threaded environments. For multi-threaded applications,
 *                synchronization mechanisms should be implemented to ensure thread safety.
 *
 *  @see        - libmemalloc.h
 *              - MEM_allocatorInit
 *              - MEM_allocatorMalloc 
 *              - MEM_allocatorFree
 **/

/* =================================
 *       PRIVATE INCLUDE FILES     *
 * ================================*/

/* dependencies: */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/* implements: */
#include <libmemalloc.h>

/* =================================
 *     PRIVATE GLOBAL VARIABLE     *
 * ================================*/

/**
 * @var     heap_memory
 * @package MEM_alloc
 * 
 * @brief   Heap memory aligned and placed in a specific memory section.
 *
 * @details Ensures that the heap is properly aligned based on the architecture for optimal memory access and performance.
 */
static uint8_t heap_memory[HEAP_SIZE] __attribute__((section(".heap"), aligned(ARCH_ALIGNMENT)));

/* =================================
 *   PRIVATE FUNCTION DECLARATION  *
 * ================================*/

/**
 * @fn      MEM_printd
 * @package MEM_alloc
 * 
 * @brief   Prints debug messages.
 *
 * @details Formats and prints debug messages to the standard output using variable arguments.
 *
 * @param   [in] format Format string.
 * @param   [in] ...    Variable arguments corresponding to the format string.
 */
void MEM_printd(const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/**
 * @fn      MEM_allocatorInit
 * @package MEM_alloc
 * 
 * @brief   Initializes the memory allocator.
 *
 * @details Sets up the memory allocator by zero-initializing the heap and creating 
 *          the initial free block that spans the entire heap.    
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure to be initialized.
 *
 * @return  0 on success, error code on failure.
 */
int MEM_allocatorInit(mem_allocator_t *allocator) 
{
    int ret                         = 0u;

    block_header_t *initial_block   = NULL;

    if (allocator == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    memset(heap_memory, 0, HEAP_SIZE);

    initial_block               = (block_header_t *)(heap_memory);

    initial_block->size         = HEAP_SIZE;
    initial_block->free         = 1u;
    initial_block->next         = NULL;
    initial_block->prev         = NULL;
    initial_block->file         = NULL;
    initial_block->line         = 0u;
    initial_block->var_name     = NULL;

    allocator->free_list        = initial_block;
    allocator->heap             = heap_memory;
    allocator->last_allocated   = initial_block;

end_of_function:
    return ret;
}

/**
 * @fn      MEM_findFirstFit
 * @package MEM_alloc
 * 
 * @brief   Finds the first free block that can accommodate the requested size.
 *
 * @details Implements the First-Fit allocation strategy by searching the free list and returning
 *          the first block that is large enough to satisfy the allocation request.
 *
 * @param   [in]      allocator Pointer to the memory allocator structure.
 * @param   [in]      size      Requested memory size.
 * @param   [out]     fit_block Output parameter to store the found free block.
 *
 * @return 0 on success, error code on failure.
 */
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

/**
 * @fn      MEM_findNextFit
 * @package MEM_alloc
 * 
 * @brief   Finds the next suitable free block starting from the last allocated block.
 *
 * @details Implements the Next-Fit allocation strategy by continuing the search from where the last
 *          allocation occurred, potentially improving allocation distribution and reducing fragmentation.
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure.
 * @param   [in]     size      Requested memory size.
 * @param   [out]    fit_block Output parameter to store the found free block.
 *
 * @return 0 on success, error code on failure.
 */
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

/**
 * @fn      MEM_findBestFit
 * @package MEM_alloc
 * 
 * @brief   Finds the best-fit free block for the requested size.
 *
 * @details Implements the Best-Fit allocation strategy by searching the free list and selecting
 *          the smallest block that is large enough to satisfy the allocation request, aiming to
 *          minimize fragmentation.
 *
 * @param   [in]      allocator Pointer to the memory allocator structure.
 * @param   [in]      size      Requested memory size.
 * @param   [out]     best_fit  Output parameter to store the best-fit free block.
 *
 * @return 0 on success, error code on failure.
 */
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

/**
 * @fn      MEM_splitBlock
 * @package MEM_alloc
 * 
 * @brief   Splits a free block into two if possible.
 *
 * @details Divides a larger free block into an allocated block and a smaller free b
 *          lock while maintaining alignment constraints. Updates the free list accordingly.
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure.
 * @param   [in/out] block     Pointer to the block to be split.
 * @param   [in]     size      Size of memory to allocate.
 *
 * @return  0 on success, error code on failure.
 */
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

/**
 * @fn      MEM_allocatorMalloc
 * @package MEM_alloc
 * 
 * @brief   Allocates memory using the custom allocator.
 *
 * @details Allocates a block of memory of the specified size from the allocator's heap. It aligns the size,
 *          selects the allocation strategy, finds the appropriate-fit block, splits it if necessary,
 *          and updates the block's metadata with the allocation source information.
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure.
 * @param   [in]     size      Size of memory to allocate.
 * @param   [in]     file      Name of the file requesting the allocation.
 * @param   [in]     line      Line number in the file requesting the allocation.
 * @param   [in]     var_name  Name of the variable being allocated.
 * @param   [in]     strategy  Allocation strategy to use (FIRST_FIT, NEXT_FIT, BEST_FIT).
 *
 * @return Pointer to the allocated memory on success, or NULL on failure.
 */
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

/**
 * @fn      MEM_validPointerCheck
 * @package MEM_alloc
 * 
 * @brief   Validates if a pointer is within the allocator's heap.
 *
 * @details Checks whether a given pointer falls within the allocator's heap range, is properly aligned,
 *          has a valid block header, and is marked as allocated.
 *
 * @param   [in]  allocator Pointer to the memory allocator structure.
 * @param   [in]  ptr       Pointer to validate.
 *
 * @return  0 on success, error code on failure.
 */
int MEM_validPointerCheck(mem_allocator_t *allocator, void *ptr) 
{
    int ret                 = 0u;

    block_header_t *block   = NULL;

    uintptr_t heap_start    = 0u;
    uintptr_t heap_end      = 0u;
    uintptr_t user_ptr      = 0u;

    heap_start  = (uintptr_t)(allocator->heap);
    heap_end    = (heap_start + HEAP_SIZE);
    user_ptr    = (uintptr_t)ptr;

    if (allocator == NULL) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    if (ptr == NULL) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    if (user_ptr < heap_start + sizeof(block_header_t) || user_ptr >= heap_end) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    if (((user_ptr - heap_start) % ALIGNMENT) != 0u) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    block = (block_header_t *)(user_ptr - sizeof(block_header_t));
    if ((uintptr_t)block < heap_start || (uintptr_t)block >= heap_end) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    if (block->free) 
    {
        errno = EINVAL;
        ret = EINVAL;
        goto end_of_function;
    }

    ret = 0u;

end_of_function:
    return ret;
}

/**
 * @fn      MEM_mergeBlocks
 * @package MEM_alloc
 * 
 * @brief   Merges adjacent free blocks to reduce fragmentation.
 *
 * @details Combines a free block with its neighboring free blocks (if any) to create a larger contiguous free block.
 *          Updates the free list accordingly.
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure.
 * @param   [in/out] block     Pointer to the block to be merged.
 *
 * @return  0 on success, error code on failure.
 */
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

/**
 * @fn      MEM_allocatorFree
 * @package MEM_alloc
 * 
 * @brief   Frees allocated memory using the custom allocator.
 *
 * @details Marks a previously allocated block as free, clears its metadata, and attempts to merge it with
 *          adjacent free blocks to minimize fragmentation.
 *
 * @param   [in/out] allocator Pointer to the memory allocator structure.
 * @param   [in]     ptr       Pointer to the memory to free.
 * @param   [in]     file      Name of the file requesting the free operation.
 * @param   [in]     line      Line number in the file requesting the free operation.
 * @param   [in]     var_name  Name of the variable being freed.
 *
 * @return 0 on success, error code on failure.
 */
int MEM_allocatorFree(mem_allocator_t *allocator, void *ptr, const char *file, int line, const char *var_name) 
{
    int ret                 = 0u;

    block_header_t *block   = NULL;

    if (allocator == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    ret = MEM_validPointerCheck(allocator, ptr);
    if (ret != 0u) 
    {
        fprintf(stderr, "MEM_allocatorFree: Invalid pointer %p for variable '%s' (in %s:%d)\n", ptr, var_name, file, line);
        goto end_of_function;
    }

    block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));

    if (block->free) 
    {
        fprintf(stderr, "MEM_allocatorFree: Double free detected for %p (variable '%s') (in %s:%d)\n", ptr, var_name, file, line);
        ret = EINVAL;
        goto end_of_function;
    }

    block->free     = 1u;
    block->file     = NULL;
    block->line     = 0u;
    block->var_name = NULL;

    MEM_printd("MEM_allocatorFree: Freed %zu bytes for variable '%s' from %p (in %s:%d)\n", 
               block->size - sizeof(block_header_t), 
               var_name ? var_name : "N/A", 
               ptr, file, line);

    ret = MEM_mergeBlocks(allocator, block);
    if (ret != 0u) 
    {
        fprintf(stderr, "MEM_allocatorFree: Failed to merge blocks (in %s:%d)\n", file, line);
        goto end_of_function;
    }

end_of_function:
    return ret;
}

/**
 * @fn      MEM_allocatorPrintAll
 * @package MEM_alloc
 * 
 * @brief   Prints the allocation table for debugging purposes.
 *
 * @details Iterates through the heap and prints the status of each memory block, including its address,
 *          size, allocation status, and the source file and line number if allocated.
 *
 * @param   [in] allocator Pointer to the memory allocator structure.
 *
 * @return  0 on success, error code on failure.
 */
int MEM_allocatorPrintAll(mem_allocator_t *allocator) 
{
    int ret = 0u;

    uint8_t *current = NULL;
    uint8_t *heap_end = NULL;

    block_header_t *block = NULL;

    if (allocator == NULL) 
    {
        ret = EINVAL;
        goto end_of_function;
    }

    printf("Allocation Table:\n");
    printf("Address\t\tSize\t\tFree\t\tFile:Line\n");

    current     = allocator->heap;
    heap_end    = allocator->heap + HEAP_SIZE;

    while (current < heap_end) 
    {
        block = (block_header_t *)current;

        printf("%p\t\t%zu\t\t%s\t\t%s:%d\n",
               (void *)(current + sizeof(block_header_t)),
               block->size - sizeof(block_header_t),
               block->free ? "Yes" : "No",
               block->free ? "N/A" : (block->file ? block->file : "Unknown"),
               block->free ? 0     : block->line);

        current += block->size;
    }

end_of_function:
    return ret;
}

/*** end of file ***/
