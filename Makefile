# 主目录 Makefile

.PHONY: all clean

# 定义 SDK 目录路径
SDK_DIR := ../../aX650_SDK_V1.45.0_P39_20240830020829_NO409

# 定义 example 子目录路径
EXAMPLE_DIR := example

# 默认目标
all: example-all

clean: example-clean

# 传递 SDK_DIR 变量到 example 子目录并执行 make all
example-all:
	@$(MAKE) -C $(EXAMPLE_DIR) SDK_DIR=$(SDK_DIR) all

# 传递 SDK_DIR 变量到 example 子目录并执行 make clean
example-clean:
	@$(MAKE) -C $(EXAMPLE_DIR) SDK_DIR=$(SDK_DIR) clean
