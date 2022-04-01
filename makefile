main: delete compile link


delete:
	del *.o

compile:
	g++ -c *.cpp

link:
	g++ -o test.exe *.o