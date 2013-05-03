CC:=mpicc
LD:=mpicc
CFLAGS:=-std=c99 -g
SRC_DIR=src
TEST_DIR=test
TEST_RES=result
MATRIX=$(TEST_DIR)/matrix
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst %.c,%.o, $(SRC))
EXE:=matrix_mult.exe
PROC_NB=4
#A will be a I by K matrix, B will be a K by J matrix
I=2
J=2
K=2

all: $(EXE)

test: $(MATRIX)A $(MATRIX)B $(MATRIX)C
	$(TEST_DIR)/check_matrix.py $^

run: $(MATRIX)C

$(MATRIX)A:
	$(TEST_DIR)/generate_matrix.py $(I) $(K) > $@

$(MATRIX)B:
	$(TEST_DIR)/generate_matrix.py $(K) $(J) > $@

$(MATRIX)C: $(EXE) $(MATRIX)A $(MATRIX)B
	mpirun -np $(PROC_NB) $(EXE) $(MATRIX)A $(MATRIX)B $(MATRIX)C

$(EXE):$(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(MATRIX)*
	rm -f $(SRC_DIR)/*.o
	rm -f $(EXE)
