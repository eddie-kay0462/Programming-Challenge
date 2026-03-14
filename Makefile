CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

transport: transport.cpp transport.h
	$(CXX) $(CXXFLAGS) -o transport transport.cpp

test: test_transport.cpp transport.h
	$(CXX) $(CXXFLAGS) -o test_transport test_transport.cpp
	./test_transport

clean:
	rm -f transport test_transport graph.dot

.PHONY: test clean
