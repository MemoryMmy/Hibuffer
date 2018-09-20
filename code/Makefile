HEADERS =-I./ -I/usr/local/include/
LIBS =-L./ -L/usr/local/lib 

OBO:  buffer.cpp
	mpicxx $(HEADERS) $(LIBS) -fopenmp -Wall -O3 -o Buffer buffer.cpp -lgdal -lmpi -lhiredis 
clean:
	rm  Buffer
	
