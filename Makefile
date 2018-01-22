TARGET := ks
OBJS := ks.o

all: $(TARGET)

$(TARGET): $(OBJS)

clean:
	rm -rf $(OBJS)

clobber: clean
	rm -rf $(TARGET)
