# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
CC=/usr/bin/gcc
CXX=/usr/bin/g++

TARGET_EXEC := vulkan_lvx

BUILD_DIR := ./build
SRC_DIRS := ./src

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# INC_DIRS +=	\
# /opt/ros/noetic/include	

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INC_FLAGS += 	\
-I/usr/include/pcl-1.13	\
-I/usr/include/eigen3	\
-I/usr/include/ni	\
-I/usr/include/openni2	\
-I/usr/include/hdf5/openmpi	\
-I/usr/lib/x86_64-linux-gnu/openmpi/include	\
-I/usr/lib/x86_64-linux-gnu/openmpi/include/openmpi	\
-I/usr/include/freetype2	\
-I/usr/include/libpng16	\
-I/usr/include/harfbuzz	\
-I/usr/include/glib-2.0	\
-I/usr/lib/glib-2.0/include	\
-I/usr/include/sysprof-6

CXXFLAGS := -Wall -O2 -std=c++17 -g

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP -fPIE

LDPATH:=	\
-L./	\

LDFLAGS :=	\
-lglfw	\
-lvulkan	\
-ldl	\
-lpthread	\
-lfreetype	\
-lspng
# -lX11	\
# -lXxf86vm	\
# -lXrandr	\
# -lXi

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDPATH)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)