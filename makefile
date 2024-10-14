# ==========================================================
# @file:			MAKEFILE
#
# @package: 		hyper-allocator
#
# @brief: 			Makefile responsible for compiling and 
#					managing the static and shared libraries 
#					of the hyper-allocator project.
#
# @version: 		v1.0.0
# @dependencies: 	gcc, ar, make
#
# @author: 			Rafael V. Volkmer
# @date: 			12/10/2023
# ==========================================================

# ------------------------------------------
# Silence All Notifications in Makefile
# ------------------------------------------
.SILENT:

# ==========================================
# ANSI Color and Style Definitions for Makefile
# ==========================================

# ------------------------------------------
# Text Colors (Foreground Colors)
# ------------------------------------------
BLACK           = \033[0;30m# Black
RED             = \033[0;31m# Red
GREEN           = \033[1;32m# Green (Bold)
YELLOW          = \033[1;33m# Yellow (Bold)
BLUE            = \033[1;34m# Blue (Bold)
PURPLE          = \033[1;35m# Purple (Bold)
CYAN            = \033[1;36m# Cyan (Bold)
WHITE           = \033[0;37m# White

# ------------------------------------------
# Light Text Colors (Bright Foreground Colors)
# ------------------------------------------
LIGHT_RED       = \033[0;91m# Light Red
LIGHT_GREEN     = \033[0;92m# Light Green
LIGHT_YELLOW    = \033[0;93m# Light Yellow
LIGHT_BLUE      = \033[0;94m# Light Blue
LIGHT_MAGENTA   = \033[0;95m# Light Magenta
LIGHT_CYAN      = \033[0;96m# Light Cyan
LIGHT_WHITE     = \033[0;97m# Light White

# ------------------------------------------
# Background Colors
# ------------------------------------------
BG_BLACK        = \033[40m# Black Background
BG_RED          = \033[41m# Red Background
BG_GREEN        = \033[42m# Green Background
BG_YELLOW       = \033[43m# Yellow Background
BG_BLUE         = \033[44m# Blue Background
BG_MAGENTA      = \033[45m# Magenta Background
BG_CYAN         = \033[46m# Cyan Background
BG_WHITE        = \033[47m# White Background

# ------------------------------------------
# Text Styles
# ------------------------------------------
BOLD            = \033[1m# Bold Text
DIM             = \033[2m# Dim Text
UNDERLINE       = \033[4m# Underlined Text
INVERT          = \033[7m# Inverted Colors
STRIKETHROUGH   = \033[9m# Strikethrough Text

# ------------------------------------------
# Reset Styles and Colors
# ------------------------------------------
RESET           = \033[0m# Reset All Styles and Colors
RESET_COLOR     = \033[39m# Reset Text Color
RESET_BG        = \033[49m# Reset Background Color

# ==========================================
# Unicode Table Border Characters Definitions
# ==========================================
# ==========================================
# Unicode Table Border Characters Definitions
# ==========================================

# ------------------------------------------
# Single Line Borders
# ------------------------------------------
SINGLE_TOP_LEFT    = ┌# Unicode character for the top-left corner of a single-line table
SINGLE_TOP_RIGHT   = ┐# Unicode character for the top-right corner of a single-line table
SINGLE_BOTTOM_LEFT = └# Unicode character for the bottom-left corner of a single-line table
SINGLE_BOTTOM_RIGHT= ┘# Unicode character for the bottom-right corner of a single-line table
SINGLE_HORIZONTAL  = ─# Unicode character for single horizontal lines
SINGLE_VERTICAL    = │# Unicode character for single vertical lines

# ------------------------------------------
# Double Line Borders
# ------------------------------------------
DOUBLE_TOP_LEFT    	= ╔# Unicode character for the top-left corner of a double-line table
DOUBLE_TOP_RIGHT   	= ╗# Unicode character for the top-right corner of a double-line table
DOUBLE_BOTTOM_LEFT 	= ╚# Unicode character for the bottom-left corner of a double-line table
DOUBLE_BOTTOM_RIGHT	= ╝# Unicode character for the bottom-right corner of a double-line table
DOUBLE_HORIZONTAL  	= ═# Unicode character for double horizontal lines
DOUBLE_VERTICAL    	= ║# Unicode character for double vertical lines

# ------------------------------------------
# Rounded Borders
# ------------------------------------------
ROUNDED_TOP_LEFT    = ╭# Unicode character for the top-left corner of a rounded table
ROUNDED_TOP_RIGHT   = ╮# Unicode character for the top-right corner of a rounded table
ROUNDED_BOTTOM_LEFT = ╰# Unicode character for the bottom-left corner of a rounded table
ROUNDED_BOTTOM_RIGHT= ╯# Unicode character for the bottom-right corner of a rounded table
ROUNDED_HORIZONTAL  = ─# Unicode character for horizontal lines in rounded tables
ROUNDED_VERTICAL    = │# Unicode character for vertical lines in rounded tables

# ------------------------------------------
# Heavy Borders
# ------------------------------------------
HEAVY_TOP_LEFT    = ╒# Unicode character for the top-left corner of a heavy table
HEAVY_TOP_RIGHT   = ╕# Unicode character for the top-right corner of a heavy table
HEAVY_BOTTOM_LEFT = ╘# Unicode character for the bottom-left corner of a heavy table
HEAVY_BOTTOM_RIGHT= ╛# Unicode character for the bottom-right corner of a heavy table
HEAVY_HORIZONTAL  = ═# Unicode character for heavy horizontal lines
HEAVY_VERTICAL    = ║# Unicode character for heavy vertical lines

# ------------------------------------------
# Mixed Borders
# ------------------------------------------
MIXED_TOP_LEFT    = ╓ # Unicode character for the top-left corner of a mixed border table
MIXED_TOP_RIGHT   = ╖ # Unicode character for the top-right corner of a mixed border table
MIXED_BOTTOM_LEFT = ╙ # Unicode character for the bottom-left corner of a mixed border table
MIXED_BOTTOM_RIGHT= ╜ # Unicode character for the bottom-right corner of a mixed border table
MIXED_HORIZONTAL  = ═ # Unicode character for mixed horizontal lines
MIXED_VERTICAL    = ║ # Unicode character for mixed vertical lines

# ------------------------------------------
# Dashed Borders
# ------------------------------------------
DASHED_TOP_LEFT    = ┌# Unicode character for the top-left corner of a dashed border table
DASHED_TOP_RIGHT   = ┐# Unicode character for the top-right corner of a dashed border table
DASHED_BOTTOM_LEFT = └# Unicode character for the bottom-left corner of a dashed border table
DASHED_BOTTOM_RIGHT= ┘# Unicode character for the bottom-right corner of a dashed border table
DASHED_HORIZONTAL  = ┄# Unicode character for dashed horizontal lines
DASHED_VERTICAL    = ┆# Unicode character for dashed vertical lines

# ------------------------------------------
# Curved Borders
# ------------------------------------------
CURVED_TOP_LEFT    = ╭# Unicode character for the top-left corner of a curved border table
CURVED_TOP_RIGHT   = ╮# Unicode character for the top-right corner of a curved border table
CURVED_BOTTOM_LEFT = ╰# Unicode character for the bottom-left corner of a curved border table
CURVED_BOTTOM_RIGHT= ╯# Unicode character for the bottom-right corner of a curved border table
CURVED_HORIZONTAL  = ─# Unicode character for curved horizontal lines
CURVED_VERTICAL    = │# Unicode character for curved vertical lines

# ==========================================
# Current Compiler
# ==========================================
CC = gcc

# ==========================================
# Directories Relative to ROOT_DIR
# ==========================================
ROOT_DIR        ?= $(abspath .)

INC_DIR         := $(ROOT_DIR)/inc
SRC_DIR         := $(ROOT_DIR)/src
TEST_DIR        := $(ROOT_DIR)/tests
SCRIPTS_DIR     := $(ROOT_DIR)/utils
BIN_DIR         := $(ROOT_DIR)/bin

# ------------------------------------------
# Create the bin directory if it doesn't exist
# ------------------------------------------
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ==========================================
# Includes and Paths
# ==========================================
INCLUDES_common := -I$(INC_DIR) -I$(ROOT_DIR)

# ==========================================
# Sources and Objects
# ==========================================
LIB_SRC  		= $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS 		= $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(LIB_SRC))

# ==========================================
# Names of Libraries and Executables
# ==========================================
LIB_STATIC 		= $(BIN_DIR)/libmemalloc.a
LIB_SHARED 		= $(BIN_DIR)/libmemalloc.so

# ==========================================
# Heap Size (default: 10 KB)
# ==========================================
HEAP_SIZE 		?= 10240

# ==========================================
# Common Compiling Flags
# ==========================================
CFLAGS_common 	:= 			\
    -Wall 					\
    -Werror 				\
    -Wextra 				\
    -Wpedantic 				\
    -Wshadow 				\
    -Wconversion 			\
    -Wstrict-prototypes 	\
    -Wformat=2 				\
    -Wunreachable-code 		\
    -Wunused-parameter 		\
    -fstrict-aliasing 		\
    -fwrapv 				\
    -fno-common 			\
    -Wno-unused-function

# ==========================================
# Release Compiling Flags
# ==========================================
CFLAGS_release := 			\
	-Ofast 					\
	-Os 					\
	-flto 					\
	-ffast-math 			\
	-fomit-frame-pointer 	\
	-funroll-loops 			\
	-fno-exceptions 		\
	-fno-unwind-tables 		\
	-fno-stack-protector 	\
	-fvisibility=hidden 	\
	-fdata-sections 		\
	-ffunction-sections

# ==========================================
# Release Linking Flags
# ==========================================
LDFLAGS_release := 			\
	-Wl,					\
	--gc-sections 			\
	-flto

# ==========================================
# Debug Compiling Flags
# ==========================================
CFLAGS_debug 	:= 			\
	-O0 					\
	-g 						\
	-D_DEBUG_=1

# ==========================================
# Debug Linking Flags
# ==========================================
LDFLAGS_debug 	:= 			\
	-Wl,					\
	--gc-sections 			\
	-flto

# ==========================================
# Position Independent Code Flag
# ==========================================
POSITION_INDEP := 			\
	-fPIC 

# ==========================================
# Phony Targets
# ==========================================
.PHONY: all clean test release debug build

# ==========================================
# Default Target
# ==========================================
all: release

# ==========================================
# Release Build Target
# ==========================================
release: CFLAGS = $(CFLAGS_common) $(CFLAGS_release)
release: LDFLAGS = $(LDFLAGS_release)
release: build

# ==========================================
# Debug Build Target
# ==========================================
debug: CFLAGS = $(CFLAGS_common) $(CFLAGS_debug)
debug: LDFLAGS = $(LDFLAGS_debug)
debug: build

# ==========================================
# Build Rules
# ==========================================
build: $(BIN_DIR) $(LIB_STATIC) $(LIB_SHARED)
	@echo " "
	@echo "$(GREEN)════════════════════════════════════════════════════════ ═══════ ════ ══$(RESET)"
	@echo "$(GREEN)Build process completed successfully!$(RESET)"
	@echo "$(GREEN)════════════════════════════════════════════════════════ ═══════ ════ ══$(RESET)"
	@echo " "

# ==========================================
# Compile Object Files with -fPIC for Shared Library
# ==========================================
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR) | $(BIN_DIR)
	@$(MAKE) print_object_file_conversion_table
	@echo "$(PURPLE)Source File from: $< $(RESET)"
	@echo "$(BLUE)Binary File to:   $@ $(RESET)"
	@echo " "
	@echo "$(YELLOW)Compiling object...$(RESET)"
	@echo " "
	@echo "$(CC) $(CFLAGS) $(INCLUDES_common) -DHEAP_SIZE=$(HEAP_SIZE) $(POSITION_INDEP) -c $< -o $@"
	$(CC) $(CFLAGS) $(INCLUDES_common) -DHEAP_SIZE=$(HEAP_SIZE) $(POSITION_INDEP) -c $< -o $@
	@echo " "
	@echo "$(GREEN)Compiled $< into: $(BIN_DIR)/$(@F)$(RESET)"

# ==========================================
# Create Static Library
# ==========================================
$(LIB_STATIC): $(LIB_OBJS) | $(BIN_DIR)
	@$(MAKE) print_static_library_table
	@echo "$(PURPLE)Object from:      $(LIB_OBJS)$(RESET)"
	@echo "$(BLUE)Static library to: $(LIB_STATIC)$(RESET)"
	@echo " "
	@echo "$(YELLOW)Creating static library...$(RESET)"
	@echo " "
	@echo "ar rcs $(LIB_STATIC) $(LIB_OBJS)"
	ar rcs $(LIB_STATIC) $(LIB_OBJS)
	@echo " "
	@echo "$(GREEN)Static library: $(LIB_STATIC) created successfully.$(RESET)"

# ==========================================
# Create Shared Library
# ==========================================
$(LIB_SHARED): $(LIB_OBJS) | $(BIN_DIR)
	@$(MAKE) print_shared_library_table
	@echo "$(PURPLE)Object from:      $(LIB_OBJS)$(RESET)"
	@echo "$(BLUE)Shared Library to: $(LIB_SHARED)$(RESET)"
	@echo " "
	@echo "$(YELLOW)Creating shared library...$(RESET)"
	@echo " "
	@echo "$(CC) -shared -o $(LIB_SHARED) $(LIB_OBJS)"
	$(CC) -shared -o $(LIB_SHARED) $(LIB_OBJS)
	@echo " "
	@echo "$(GREEN)Shared library: $(LIB_SHARED) created successfully.$(RESET)"

# ==========================================
# Clean Target
# ==========================================
clean:
	@$(MAKE) print_clean_table
	@echo "rm -rf $(BIN_DIR)"
	rm -rf $(BIN_DIR)
	@echo " "
	@echo "$(RED)═ ════ ═══════ ═════════════════════════════════════════════════════════$(RESET)"
	@echo "$(RED)Cleanup completed.$(RESET)"
	@echo "$(RED)═ ═══ ═══════ ══════════════════════════════════════════════════════════$(RESET)"
	@echo " "

# ==========================================
# Print Object File Conversion Table 
# ==========================================
print_object_file_conversion_table:
	@echo " "
	@echo "$(YELLOW)$(SINGLE_TOP_LEFT)─────────────────────────────────────────────────────────────────$(SINGLE_TOP_RIGHT)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) Building Code Binary (.o)                                       $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)─────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)  gcc <GccFlags> <LdFlags> -I <IncPath> *.c -o <ObjectPath/name> $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)─────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) Source File (.c)  ->  Object File (.o)                          $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) Compiles each .c file into a .o file for later linking.         $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_BOTTOM_LEFT)─────────────────────────────────────────────────────────────────$(SINGLE_BOTTOM_RIGHT)$(RESET)"
	@echo " "

# ==========================================
# Print Static Library Creation Table
# ==========================================
print_static_library_table:
	@echo " "
	@echo "$(YELLOW)$(SINGLE_TOP_LEFT)────────────────────────────────────────────────────────────────$(SINGLE_TOP_RIGHT)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) Building Static Library (.a)                                   $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)               ar rcs <LibName.a> <ObjectName.o>                $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) This command creates a static library named                    $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) libmemalloc.a from object files (.o). The rcs flags            $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) mean:                                                          $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) r: Replace or insert members.                                  $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) c: Create the archive if it doesn't exist.                     $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) s: Create an index for faster symbol lookup.                   $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_BOTTOM_LEFT)────────────────────────────────────────────────────────────────$(SINGLE_BOTTOM_RIGHT)$(RESET)"
	@echo " "

# ==========================================
# Print Shared Library Creation Table
# ==========================================
print_shared_library_table:
	@echo " "
	@echo "$(YELLOW)$(SINGLE_TOP_LEFT)───────────────────────────────────────────────────────────────$(SINGLE_TOP_RIGHT)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) Building Shared Library (.so)                                 $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)───────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)          gcc -shared -o <LibName.so> <ObjectName.o>           $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL)───────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) This command compiles the object files into a                 $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) shared library (.so) that can be dynamically linked           $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_VERTICAL) during runtime.                                               $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(YELLOW)$(SINGLE_BOTTOM_LEFT)───────────────────────────────────────────────────────────────$(SINGLE_BOTTOM_RIGHT)$(RESET)"
	@echo " "

# ==========================================
# Print Clean Up Table 	
# ==========================================
print_clean_table:
	@echo " "
	@echo "$(RED)$(SINGLE_TOP_LEFT)─────────────────────────────────────────────────────────────────$(SINGLE_TOP_RIGHT)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL) Clean Up                                                        $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL)─────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL)                         rm -rf bin                              $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL)─────────────────────────────────────────────────────────────────$(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL) Removes the entire bin directory along with                     $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_VERTICAL) object files and compiled libraries.                            $(SINGLE_VERTICAL)$(RESET)"
	@echo "$(RED)$(SINGLE_BOTTOM_LEFT)─────────────────────────────────────────────────────────────────$(SINGLE_BOTTOM_RIGHT)$(RESET)"
	@echo " "

### end of file ###
