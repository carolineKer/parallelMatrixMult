CC:=mpicc
LD:=mpicc
LDFLAGS=-lm -lblas
CFLAGS:=-std=c99 -g
SRC_DIR=src
TEST_DIR=test
TEST_RES=result
MATRIX=$(TEST_DIR)/matrix
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst %.c,%.o, $(SRC))
EXE:=matrix_mult.exe
PROC_NB=100
#A will be a I by K matrix, B will be a K by J matrix
I=140
J=100
K=145

all: $(EXE)

check: $(MATRIX)A $(MATRIX)B $(MATRIX)C $(TEST_DIR)/check_matrix.py
	$(TEST_DIR)/check_matrix.py  $(MATRIX)A $(MATRIX)B $(MATRIX)C 

run: $(MATRIX)C

$(MATRIX)A:
	$(TEST_DIR)/generate_matrix.py $(I) $(K) > $@

$(MATRIX)B:
	$(TEST_DIR)/generate_matrix.py $(K) $(J) > $@

$(MATRIX)C: $(EXE) $(MATRIX)A $(MATRIX)B
	mkdir -p $(MATRIX)C
	mpirun -np $(PROC_NB) $(EXE) $(MATRIX)A $(MATRIX)B $(MATRIX)C

$(EXE):$(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS) 

clean:
	rm -f $(MATRIX)A $(MATRIX)B
	rm -rf $(MATRIX)C
	rm -f $(SRC_DIR)/*.o
	rm -f $(EXE)
