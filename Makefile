all:
	g++ -g -flto -O2 -std=c++11 PCY.cpp -o PCY

clean:
	rm PCY *.out qualified_*
