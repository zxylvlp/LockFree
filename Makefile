.PHONY: all install clean lint test release
all install clean lint test release:
	g++ main.cc -std=c++1z
simplespin:
	g++ TestSimpleSpinLock.cpp -std=c++1z -o4 -o simple
ticketspin:
	g++ TestTicketSpinLock.cpp -std=c++1z -o4
mcsspin:
	g++ TestMCSSpinLock.cpp -std=c++1z -o4
seqlock:
	g++ TestSequenceLock.cpp -std=c++1z -o4 -o seqlock
rwspin: RWSpinLock.h TestRWSpinLock.cpp
	g++ TestRWSpinLock.cpp -std=c++1z -o4 -o rwspin
cq: TestConcurrentQueue.cpp concurrent_queue.h
	g++ TestConcurrentQueue.cpp -std=c++1z -o4 -o cq
cqv: test_concurrent_queue_with_version.cpp concurrent_queue_with_version.h
	g++ test_concurrent_queue_with_version.cpp -std=c++1z -o1 -o cqv