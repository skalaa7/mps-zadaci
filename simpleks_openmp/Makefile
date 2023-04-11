all: sequential parallel

sequential: main.cpp
	g++ -fopenmp -Wall -O2 -o sequential main.cpp
parallel: projekat.cpp
	g++ -fopenmp -Wall -O2 -o parallel projekat.cpp
clean:
	rm sequential
	rm parallel
