 /**
 *  @addtogroup MemoryManagement
 *  @{
 *  @addtogroup MemoryManagement MEM_alloc
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
 *  @see        - MEM_allocatorInit
 *              - MEM_allocatorMalloc 
 *              - MEM_allocatorFree
 **/

/* Header Include Protection */
#ifndef MEM_ALLOCATOR_H_
#define MEM_ALLOCATOR_H_

/* =================================
 *       PRIVATE INCLUDE FILES     *
 * ================================*/

/* dependencies: */
#include <stdint.h>
#include <stdlib.h>

/* =================================
 *          PUBLIC DEFINES         *
 * ================================*/

/**
 * @def ALIGNMENT
 * @package MEM_alloc
 * 
 * @brief Defines the default byte alignment for memory blocks.
 *
 * @details This constant sets the default alignment boundary (in bytes) for all memory allocations.
 *          Proper alignment ensures optimal performance and adherence to architecture-specific
 *          alignment requirements.
 */
#define ALIGNMENT (uint8_t)(8U)

/**
 * @def ARCH_ALIGNMENT
 * @package MEM_alloc
 * 
 * @brief Defines the architecture-specific alignment for memory blocks.
 *
 * @details This set of preprocessor directives detects the target architecture and sets the
 *          ARCH_ALIGNMENT macro accordingly. Proper alignment ensures optimal memory access
 *          performance and adherence to architecture-specific requirements.
 */
#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_ALIGNMENT (uint8_t)(16U)                   /**< x86_64 typically benefits from 16-byte alignment */
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ALIGNMENT (uint8_t)(16U)                   /**< ARM64 also benefits from 16-byte alignment */
#elif defined(__i386__) || defined(_M_IX86)
    #define ARCH_ALIGNMENT (uint8_t)(8U)                    /**< 32-bit x86 uses 8-byte alignment */
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ALIGNMENT (uint8_t)(8U)                    /**< 32-bit ARM uses 8-byte alignment */
#else
    #define ARCH_ALIGNMENT ALIGNMENT                        /**< Default alignment for other architectures */
#endif

/**
 * @def ALIGN
 * @package MEM_alloc
 * 
 * @brief Aligns a given size to the nearest multiple of ALIGNMENT.
 *
 * @details This macro takes a size and rounds it up to the nearest multiple of ALIGNMENT.
 *          It ensures that memory allocations are properly aligned, which is crucial for
 *          performance and preventing undefined behavior on some architectures.
 *
 * @param size [in]: The size to be aligned.
 *
 * @return The aligned size.
 */
#define ALIGN(size) (((size_t)(size) + ((size_t)ARCH_ALIGNMENT - 1)) & ~((size_t)ARCH_ALIGNMENT - 1))

/* =================================
 *      PUBLIC DATA STRUCTURES     *
 * ================================*/

/**
 * @enum    allocation_strategy
 * @package MEM_alloc
 * 
 * @typedef allocation_strategy_t
 * 
 * @brief   Defines the memory allocation strategies.
 *
 * @details Specifies the different algorithms that can be used for memory allocation.
 */
typedef enum 
{
    FIRST_FIT   = (uint8_t)(0u),                         /**< Allocates the first suitable free block */
    NEXT_FIT    = (uint8_t)(1u),                         /**< Allocates the next suitable free block after the last allocated block */
    BEST_FIT    = (uint8_t)(2u)                          /**< Allocates the smallest suitable free block */
} allocation_strategy_t;

/**
 * @struct  block_header
 * @package MEM_alloc
 * 
 * @typedef block_header_t
 * 
 * @brief   Represents the header of a memory block in the heap.
 *
 * @details This structure contains metadata about each memory block, including its size,
 *          allocation status, pointers to adjacent blocks, and debug information.
 */
typedef struct block_header 
{
    size_t size;                                        /**< Size of the block, including the header */
    int free;                                           /**< Allocation status: 1 if free, 0 if allocated */

    const char *file;                                   /**< Source file requesting the allocation */
    int line;                                           /**< Line number in the source file */

    const char *var_name;                               /**< Name of the variable associated with the allocation */

    struct block_header *next;                          /**< Pointer to the next block in the free list */
    struct block_header *prev;                          /**< Pointer to the previous block in the free list */
} block_header_t;

/**
 * @struct  mem_allocator
 * @package MEM_alloc
 * 
 * @typedef mem_allocator_t
 * 
 * @brief   Represents the memory allocator's state.
 *
 * @details This structure maintains the free list, the heap's starting address, and
 *          the last allocated block for Next-Fit allocation.
 */
typedef struct mem_allocator 
{
    block_header_t *free_list;                          /**< Pointer to the first free block in the heap */
    block_header_t *last_allocated;                     /**< Pointer to the last allocated block (used for Next-Fit) */

    uint8_t *heap;                                      /**< Pointer to the beginning of the heap memory */
} mem_allocator_t;

/* =================================
 * PUBLIC DATA FUNCTION PROTOTYPES *
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
void MEM_printd(const char *format, ...);

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
int MEM_allocatorInit(mem_allocator_t *allocator);

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
int MEM_findFirstFit(mem_allocator_t *allocator, size_t size, block_header_t **fit_block);

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
int MEM_findNextFit(mem_allocator_t *allocator, size_t size, block_header_t **fit_block);

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
int MEM_findBestFit(mem_allocator_t *allocator, size_t size, block_header_t **best_fit);

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
int MEM_splitBlock(mem_allocator_t *allocator, block_header_t *block, size_t size);

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
void *MEM_allocatorMalloc(mem_allocator_t *allocator, size_t size, const char *file, int line, const char *var_name, allocation_strategy_t strategy);

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
int MEM_validPointerCheck(mem_allocator_t *allocator, void *ptr);

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
int MEM_mergeBlocks(mem_allocator_t *allocator, block_header_t *block);

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
int MEM_allocatorFree(mem_allocator_t *allocator, void *ptr, const char *file, int line, const char *var_name);

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
int MEM_allocatorPrintAll(mem_allocator_t *allocator);

/**
 * @def MEM_ALLOCATOR
 * @package MEM_alloc
 * 
 * @brief Allocates memory using the custom allocator with file and line information.
 *
 * @details This macro simplifies memory allocation by automatically passing the current file name and line number
 *          to the allocator, aiding in debugging.
 *
 * @param allocator Pointer to the memory allocator structure.
 * @param size      The size of memory to allocate.
 * @param var_name  The name of the variable being allocated.
 *
 * @return Pointer to the allocated memory.
 */
#define MEM_ALLOCATOR(allocator, size, var_name) \
    MEM_allocatorMalloc(allocator, size, __FILE__, __LINE__, var_name, FIRST_FIT)

/**
 * @def MEM_ALLOC_FIRST_FIT
 * @package MEM_alloc
 * 
 * @brief Allocates memory using the First-Fit strategy.
 *
 * @param allocator Pointer to the memory allocator structure.
 * @param size      The size of memory to allocate.
 * @param var_name  The name of the variable being allocated.
 *
 * @return Pointer to the allocated memory.
 */
#define MEM_ALLOC_FIRST_FIT(allocator, size, var_name) \
    MEM_allocatorMalloc(allocator, size, __FILE__, __LINE__, var_name, FIRST_FIT)

/**
 * @def MEM_ALLOC_NEXT_FIT
 * @package MEM_alloc
 * 
 * @brief Allocates memory using the Next-Fit strategy.
 *
 * @param allocator Pointer to the memory allocator structure.
 * @param size      The size of memory to allocate.
 * @param var_name  The name of the variable being allocated.
 *
 * @return Pointer to the allocated memory.
 */
#define MEM_ALLOC_NEXT_FIT(allocator, size, var_name) \
    MEM_allocatorMalloc(allocator, size, __FILE__, __LINE__, var_name, NEXT_FIT)

/**
 * @def MEM_ALLOC_BEST_FIT
 * @package MEM_alloc
 * 
 * @brief Allocates memory using the Best-Fit strategy.
 *
 * @param allocator Pointer to the memory allocator structure.
 * @param size      The size of memory to allocate.
 * @param var_name  The name of the variable being allocated.
 *
 * @return Pointer to the allocated memory.
 */
#define MEM_ALLOC_BEST_FIT(allocator, size, var_name) \
    MEM_allocatorMalloc(allocator, size, __FILE__, __LINE__, var_name, BEST_FIT)

/**
 * @def MEM_FREE
 * @package MEM_alloc
 * 
 * @brief Frees allocated memory using the custom allocator with file and line information.
 *
 * @details This macro simplifies memory deallocation by automatically passing the current file name and line number
 *          to the allocator, aiding in debugging.
 *
 * @param allocator Pointer to the memory allocator structure.
 * @param ptr       Pointer to the memory to free.
 * @param var_name  The name of the variable being freed.
 *
 * @return 0 on success, error code on failure.
 */
#define MEM_FREE(allocator, ptr, var_name) \
    MEM_allocatorFree(allocator, ptr, __FILE__, __LINE__, var_name)

/* end of header*/
#endif /* MEM_ALLOCATOR_H_ */
/**@}*/
/**@}*/
