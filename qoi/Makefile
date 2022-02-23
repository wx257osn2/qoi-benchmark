libqoi.a: qoi.o
	$(AR) rcs $@ $<

qoi.o: qoi.c qoi/qoi.h
	$(CC) -c -O3 -march=native -mtune=native -Wall -Wextra -o$@ $<

clean:
	$(RM) -f libqoi.a qoi.o

.PHONY: clean
