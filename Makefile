LDFLAGS	 	=
COMMON_FLAGS 	= -Wall -O2
CFLAGS 		= $(COMMON_FLAGS) -std=c99 -Wno-unused-function
CXXFLAGS 	= $(COMMON_FLAGS) -std=c++20
CC	 	= cc
DEPS 	 	=
SOURCES  	= src/gen.c src/md4c.c src/md4c-html.c src/entity.c
DEFINES	 	=
OBJ	 	= $(addsuffix .o, $(basename $(notdir $(SOURCES))))

%.o: src/%.c $(DEPS)
	$(CC) -g -c -O2 $< $(CFLAGS) $(DEFINES) -o $@

g: $(OBJ)
	$(CC) -g -O2 $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) build
