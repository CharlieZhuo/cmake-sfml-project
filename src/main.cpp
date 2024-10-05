#include <SFML/Graphics.hpp>
#include <time.h>
#include <queue>
#include <vector>
using namespace sf;
using namespace std;

struct Point
{
	int x, y;
} a[4], b[4];

struct TetrominoType
{
	String name;
	Color color;
	std::vector<Point> pos; // 每个小方块在4*4的矩阵中的位置
};
TetrominoType AvailableTetrominoes[]{
	{"I", Color::Red, {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{3, 1}}},
	{"Z", Color::Green, {Point{0, 1}, Point{1, 1}, Point{1, 0}, Point{2, 0}}},
	{"S", Color::Blue, {Point{0, 0}, Point{1, 0}, Point{1, 1}, Point{2, 1}}},
	{"T", Color::Yellow, {Point{1, 0}, Point{0, 1}, Point{1, 1}, Point{2, 1}}},
	{"L", Color::Magenta, {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{2, 0}}},
	{"J", Color::Cyan, {Point{0, 0}, Point{0, 1}, Point{1, 1}, Point{2, 1}}},
	{"O", Color(100, 120, 140, 255), {Point{0, 0}, Point{1, 0}, Point{0, 1}, Point{1, 1}}}};

enum class MoveDirection
{
	Down,
	Left,
	Right,
	Rotate
};

class Tetromino
{
public:
	Tetromino()
	{
		int n = rand() % 7;
		tetrominoType = AvailableTetrominoes[n];
		currentPosition = tetrominoType.pos;
	};

	// 获取当前方块的移动后各个小方块的位置
	std::vector<Point> calculateMoveDestination(MoveDirection moveDirection)
	{
		std::vector<Point> moveDestination=currentPosition;
		switch (moveDirection)
		{
		case MoveDirection::Down:
			for (size_t i = 0; i < 4; i++)
			{
				moveDestination[i].y++;
			}
			break;
		case MoveDirection::Left:
			for (size_t i = 0; i < 4; i++)
			{
				moveDestination[i].x--;
			}
			break;
		case MoveDirection::Right:
			for (size_t i = 0; i < 4; i++)
			{
				moveDestination[i].x++;
			}
			break;
		case MoveDirection::Rotate: // 绕4*4矩阵的中心旋转90度
			Point center = currentPosition[1];
			for (size_t i = 0; i < 4; i++)
			{
				int x = currentPosition[i].y - center.y;
				int y = currentPosition[i].x - center.x;
				moveDestination[i].x = center.x - x;
				moveDestination[i].y = center.y + y;
			}
			break;
		};
		return moveDestination;
	};

	void setPosition(std::vector<Point> pos)
	{
		currentPosition = pos;
	};

	std::vector<Point> getCurrentPosition()
	{
		return currentPosition;
	};

	TetrominoType getType()
	{
		return tetrominoType;
	}

private:
	TetrominoType tetrominoType;
	std::vector<Point> currentPosition;
};

class Board
{
public:
	static const int BoardHeight = 20;
	static const int BoardWidth = 10;

	Board()
	{
		activeTetromino = Tetromino();
		futureTetrominoes.push(Tetromino());
		futureTetrominoes.push(Tetromino());
		futureTetrominoes.push(Tetromino());
	};

	// 检查是否与边界或其他方块碰撞
	bool willCollide(std::vector<Point> moveDestination)
	{
		for (size_t i = 0; i < moveDestination.size(); i++)
		{
			if (moveDestination.at(i).x < 0 || moveDestination.at(i).x >= BoardWidth || moveDestination.at(i).y >= BoardHeight)
				return true;
			else if (field[moveDestination.at(i).y][moveDestination.at(i).x] != Color::Black)
				return true;
		}
		return false;
	};

	// 检查指定行已经填满
	bool checkLine(int lineNum)
	{
		for (int i = 0; i < BoardWidth; i++)
		{
			if (field[lineNum][i] == Color::Black)
			{
				return false;
			}
		}
		return true;
	};

	// 消除指定行内容，并将上面的方块下移
	void clearLine(int lineNum)
	{
		for (int i = 0; i < BoardWidth; i++)
		{
			field[lineNum][i] = Color::Black;
		}
		for (int i = lineNum; i > 0; i--)
		{
			for (int j = 0; j < BoardWidth; j++)
			{
				field[i][j] = field[i - 1][j];
			}
		}
	};

	// 检查所有行是否有填满的，有则消除
	void clearLines()
	{
		for (int i = 0; i < BoardHeight; i++)
		{
			while (checkLine(i))
			{
				clearLine(i);
			}
		}
	};

	// 固定当前方块，将其加入到field中，并生成新的方块作为活动方块
	void fixTetromino()
	{
		std::vector<Point> currentPosition = activeTetromino.getCurrentPosition();
		for (size_t i = 0; i < currentPosition.size(); i++)
		{
			field[currentPosition[i].y][currentPosition[i].x] = activeTetromino.getType().color;
		}

		activeTetromino = futureTetrominoes.front();
		futureTetrominoes.pop();

		futureTetrominoes.push(Tetromino());
	};

	bool isGameOver()
	{ // 检查最上面一行是否有方块
		for (int i = 0; i < BoardWidth; i++)
		{
			if (field[0][i] != Color::Black)
			{
				return true;
			}
		}
		return false;
	};

	void draw(RenderWindow &window)
	{
		window.clear(Color::Black);

		// 画边界
		RectangleShape border(Vector2f(180, 360));
		border.setFillColor(Color::Black);
		border.setOutlineColor(Color::White);
		border.setOutlineThickness(2);
		border.setPosition(28, 31);
		window.draw(border);

		// 画field
		for (int i = 0; i < BoardHeight; i++)
		{
			for (int j = 0; j < BoardWidth; j++)
			{
				if (field[i][j] == Color::Black)
					continue;
				RectangleShape rect(Vector2f(18, 18));
				rect.setFillColor(field[i][j]);
				rect.setOutlineColor(Color::Black);
				rect.setOutlineThickness(1);
				rect.setPosition(j * 18, i * 18);
				rect.move(28, 31); // offset
				window.draw(rect);
			}
		}

		// 画当前方块
		for (int i = 0; i < 4; i++)
		{
			RectangleShape rect(Vector2f(18, 18));
			rect.setFillColor(activeTetromino.getType().color);
			rect.setOutlineColor(Color::Black);
			rect.setOutlineThickness(1);
			rect.setPosition(activeTetromino.getCurrentPosition()[i].x * 18, activeTetromino.getCurrentPosition()[i].y * 18);
			rect.move(28, 31); // offset
			window.draw(rect);
		}
		window.display();
	}

	// 定时运行
	void tick()
	{
		std::vector<Point> pts =
			activeTetromino.calculateMoveDestination(MoveDirection::Down);
		if (willCollide(pts))
		{
			fixTetromino();
			clearLines();
		}
		else
		{
			activeTetromino.setPosition(pts);
		}
	}

	void handleKeyEvent(Event e)
	{
		if (e.type == Event::KeyPressed)
		{
			std::vector<Point> pts;
			if (e.key.code == Keyboard::Up)
			{
				pts = activeTetromino.calculateMoveDestination(MoveDirection::Rotate);
			}
			else if (e.key.code == Keyboard::Left)
			{
				pts = activeTetromino.calculateMoveDestination(MoveDirection::Left);
			}
			else if (e.key.code == Keyboard::Right)
			{
				pts = activeTetromino.calculateMoveDestination(MoveDirection::Right);
			}
			else if (e.key.code == Keyboard::Down)
			{
				pts = activeTetromino.calculateMoveDestination(MoveDirection::Down);
			}
			if (!willCollide(pts))
			{
				activeTetromino.setPosition(pts);
			}
		}
	}

private:
	// 20*10的矩阵，每个元素表示一个方块的颜色
	// 最上方的行号为0，最左边的列号为0
	Color field[BoardHeight][BoardWidth] = {Color::Black}; // Black表示空，其他颜色表示有方块
	Tetromino activeTetromino;
	std::queue<Tetromino> futureTetrominoes;
};

int main()
{
	Time tickInterval = milliseconds(500);
	Clock clock;

	RenderWindow window(VideoMode(320, 480), "The Game!");
	Board board;

	while (window.isOpen())
	{

		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed)
				window.close();
			board.handleKeyEvent(e);
			board.draw(window);
		}
		Time elapsed = clock.getElapsedTime();
		if (elapsed > tickInterval)
		{
			board.tick();
			board.draw(window);
			clock.restart();
		}
	}

	return 0;
}