main : main.cpp
	g++ -o main main.cpp

.PHONY : clean
clean :
	-rm main
