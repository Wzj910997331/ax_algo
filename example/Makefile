# Makefile
.PHONY: all clean

CXX = aarch64-linux-gnu-g++

# BSP_MSP_DIR 依赖于 SDK_DIR
BSP_MSP_DIR = $(SDK_DIR)/msp/out/

CXXFLAGS = -I./ -I../include -I$(BSP_MSP_DIR)include/
CXXFLAGS += -I$(SDK_DIR)/third-party/opencv-4.5.5/include/opencv4/ -DOPENCV_STATIC

LDFLAGS = -L../lib -lax_algorithm
LDFLAGS += -L$(BSP_MSP_DIR)lib/ -lax_sys -lax_ivps -lax_engine -lax_interpreter

LDFLAGS += -L$(SDK_DIR)/third-party/opencv-4.5.5/lib/opencv4/3rdparty
LDFLAGS += -L$(SDK_DIR)/third-party/opencv-4.5.5/lib/ 

LDFLAGS += -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -ldl -lm -lpthread -ltegra_hal -littnotify  -llibjpeg-turbo -llibwebp -llibpng -llibtiff -llibopenjp2

LDFLAGS += -L$(SDK_DIR)/build/out/AX650_slave/objs/rootfs/usr/lib/ -lz

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = main_executable

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
