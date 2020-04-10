 Objects = main.o  dataload.o allPairs.o sizeAware.o filter.o sizeawareasfilter.o scanCount.o divideSkip.o
setSimilarity : $(Objects) 

	g++  -o3 -o filterAllPairs -lpthread $(Objects) 

Main.o : main.cpp dataload.h allPairs.h sizeAware.h filter.h sizewareasfilter.h scanCount.h divideSkip.h

	g++  -c main.cpp

allPairs.o : allPairs.cpp dataload.h allPairs.h 

	g++  -c allPairs.cpp


sizeAware.o : sizeAware.cpp dataload.h sizeAware.h 

	g++  -c sizeAware.cpp

filter.o : filter.cpp dataload.h filter.h allPairs.h

	g++   -c filter.cpp

sizeawareasfilter.o : filter.cpp dataload.h filter.h allPairs.h sizewareasfilter.h

	g++   -c sizeawareasfilter.cpp

scanCount.o : scanCount.cpp dataload.h scanCount.h 

	g++   -c scanCount.cpp

divideSkip.o : divideSkip.cpp dataload.h divideSkip.h
	g++   -c divideSkip.cpp

clean:

	rm filterAllPairs $(Objects)