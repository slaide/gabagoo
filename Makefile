CC=clang

CGLM_DIR=src/deps/cglm
CGLM_BUILD_DIR=$(CGLM_DIR)/build
CGLM_LIB=$(CGLM_BUILD_DIR)/libcglm.a
CGLM_INCLUDE=$(CGLM_DIR)/include

CFLAGS=-std=gnu23 -Wall -Werror -Wextra -Wpedantic -g -O1 -march=native
LFLAGS=
#-flto
INCLUDES=-Iinclude -I$(CGLM_INCLUDE)
OBJECTS=main.o egl.o gl.o window.o object.o
LIBS=-lxcb -lm -ldl

%.o: src/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $<

main: $(OBJECTS) $(CGLM_LIB)
	$(CC) -o main $(CFLAGS) $(LFLAGS) $(LIBS) $(CGLM_LIB) $(OBJECTS)

$(CGLM_LIB):
	mkdir -p $(CGLM_BUILD_DIR)
	cd $(CGLM_BUILD_DIR) && cmake .. -DCGLM_STATIC=ON -DCGLM_SHARED=OFF && make

.PHONY: run
run: main
	./main

.PHONY: clean
clean:
	rm -f main main.o
	rm -rf $(CGLM_BUILD_DIR)