CFLAGS = -std=c99 -Wall -Wextra -pedantic -g -I/usr/include/libxml2
LIBS = -lncurses -lxml2 -ltinfo
SRC = src/main.c src/ui.c src/vm_config.c src/xml_generator.c src/xml_parser.c src/editor.c src/command.c src/system_check.c
OBJ = $(SRC:.c=.o)
TARGET = eqemu

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
