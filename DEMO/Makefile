CC = g++
CFLAGS = -g
LDFLAGS = 
LIBS = .
SRC = src/rvm.cpp
OBJ = $(SRC:.cpp=.o)

OUT = bin/librvm.a

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

basic:
	$(CC) $(CFLAGS) src/basic.c $(OUT) -o bin/rvm

abort:
	$(CC) $(CFLAGS) src/abort.c $(OUT) -o bin/rvm
	
multi:
	$(CC) $(CFLAGS) src/multi.c $(OUT) -o bin/rvm
	
multiabort:
	$(CC) $(CFLAGS) src/multi-abort.c $(OUT) -o bin/rvm
	
truncate:
	$(CC) $(CFLAGS) src/truncate.c $(OUT) -o bin/rvm
	
music:
	$(CC) $(CFLAGS) src/music.c $(OUT) -o bin/rvm
	
uncommitted:
	$(CC) $(CFLAGS) src/uncommitted.c $(OUT) -o bin/rvm
	
tree:
	$(CC) $(CFLAGS) src/tree.c $(OUT) -o bin/rvm
	
clean :
	@rm src/*.o bin/*.a bin/rvm
	@echo Cleaned!
