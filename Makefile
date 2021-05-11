main : main.cpp
	g++ -O2 -o main main.cpp

.PHONY : clean
clean :
	-rm main
