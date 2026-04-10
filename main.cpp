#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <utility>
#include <algorithm>
#include <thread>
#include <stop_token>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <cstddef>
#include <conio.h>

double Calculate(std::string_view);
void DisplayCharAt(std::size_t,
				std::size_t,
				char);

int main()
{
	constexpr std::size_t rows = 40,
		columns = 60;

	std::mutex mtx;
	std::condition_variable cv;
	double minX = 0,
		maxX = 10,
		minY = 0,
		maxY = 12.5;

	std::string input;
	std::cout << "> ";
	std::getline(std::cin, input);

	std::jthread displayThread{
	[&](std::stop_token st) {
		double prevMinX = minX,
			prevMinY = minY;
		bool firstDisplay = true;

		while (!st.stop_requested())
		{
			std::unique_lock lck{ mtx };
			cv.wait(lck, [&]()
				{return firstDisplay
				|| st.stop_requested()
				|| (prevMinX != minX)
				|| (prevMinY != minY); });

			if (st.stop_requested())
				return;

			const double
				xIncrement = (maxX - minX) / columns,
				yIncrement = (maxY - minY) / rows;

			std::vector<std::string> graph(rows,
						std::string(columns, ' '));

			for (std::size_t column = 0; column < columns;
				++column)
			{
				const double xValue
					= minX + xIncrement * column;

				std::string expression{ input };
				std::size_t index;
				while ((index = expression.find('x'))
					!= std::string::npos)
					expression.replace(index, 1,
						std::to_string(xValue));

				const double yValue = Calculate(expression);

				if ((yValue < minY) || (yValue > maxY))
					continue;

				const std::size_t offset
					= static_cast<std::size_t>(
						std::round((yValue - minY) / yIncrement))
						+ 1;

				const std::size_t row = (rows >= offset)
							? (rows - offset) : (rows - 1);

				graph[row][column] = 'X';
			}

			if ((0 > minY) && (0 < maxY))
			{
				const std::size_t offset
					= static_cast<std::size_t>(
						std::round((0 - minY) / yIncrement))
					+ 1;

				const std::size_t row = (rows >= offset)
					? (rows - offset) : (rows - 1);

				for (std::size_t column = 0;
					column < columns; ++column)
					graph[row][column] = '-';
			}

			if ((0 > minX) && (0 < maxX))
			{
				const std::size_t offset
					= static_cast<std::size_t>(
						std::round((0 - minX) / xIncrement))
					+ 1;

				const std::size_t column = (columns >= offset)
					? (columns - offset) : (columns - 1);

				for (std::size_t row = 0; row < rows;
					++row)
					graph[row][column] = '|';
			}

			firstDisplay = false;
			prevMinX = minX;
			prevMinY = minY;
			lck.unlock();

			for (std::size_t row = 0; row < rows; ++row)
				for (std::size_t column = 0;
					column < columns; ++column)
					DisplayCharAt(row, column,
									graph[row][column]);
		}
	} };

	char inputChar;
	bool takingInput = true;

	while (takingInput)
	{
		inputChar = _getch();

		switch (inputChar)
		{
		case 'w':
		{
			std::unique_lock lck{ mtx };
			maxY += 1.5;
			minY += 1.5;
			lck.unlock();
			cv.notify_one();
			break;
		}
		case 'a':
		{
			std::unique_lock lck{ mtx };
			minX = std::max(minX - 1.5, 0.0);
			if (minX != 0)
				maxX -= 1.5;
			lck.unlock();
			cv.notify_one();
			break;
		}
		case 's':
		{
			std::unique_lock lck{ mtx };
			maxY -= 1.5;
			minY -= 1.5;
			lck.unlock();
			cv.notify_one();
			break;
			break;
		}
		case 'd':
		{
			std::unique_lock lck{ mtx };
			maxX += 1.5;
			minX += 1.5;
			lck.unlock();
			cv.notify_one();
			break;
		}
		case '.':
		{
			takingInput = false;
			displayThread.request_stop();
			cv.notify_one();
			break;
		}
		}
	}
}
