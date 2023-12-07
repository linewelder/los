TARGET_BIN := ../los.bin

BUILD_DIR := ../build/kernel
INCLUDE_DIR := ./include
SRC_DIR := .
DEP_DIR := $(BUILD_DIR)/.deps

AS := i686-elf-as

CXX := i686-elf-g++
CXXFLAGS := -fno-exceptions -fno-rtti -ffreestanding -nostdlib \
	        -O2 -mgeneral-regs-only \
			-Wall -Wextra -Werror \
			-I$(INCLUDE_DIR)
CXX_DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.cpp.Td

SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp' -or -name '*.s')
OBJS := $(SRCS:$(SRC_DIR)/%=$(BUILD_DIR)/%.o)

all: $(TARGET_BIN)

clean:
	-rm -r $(BUILD_DIR)
	-rm $(TARGET_BIN)

.PHONY: all clean

$(TARGET_BIN): linker.ld $(OBJS)
	$(CXX) $(CXXFLAGS) -T $< -o $@ $(OBJS) -lgcc

$(BUILD_DIR)/%.s.o: $(SRC_DIR)/%.s  | $(BUILD_DIR)
	$(AS) $< -o $@

%.o: %.cpp # Delete the default implicit rule.
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.cpp.d | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_DEP_FLAGS) $(CXXFLAGS) -c $< -o $@

# During compilation dependencies are written to a temporary file in order to
# avoid generating broken dependency files or incorrect timestamps.
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# Move them to a non-temporary file only after the compilation has
# succeeded.
	mv -f $(DEP_DIR)/$*.cpp.Td $(DEP_DIR)/$*.cpp.d
	touch $@

DEP_FILES := $(SRCS:$(SRC_DIR)/%=$(DEP_DIR)/%.d)
$(DEP_FILES):
	mkdir -p $(dir $@)

include $(wildcard $(DEP_FILES))

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
