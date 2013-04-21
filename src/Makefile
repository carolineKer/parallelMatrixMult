CC:=mpicc
LD:=mpicc
CFLAGS:=-std=c99 -g
SRC:=$(wildcard *.c)
OBJ:=$(patsubst %.c,%.o, $(SRC))
EXE:=matrix_mult.exe

all: $(EXE)

$(EXE):$(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f $(EXE)
