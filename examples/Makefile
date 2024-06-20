.POSIX:

CFLAGS = -g -O0 -I..

.PHONY: all
all: jpp app_menu

json.o: json.c

app_menu: app_menu.o json.o
app_menu.o: app_menu.c

jpp: jpp.o json.o
jpp.o: jpp.c

.PHONY: clean
clean:
	rm -f *.o jpp app_menu
