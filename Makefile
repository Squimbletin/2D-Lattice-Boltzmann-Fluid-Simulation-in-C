CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDLIBS = -lm

SRC = src/grid.c src/lbm.c src/boundary.c src/visualize.c
MAIN = src/main.c

wind_tunnel: $(MAIN) $(SRC)
	$(CC) $(CFLAGS) $(MAIN) $(SRC) -o wind_tunnel $(LDLIBS)

test_grid: tests/test_grid.c src/grid.c
	$(CC) $(CFLAGS) tests/test_grid.c src/grid.c -o tests/test_grid $(LDLIBS)

test_lbm: tests/test_lbm.c src/grid.c src/lbm.c
	$(CC) $(CFLAGS) tests/test_lbm.c src/grid.c src/lbm.c -o tests/test_lbm $(LDLIBS)

clean:
	rm -f wind_tunnel tests/test_grid tests/test_lbm
	rm -rf frames

.PHONY: clean
