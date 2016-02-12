coro: coro.cpp
	g++ -O3 -o coro coro.cpp -std=c++11 -lboost_coroutine -lboost_system -lrt
hello: hello.cpp
	g++ -o hello hello.cpp -lboost_locale
clean:
	rm hello coro
