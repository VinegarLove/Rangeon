#include "rangeon.hpp"
#include <sstream>
#include <array>
#include <algorithm>
#include <iostream>

RangeonGenerator::RangeonGenerator(int tilemap_x, int tilemap_y,
																	 int max_room_size, int retry_room_count,
																	 unsigned int percentage_maze_winding,
																	 unsigned int percentage_extra_holes,
																	 unsigned int seed)
																	 
																	 : bf_room_retry(retry_room_count), 
																	 _rng(seed), 
																	 _roomd(2, max_room_size),
																	 map_bound_x(tilemap_x), map_bound_y(tilemap_y), curr_region(0),
																	 map(tilemap_x, std::vector<Tile>(tilemap_y)), 
																	 maze_winding(percentage_maze_winding)
{
	Rngroomsize = std::bind(_roomd, _rng);
	
}

RangeonGenerator::~RangeonGenerator() {
}

void RangeonGenerator::Build() {
	_buildRooms();
	_buildMaze();
	_buildDoors();
}

std::string RangeonGenerator::Draw() {
	char symb[] = "#_O";
	std::stringstream ss;
	for (int i = 0; i < map_bound_x; i++) {
		for (int j = 0; j < map_bound_y; j++) {
			ss << symb[int(map[i][j].type)];
			//ss << map[i][j].region;
		}
		ss << std::endl;
	}
	return ss.str();
}

void RangeonGenerator::_buildRooms() {
	int try_counter = bf_room_retry;
	int count = 0;
	while (--try_counter >= 0) {
		// room always with a center, odd sized
		int sx = Rngroomsize() * 2 + 1;
		int sy = Rngroomsize() * 2 + 1;

		// attempt a valid location (clever without using if, found also on web)
		int x = (_rng() % (int((map_bound_x - sx) / 2))) * 2 + 1;
		int y = (_rng() % (int((map_bound_y - sy) / 2))) * 2 + 1;

		Rectangle room(x, y, sx, sy);
		bool intersect = false;
		for (std::vector<Rectangle>::const_iterator i = rooms.begin(), end = rooms.end(); i != end; ++i) {
			if (room.Intersect((*i), 1)) {
				intersect = true;
				break;
			}
		}
		
		if (!intersect) {
			rooms.push_back(room);
			count++;
		}
	}
	
	// paint room on the tilemap, assigning a different region for each room
	curr_region = 0;
	for (int i = 0; i < count; i++) {
		_paintRoom(rooms[i], curr_region);
		curr_region++;
	}
}

void inline RangeonGenerator::_paintRoom(Rectangle& room, int region) {
	int right = room.left + room.width;
	int bottom = room.top + room.height;
	for (int i = room.left; i < right; i++) {
		for (int j = room.top; j < bottom; j++) {
			_paintCell(i, j, region, TileType::Floor);
		}
	}
}

void inline RangeonGenerator::_paintCell(int x, int y, int region, TileType type) {
	map[x][y].region = region;
	map[x][y].type = type;
}

void RangeonGenerator::_buildMaze() {
	// Carve mazes in wall where possible
	for (int y = 1; y < map_bound_y; y += 2) {
		for (int x = 1; x < map_bound_x; x += 2) {
			if (map[x][y].type != TileType::Wall) continue;
			_buildMaze(x, y, curr_region);
			curr_region++;
		}
	}
}

// construct the list of cells to start the maze, and carve the first
void inline RangeonGenerator::_buildMaze(int x, int y, int region) {
	// init array with direction pair
	std::array<std::array<int, 2>, 4> Dir{ { { { 0, -1 } }, { { 0, 1 } }, { { -1, 0 } }, { { 1, 0 } } } };
	
	// init the list of cells with only the first member and carve it
	std::list<std::array<int, 2>> avaiable_cells;
	avaiable_cells.push_back(std::array < int, 2 > {{x, y}});
	_paintCell(x, y, region, TileType::Floor);

	// we use this to store the avaiable direction found
	std::list<Direction> avaiable_directions;

	// last direction followed by the maze
	Direction last_dir = Direction::INVALID;

	// continue growing the maze until the avaiable cells are null, snakelike.
	// if possible, we want to follow the starting direction.
	while (!avaiable_cells.empty()) {
		auto cell = avaiable_cells.back();
		avaiable_directions.clear();
		bool found = false;

		// left check
		if (cell[0] - 3 >= 0 && map[cell[0] - 2][cell[1]].type == TileType::Wall) {
			avaiable_directions.push_back(Direction::Left);
			found = true;
		}
		// right check
		if (cell[0] + 3 < map_bound_x && map[cell[0] + 2][cell[1]].type == TileType::Wall) {
			avaiable_directions.push_back(Direction::Right);
			found = true;
		}
		// up check
		if (cell[1] - 3 >= 0 && map[cell[0]][cell[1] - 2].type == TileType::Wall) {
			avaiable_directions.push_back(Direction::Up);
			found = true;
		}
		// down check
		if (cell[1] + 3 < map_bound_y && map[cell[0]][cell[1] + 2].type == TileType::Wall) {
			avaiable_directions.push_back(Direction::Down);
			found = true;
		}

		if (found) {
			// Chosing direction and carving 2 cells in that direction
			Direction chosen_direction;
			if (last_dir != Direction::INVALID && _findDirection(avaiable_directions, last_dir) && (_rng() % 100) > maze_winding) {
				chosen_direction = last_dir;
			} else {
				int rnd = _rng() % avaiable_directions.size();
				std::list<Direction>::iterator it = avaiable_directions.begin();
				std::advance(it, rnd);
				chosen_direction = (*it);
				last_dir = chosen_direction;
			}

			int d = static_cast<int>(chosen_direction);

			// first cell painting
			_paintCell(cell[0] + Dir[d][0], cell[1] + Dir[d][1], region, TileType::Floor);

			// second cell painting
			_paintCell(cell[0] + (Dir[d][0] * 2), cell[1] + (Dir[d][1] * 2), region, TileType::Floor);

			// add cell to unfinished ones
			avaiable_cells.push_back(std::array < int, 2 > { { (cell[0] + (Dir[d][0] * 2)), (cell[1] + (Dir[d][1] * 2)) } });
		} else {
			avaiable_cells.pop_back();
			last_dir = Direction::INVALID;
		}
	}
}

bool RangeonGenerator::_findDirection(std::list<Direction> &ls, Direction &dir) {
	for (auto i = ls.begin(); i != ls.end(); i++) {
		if ((*i) == dir) return true;
	}
	return false;
}

void RangeonGenerator::_buildDoors() {
	// index of door position pointing to the region that would open
	std::vector < std::pair < std::array<int, 2>, std::set<int> >> index_door_region;

	// index of opened region
	std::vector<bool> open_region;
	open_region.resize(curr_region);

	// - if it's a wall, check if it's a valid door and sign which region would join
	int bx = map_bound_x - 1;
	int by = map_bound_y - 1;
	for (int x = 1; x < bx; x++) {
		for (int y = 1; y < by; y++) {
			// skip non wall
			if (map[x][y].type != TileType::Wall) continue;
			
			std::set<int> temp_region{};
			_checkAndInsert(map[x - 1][y], temp_region);		// l
			_checkAndInsert(map[x + 1][y], temp_region);		// r
			_checkAndInsert(map[x][y - 1], temp_region);		// u
			_checkAndInsert(map[x][y + 1], temp_region);		// d

			// at least 2 exit must be found
			if (temp_region.size() < 2) continue;

			index_door_region.push_back(std::make_pair(std::array < int, 2 > {{x, y}}, temp_region));
		}
	}
	
	// deallocate useless memory, slower but limits memory usage, good for low end devices
	index_door_region.shrink_to_fit();

	// random shuffle the door to check
	std::random_shuffle(index_door_region.begin(), index_door_region.end(), std::bind(&RangeonGenerator::_RandomFun, this, std::placeholders::_1));
	
	// carve needed door
	for (auto i = index_door_region.begin(); i != index_door_region.end(); i++) {
		// we don't want door / holes next to each other
		if (map[(*i).first[0] - 1][(*i).first[1]].type == TileType::Opening ||
				map[(*i).first[0] + 1][(*i).first[1]].type == TileType::Opening ||
				map[(*i).first[0]][(*i).first[1] - 1].type == TileType::Opening ||
				map[(*i).first[0]][(*i).first[1] + 1].type == TileType::Opening) continue;

		int valid_region = 0;
		for (auto r_set = (*i).second.begin(); r_set != (*i).second.end(); r_set++) {
			if (!open_region[(*r_set)]) valid_region++;
		}

		// not a valid door, and not saved by test for extra holes
		if (valid_region < 1 && (_rng() % 100 >= extra_holes)) continue;

		// dig door
		_paintCell((*i).first[0], (*i).first[1], -1, TileType::Opening);
		for (auto r_set = (*i).second.begin(); r_set != (*i).second.end(); r_set++) {
			open_region[(*r_set)] = true;
		}
	}
}

void inline RangeonGenerator::_checkAndInsert(Tile &t, std::set<int> &covered) {
	if (t.type != TileType::Wall) {
		covered.insert(t.region);
	}
}

int RangeonGenerator::_RandomFun(int i) {
	return _rng() % i;
}
