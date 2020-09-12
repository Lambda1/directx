#include <iostream>
#include <vector>
#include <functional>

int main()
{
	std::vector<std::function<void(void)>> command_list;

	command_list.emplace_back([](){ std::cout << "GPU Set RTV-1" << std::endl; });
	std::cout << "GPU Set Op-2" << std::endl;
	
	command_list.emplace_back([](){ std::cout << "GPU Clear RTV-3" << std::endl; });
	std::cout << "GPU Clear Op-4" << std::endl;

	command_list.emplace_back([](){ std::cout << "GPU Close-5" << std::endl; });
	std::cout << "GPU Close Op-6" << std::endl;

	std::cout << std::endl;
	for(auto &cmd : command_list)
	{
		cmd();
	}

	return 0;
}
