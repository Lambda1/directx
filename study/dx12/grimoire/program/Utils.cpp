#include "Utils.hpp"

// �G���[����
void ErrorHandling(const std::string& error_str)
{
	std::cerr << error_str << std::endl;
	std::exit(EXIT_FAILURE);
}
