#include <fstream>
#include <random>
#include <iostream>
#include "rangeon.hpp"

int main() {
	std::random_device rd;
	RangeonGenerator rng(51, 51, 4, 150, 70, 102, rd());
	rng.Build();
	auto maze_layout = rng.Draw();
	
	std::cout << maze_layout << std::endl;
	std::ofstream file("maze.txt", std::ofstream::out);
	file << maze_layout << std::flush;
	//std::getchar();
	return 0;
}
