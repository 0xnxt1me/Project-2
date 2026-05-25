CC=gcc
CXX=g++
CFLAGS=-g -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -std=c11 -Wall -I.. -Ilibrary/include -fprofile-arcs -ftest-coverage 
CXXFLAGS=-Wall -I.. -Ilibrary/include -Llibrary/lib -fprofile-arcs -ftest-coverage -lgtest -lgtest_main -lpthread 

all: tests

tests: testuser testpass coverage_report

testuser: validation.o testuser.cpp
	$(CXX) validation.o testuser.cpp -o testuser $(CXXFLAGS)

testpass: validation.o testpass.cpp
	$(CXX) validation.o testpass.cpp -o testpass $(CXXFLAGS)

validation.o: ../validation.c
	$(CC) $(CFLAGS) -c ../validation.c -o validation.o

coverage_report:
	@rm -f *.gcda
	@./testuser --gtest_color=no || true
	@./testpass --gtest_color=no || true
	@gcov -b validation.c 

clean: 
	rm -f validation.o testpass testuser *.gcno *.gcda validation.c.gcov
