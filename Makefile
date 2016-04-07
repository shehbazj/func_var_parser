all:	func_grab dataStructureGenerator

func_grab: func_grab.cpp
	g++ -g -std=c++11 func_grab.cpp -o func_grab

dataStructureGenerator: dataStructureGenerator.cpp
	g++ -g -std=c++11 dataStructureGenerator.cpp -o dataStructureGenerator

clean:
	rm -rf func_grab dataStructureGenerator
