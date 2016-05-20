TARGET=../build/httpd

LN=gcc
CFLAGS=-Wall -Wstrict-prototypes -g

all: $(TARGET)

.PHONY: clean

../build/%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o ../build/$@

$(TARGET): ../build/http.o ../build/httpd.o ../build/utils.o ../build/forker.o
	$(LN) $(LDFLAGS) $^ -o $(TARGET)

clean:
	rm ../build/*
