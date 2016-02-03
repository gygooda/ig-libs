#ifndef BOSS_FILELISTER_H
#define BOSS_FILELISTER_H

#include <dirent.h>

#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;

class FileLister {
public:
	FileLister(const std::string& path = "");
	~FileLister();
	// 打开目录文件
	int open(const std::string& path);
    int open();
    void close();
	// 从头开始重新读取目录下所有文件
	void start();
	// 只返回普通文件（DT_REG），如果已经没有文件了，则返回空串
	bool next(string &filename);
	
private:
	std::string m_dirpath;
	DIR* m_dir;
};

#endif
