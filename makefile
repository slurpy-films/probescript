CC = g++
CFLAGS = -Iinclude -std=c++17
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:.cpp=.o)
TARGET = a

UNAME_S := $(shell uname -s)
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    LDFLAGS = -lws2_32
else ifeq ($(findstring CYGWIN,$(UNAME_S)),CYGWIN)
    LDFLAGS = -lws2_32
else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    LDFLAGS = -lws2_32
else
    LDFLAGS =
endif

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	rm -f $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean
	$(MAKE)

debug: CFLAGS += -g -O0
debug: LDFLAGS +=
debug: clean
	$(MAKE)
