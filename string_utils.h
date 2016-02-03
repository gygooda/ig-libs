#ifndef BOSS_STRING_UTILS_HPP_
#define BOSS_STRING_UTILS_HPP_

#include <string.h>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <time.h>

using std::string;
using std::vector;
using std::istringstream;
using std::stringstream;

namespace BossCommon
{
    // 根据delim分解字符串
    void split(const std::string& s, char delim, std::vector<std::string>& elems);
    // 去除字符串s头尾的charlist字符
    void trim(std::string& s, const std::string& charlist);
    // 判断给定的字符串是否utf8编码
    bool is_utf8(const std::string& s);
    // 判断字符串是否全是数字
    bool is_all_digit(const std::string& s);
    // 字符串转换成数字类型，整型，浮点型等。
    template<typename T> void strtodigit(const string& str, T& out);

    // 只做简单转换，调用时要确保str的值。
    template<typename T> void strtodigit(const string& str, T& out)
    {
        stringstream ss;

        ss.clear();

        ss << str;
        ss >> out;
    }


	//只做int类型的转换
	int str2int(const char* p_str, uint32_t &out);

    //将字符串格式（2014-09-05 20：34：23）的时间转换为时间戳
    int parse_str_time(const char *str_time, time_t &time);

}

#endif
