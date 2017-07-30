all:
	g++-5 -std=c++11 -O3 -o main main.cc
	g++-5 -std=c++11 -O3 -o main_tc main.cc -ltcmalloc
	g++-5 -std=c++11 -O3 -o main_je main.cc -ljemalloc
clean:
	rm main main_tc main_je
