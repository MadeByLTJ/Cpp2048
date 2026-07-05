// 2048 - Made by LTJ

#include <iostream>  // 包含相关库
#include <array>
#include <random>
#include <utility>
#include <algorithm>
#include <conio.h>
#include <iomanip>
#include <cmath>
#include <windows.h>
#include <fstream>
#include <string>

// 定义移动
enum class D { UP, DOWN, LEFT, RIGHT };

// 游戏类
class Game2048
{
private:
	std::array<std::array<int, 4>, 4> board{};
	int total_score = 0;
	int high_score = 0;
public:
	Game2048() {}
	~Game2048() {}

	// 光标位置
	void set_cursor(int x, int y) {
		std::cout << "\033[" << y << ";" << x << "H" << "\033[K";
	}

	// 光标可见
	void set_cursor_visible(bool v) {
		HANDLE h_con = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursor_info;
		GetConsoleCursorInfo(h_con, &cursor_info);
		cursor_info.bVisible = v;
		SetConsoleCursorInfo(h_con, &cursor_info);
	}

	// 旋转
	void rotate(D direction) {
		if (direction == D::RIGHT) {
			for (auto& row : board) std::reverse(row.begin(), row.end());
		}
		else if (direction == D::UP) {
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < i; ++j) {
					std::swap(board[i][j], board[j][i]);
				}
			}
		}
		else if (direction == D::DOWN) {
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4 - i; ++j) {
					std::swap(board[i][j], board[3 - j][3 - i]);
				}
			}
		}
	}

	// 向左压缩 (合并)
	int process_line_left(std::array<int, 4>& line) {
		int score = 0;
		std::array<int, 4> res{};
		int index = 0;

		for (int val : line) {
			if (val != 0) {
				res[index++] = val;
			}
		}

		// 真正的合并
		for (int i = 0; i < 3; ++i) {
			if (res[i] != 0 && res[i] == res[i + 1]) {
				res[i] *= 2;
				score += res[i];
				res[i + 1] = 0;
			}
		}

		index = 0;
		for (int val : res) {
			if (val != 0) {
				line[index++] = val;
			}
		}

		while (index < 4) line[index++] = 0;

		return score;
	}

	// 移动
	bool move(D direction) {
		int move_score = 0;
		std::array<std::array<int, 4>, 4> old = board;

		rotate(direction);
		for (int c = 0; c < 4; ++c) move_score += process_line_left(board[c]);
		rotate(direction);
		bool changed = (old != board);
		if (changed) {
			total_score += move_score;
		}
		else {
			board = old;
		}
		return changed;
	}

	// 生成随机数字
	bool generate() {
		std::vector<std::pair<int, int>> empty_cells;
		// 检查空位
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				if (board[i][j] == 0) {
					empty_cells.emplace_back(i, j);
				}
			}
		}
		// 没有则返回 false
		if (empty_cells.empty()) return false;

		// 初始化
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, static_cast<int>(empty_cells.size()) - 1);

		int index = dist(gen);
		auto [row, col] = empty_cells[index];

		// 生成 2 或 4 (概率: 9比1)
		std::uniform_int_distribution<> valDist(1, 10);
		board[row][col] = (valDist(gen) == 1) ? 4 : 2;
		return true;
	}

	// 显示 cell
	void show_cell(int val) {
		const char* fgcolor = "\033[37m";

		const char* bgcolors[] = {
			"\033[41m",
			"\033[42m",
			"\033[43m",
			"\033[44m",
			"\033[45m",
			"\033[46m"
		};

		const int color_count = 6;

		if (val == 0) {
			// empty: 4*"-"
			std::cout << "    ";
		}
		else {
			int color_index = static_cast<int>(log2(val));
			std::cout << fgcolor << bgcolors[color_index % color_count] << std::setw(4) << std::setfill(' ') << val << "\033[0m";
		}
	}

	// 显示 board
	void show_board() {
		for (int i = 0; i < 4; ++i) {
			std::cout << "+----+----+----+----+\n";
			std::cout << "|";
			for (int j = 0; j < 4; ++j) {
				show_cell(board[i][j]);
				std::cout << "|";
			}
			std::cout << std::endl;
		}
		std::cout << "+----+----+----+----+\n";
	}

	bool is_game_over() {
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				if (board[r][c] == 0) return false;
				if (c + 1 < 4 && board[r][c] == board[r][c + 1]) return false;
				if (r + 1 < 4 && board[r][c] == board[r + 1][c]) return false;
			}
		}
		return true;
	}

	void load_high_score() {
		std::ifstream infile("score.txt");
		int tmp = 0;
		if (infile.is_open()) {
			while (infile >> tmp) {
				if (tmp > high_score) {
					high_score = tmp;
				}
			}
			infile.close();
		}
	}

	void save_score(int val) {
		std::ofstream outfile("score.txt", std::ios::app);
		if (outfile.is_open()) {
			outfile << val << "\n";
			outfile.close();
		}
	}

	// 开始游戏
	void start() {
		D dir{};
		bool moved = false;
		bool validkey = true;
		set_cursor_visible(false);
		load_high_score();
		generate();
		while (1) {
			set_cursor(1, 1);
			show_board();
			std::cout << "分数: " << total_score << std::endl;
			int key = _getch();
			if (key == 0 || key == 224) {
				key = _getch();
				switch (key) {
				case 72: dir = D::UP; break;
				case 75: dir = D::LEFT; break;
				case 80: dir = D::DOWN; break;
				case 77: dir = D::RIGHT; break;
				default: validkey = false; break;
				}
			}
			else {
				switch (key) {
				case 87: case 119: dir = D::UP; break;
				case 65: case 97: dir = D::LEFT; break;
				case 83: case 115: dir = D::DOWN; break;
				case 68: case 100: dir = D::RIGHT; break;
				default: validkey = false; break;
				}
			}
			moved = move(dir);
			if (moved) {
				generate();
			}
			if (is_game_over()) {
				break;
			}
		}
		save_score(total_score);
		std::cout << "\n================\n";
		if (total_score > high_score) std::cout << "新纪录\n分数: " << total_score;
		else if (high_score > total_score) std::cout << "分数: " << total_score << "\n最高分数: " << high_score;
		std::cout << "\n================\n";
	}
};

int main()
{
	// 创建游戏实例
	Game2048 g;
	// 开始
	g.start();
	std::cout << "\n按回车退出... ";
	std::cin.get();
	return 0;
}
