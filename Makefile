main : main.cpp
	g++ -O3 -o main main.cpp

.PHONY : clean
clean :
	-rm main
