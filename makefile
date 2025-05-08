CC = g++
CFLAGS = -Iinclude
SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = a.exe

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

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

reset: clean
	$(MAKE)
