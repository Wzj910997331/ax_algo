# Makefile
CXX = aarch64-linux-gnu-g++
CXXFLAGS = -I./ -I../include -I../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/msp/out/include/ 
CXXFLAGS += -I../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/third-party/opencv-4.5.5/include/opencv4/ -DOPENCV_STATIC

LDFLAGS = -L../lib -lax_algorithm
LDFLAGS += -L../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/msp/out/lib/ -lax_sys -lax_ivps -lax_engine -lax_interpreter

LDFLAGS += -L../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/third-party/opencv-4.5.5/lib/opencv4/3rdparty
LDFLAGS += -L../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/third-party/opencv-4.5.5/lib/ 

LDFLAGS += -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -ldl -lm -lpthread -ltegra_hal -littnotify  -llibjpeg-turbo -llibwebp -llibpng -llibtiff -llibopenjp2

LDFLAGS += -L../../aX650_SDK_V1.45.0_P39_20240830020829_NO409/build/out/AX650_slave/objs/rootfs/usr/lib/ -lz

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