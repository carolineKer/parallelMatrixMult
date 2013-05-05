CC:=mpicc
LD:=mpicc
#LDFLAGS:=-lm -lblas
LDFLAGS+=-L/pdc/vol/i-compilers/11.1/icc/mkl/lib/em64t -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -Wl,--rpath,/pdc/vol/i-compilers/11.1/icc/mkl/lib/em64t
CFLAGS:=-std=c99 -g
CFLAGS+=-I/pdc/vol/i-compilers/11.1/icc/mkl/include
SRC_DIR=src
TEST_DIR=test
TEST_RES=result
MATRIX_DIR=$(TEST_DIR)/matrix
MATRIX=$(MATRIX_DIR)/matrix
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst %.c,%.o, $(SRC))
EXE:=matrix_mult.exe
PROC_NB=4
#A will be a I by K matrix, B will be a K by J matrix
I=2
J=1007
K=2

all: $(EXE)

check: $(MATRIX)A $(MATRIX)B $(MATRIX)C $(TEST_DIR)/check_matrix.py
	$(TEST_DIR)/check_matrix.py  $(MATRIX)A $(MATRIX)B $(MATRIX)C 

run: $(MATRIX)C

$(MATRIX)A:
	mkdir -p $(MATRIX_DIR)
	$(TEST_DIR)/generate_matrix.py $(I) $(K) > $@

$(MATRIX)B:
	mkdir -p $(MATRIX_DIR)
	$(TEST_DIR)/generate_matrix.py $(K) $(J) > $@

$(MATRIX)C: $(EXE) $(MATRIX)A $(MATRIX)B
	mkdir -p $(MATRIX)C
	mpirun -np $(PROC_NB) $(EXE) $(MATRIX_DIR)

$(EXE):$(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS) 

clean:
	rm -f $(MATRIX)A $(MATRIX)B
	rm -rf $(MATRIX)C
	rm -f $(SRC_DIR)/*.o
	rm -f $(EXE)
