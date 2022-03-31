#pragma once
#include <vector>
#include <random>
#include <functional>
#include <string>
#include <list>
#include <set>
#include <utility>

class RangeonGenerator {
public:
	RangeonGenerator(int tilemap_x, int tilemap_y, int additive_room_size, int retry_room_count, unsigned int percentage_maze_winding, unsigned int percentage_extra_holes, unsigned int seed);
	~RangeonGenerator();
	void Build();
	std::string Draw();
private:
	// needed definition
	enum class TileType {
		Wall = 0,
		Floor = 1,
		Opening = 2
	};

	enum class Direction {
		Up = 0,
		Down = 1,
		Left = 2,
		Right = 3,
		INVALID = 4
	};

	struct Tile {
		TileType type;
		int region;

		Tile() {
			type = TileType::Wall;
			region = -1;
		}
	};

	struct Rectangle {
		int top;
		int left;
		int width;
		int height;

		Rectangle(int l, int t, int w, int h) : left(l), top(t), width(w), height(h) {}

		bool Intersect(Rectangle &other) {
			return (left < other.left + other.width) &&
				(other.left < left + width) &&
				(top < other.top + other.height) &&
				(other.top < top + height);
		}

		bool Intersect(Rectangle &other, int addbound) {
			return (left < other.left + other.width + addbound) &&
				(other.left < left + width + addbound) &&
				(top < other.top + other.height + addbound) &&
				(other.top < top + height + addbound);
		}

		bool Intersect(const Rectangle &other) {
			return (left < other.left + other.width) &&
				(other.left < left + width) &&
				(top < other.top + other.height) &&
				(other.top < top + height);
		}

		bool Intersect(const Rectangle &other, int addbound) {
			return (left < other.left + other.width + addbound) &&
				(other.left < left + width + addbound) &&
				(top < other.top + other.height + addbound) &&
				(other.top < top + height + addbound);
		}
	};

	std::default_random_engine _rng;
	std::uniform_int_distribution<int> _roomd;
	
	std::vector<Rectangle> rooms;
	std::vector<std::vector<Tile>> map;
	
	unsigned int maze_winding;
	unsigned int extra_holes;
	int bf_room_retry;
  int map_bound_x;
	int map_bound_y;
	int curr_region;

	std::function<int()> Rngroomsize;

	void _buildRooms();
	void _buildMaze();
	void _buildDoors();

	// Accessory functions
	void inline _buildMaze(int x, int y, int region);
	void inline _paintRoom(Rectangle& room, int region);
	void inline _paintCell(int x, int y, int region, TileType type = TileType::Floor);
	bool _findDirection(std::list<Direction> &ls, Direction &dir);
	void inline _checkAndInsert(Tile &t, std::set<int> &covered);
	int _RandomFun(int i);
};
