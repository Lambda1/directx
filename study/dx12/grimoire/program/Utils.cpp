#include "Utils.hpp"

// エラー処理
void ErrorHandling(const std::string& error_str)
{
	std::cerr << error_str << std::endl;
	std::exit(EXIT_FAILURE);
}

// ログ出力
void OutputLog(const std::wstring& log)
{
	static std::wofstream output("./output_log.txt", std::ios::out);
	
	// 時刻取得
	const time_t t = time(nullptr);
	tm locale_time = {};
	localtime_s(&locale_time, &t);

	// 時刻出力
	output
		<< (locale_time.tm_year + 1900) << "/"
		<< (locale_time.tm_mon + 1) << "/"
		<< (locale_time.tm_mday) << "-"
		<< (locale_time.tm_hour) << ":"
		<< (locale_time.tm_min) << ":"
		<< (locale_time.tm_sec)
		<< std::endl;
	// ログ出力
	output << "\t" << log << std::endl;
}
