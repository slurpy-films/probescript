CC = g++
CFLAGS = -Iinclude
SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = a.exe

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

reset: clean
	$(MAKE)
