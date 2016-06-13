cc = g++
exe = server
obj = app.o server.o

$(exe) : $(obj)
	$(cc) -std=c++11 -o $(exe) $(obj) -lpthread

app.o : app.cc
	$(cc) -std=c++11 -c app.cc -lpthread

server.o : server.cc
	$(cc) -std=c++11 -c server.cc -lpthread


clean:
	rm -rf *.o server