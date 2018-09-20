HEADERS =-I./ -I/usr/local/include/
LIBS =-L./ -L/usr/local/lib

OBO:  pointindex.cpp linestringindex.cpp
	g++ $(HEADERS) $(LIBS) -std=c++11 -fopenmp -Wall -O3 -o Pointindex pointindex.cpp -lgdal
	g++ $(HEADERS) $(LIBS) -std=c++11 -fopenmp -Wall -O3 -o Linestringindex linestringindex.cpp -lgdal
clean:
	rm  Pointindex Linestringindex
	
