SOURCE  := $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TARGET  := myserver
CC      := g++
INCLUDE := -I./mysql/include
LIBS := $(SUBLIBS) -L./mysql/ -lmysqlclient -lpthread

CFLAGS  := -std=c++11 -g -Wall -O0 -w $(INCLUDE) -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

.PHONY : objs clean veryclean rebuild all
all : $(TARGET)
objs : $(OBJS)
rebuild: veryclean all
clean :
	rm -fr *.o
veryclean : clean
	rm -rf $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

