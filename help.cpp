#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <format>
#include <set>
#include <cstddef>
#include <cmath>
#define NOMINMAX
#include <Windows.h>

double Calculate(std::string_view input)
{
	const std::set<char> allowedCharacters{
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '.',
		'^', '*', '/', '+', '-',
		'(', ')'
	};

	const std::string operators{ "^*/+-" };

	std::size_t lParenCount = 0,
				rParenCount = 0;
	for (const char c : input)
	{
		if (c == '(')
			++lParenCount;
		else if (c == ')')
			++rParenCount;
		else if (!allowedCharacters.contains(c))
			throw std::invalid_argument{
			"Input contains one or more"
			" invalid characters"
			};
	}

	if (lParenCount != rParenCount)
		throw std::invalid_argument{
		"Input is improperly formatted"
		};

	std::size_t start,
				end,
				opIndex = std::string_view::npos;
	double expressionResult;

	if ((end = input.find(')'))
		!= std::string_view::npos)
	{
		start = input.rfind('(', end);

		const std::string_view contentInParentheses(
			input.data() + start + 1, input.data() + end
		);

		expressionResult = Calculate(contentInParentheses);

		if ((start == 0) && (end == input.size() - 1))
			return expressionResult;

		return Calculate(std::format("{}{}{}",
						input.substr(0, start),
						expressionResult,
						input.substr(end + 1)));
	}

	for (const char opChar : operators)
		if ((opIndex = input.find(opChar))
			!= std::string::npos)
			break;

	if ((opIndex == 0) || (opIndex == input.size() - 1)
		|| (opIndex == std::string::npos))
		throw std::invalid_argument{
			"Input is improperly formatted"
		};

	if (input[opIndex] == '*')
	{
		if (const std::size_t index = input.find('/');
			index < opIndex)
			opIndex = index;
	}
	else if (input[opIndex] == '+')
	{
		if (const std::size_t index = input.find('-');
			index < opIndex)
			opIndex = index;
	}

	start = opIndex - 1;
	while ((start > 0)
		&& !operators.contains(input[start - 1]))
		--start;

	end = opIndex + 1;
	while ((end < input.size())
		&& !operators.contains(input[end]))
		++end;

	const std::string
		lArgStr(input.data() + start,
				input.data() + opIndex),
		rArgStr(input.data() + opIndex + 1,
				input.data() + end);

	const double lArg = std::stod(lArgStr),
				 rArg = std::stod(rArgStr);

	switch (input[opIndex])
	{
	case '^':
		expressionResult = std::pow(lArg, rArg);
		break;
	case '*':
		expressionResult = lArg * rArg;
		break;
	case '/':
		expressionResult = lArg / rArg;
		break;
	case '+':
		expressionResult = lArg + rArg;
		break;
	case '-':
		expressionResult = lArg - rArg;
		break;
	}

	if ((start == 0) && (end == input.size()))
		return expressionResult;

	return Calculate(std::format("{}{}{}",
					input.substr(0, start),
					expressionResult,
					input.substr(end)));
}

void DisplayCharAt(std::size_t row,
					std::size_t column,
					char c)
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos{ .X = static_cast<SHORT>(column),
				.Y = static_cast<SHORT>(row) };
	SetConsoleCursorPosition(h, pos);
	std::cout << c;
}
