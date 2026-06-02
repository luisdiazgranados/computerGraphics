CXX = clang++
CXXFLAGS = -std=c++11 -Wall -DNO_CG -DGL_SILENCE_DEPRECATION -O2
CXXFLAGS += $(shell fltk-config --cxxflags)
CXXFLAGS += $(shell pkg-config --cflags libtiff-4 2>/dev/null || echo -I/opt/homebrew/include)
LDFLAGS = $(shell fltk-config --ldflags --use-gl --use-images)
LDFLAGS += $(shell pkg-config --libs libtiff-4 2>/dev/null || echo -L/opt/homebrew/lib -ltiff)
LDFLAGS += -L/opt/homebrew/opt/jpeg-turbo/lib

TARGET = GraphicsPipeline

SRCS = V3.cpp M33.cpp ppc.cpp framebuffer.cpp TM.cpp scene.cpp \
       gui.cxx CGInterface.cpp cubemap.cpp font.cpp GLSLShader.cpp
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.cxx=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
