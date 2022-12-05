CC = gcc
CFLAGS = -fPIC -Wall

.PHONY: build
build: libscheduler.so

libscheduler.so: so_scheduler.o so_schedpreemt.o
	$(CC) $(LDFLAGS) -shared -o $@ $^

so_scheduler.o: so_scheduler.c util/so_scheduler.h util/utils.h
	$(CC) $(CFLAGS) -o $@ -c $<

so_schedpreemt.o: so_schedpreemt.c util/so_schedpreemt.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f so_scheduler.o so_schedpreemt.o libscheduler.so