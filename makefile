CC = g++

#  -g			: debugging
#  -Wall  		: compiler warnings
#  -std=c++2a 	: C++ 20
CFLAGS  = -g -Wall -std=c++2a
TARGET = simple_cross

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	rm -f $(ODIR)/*.o $(OUT)
