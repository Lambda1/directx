#include "Utils.hpp"

// �G���[����
void ErrorHandling(const std::string& error_str)
{
	std::cerr << error_str << std::endl;
	std::exit(EXIT_FAILURE);
}

// ���O�o��
void OutputLog(const std::wstring& log)
{
	static std::wofstream output("./output_log.txt", std::ios::out);
	
	// �����擾
	const time_t t = time(nullptr);
	tm locale_time = {};
	localtime_s(&locale_time, &t);

	// �����o��
	output
		<< (locale_time.tm_year + 1900) << "/"
		<< (locale_time.tm_mon + 1) << "/"
		<< (locale_time.tm_mday) << "-"
		<< (locale_time.tm_hour) << ":"
		<< (locale_time.tm_min) << ":"
		<< (locale_time.tm_sec)
		<< std::endl;
	// ���O�o��
	output << "\t" << log << std::endl;
}
