GPP=g++
GCC=gcc

OUTFILE="bin/mysql.so"

COMPILE_FLAGS=-c -m32 -O3 -fPIC -w -DLINUX -Wall -I src/SDK/amx/

all:
	$(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/*.cpp
	$(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o -L/usr/lib/mysql -lmysqlclient_r -lpthread
	rm -f *.o