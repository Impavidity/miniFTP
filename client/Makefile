cc = g++
exe = client
obj = app.o client.o

$(exe) : $(obj)
	$(cc) -std=c++11 -o $(exe) $(obj)

app.o : app.cc
	$(cc) -std=c++11 -c app.cc

client.o : client.cc
	$(cc) -std=c++11 -c client.cc

clean:
	rm -rf *.o main
