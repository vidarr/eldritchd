TARGET=../build/httpd

LN=gcc
CCFLAGS=-Wall -Wansi --strict-prototypes

all: $(TARGET)

.PHONY: clean

../build/%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o ../build/$@

$(TARGET): ../build/httpd.o ../build/utils.o ../build/forker.o
	$(LN) $(LDFLAGS) $^ -o $(TARGET)

clean:
	rm ../build/*
