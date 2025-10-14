MTL       = ./mtl
CHDRS     = $(wildcard *.h) $(wildcard $(MTL)/*.h)
EXEC      = rcpsp-gpr-sat
CFLAGS    = -I$(MTL) -Wno-deprecated -ffloat-store 
LFLAGS    = -lz

include ./mtl/template.mk
