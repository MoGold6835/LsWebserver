CXX =g++
# CFLAGS = -std =c++14 -02 -Wall -g

TARGET = myserver

OBJS = log/*.cpp http/*.cpp buffer/*.cpp epoll/*.cpp thread/*.cpp timer/*.cpp webserver/*.cpp ./main.cpp
all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET) -pthread -lmysqlclient -fpermissive

clean:
	rm -rf ./$(OBJS) $(TARGET)