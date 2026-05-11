#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

std::vector<std::string> splitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delimiter)) {
		result.push_back(item);
	}
	// 兼容末尾分隔符情况：例如 "a,b,"
	if (!str.empty() && str.back() == delimiter) {
		result.push_back("");
	}
	return result;
}

std::string toLower(const std::string& str) {
	std::string out = str;
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}

std::string trim(const std::string& str) {
	if (str.empty()) return "";

	size_t left = 0;
	while (left < str.size() && std::isspace(static_cast<unsigned char>(str[left]))) {
		++left;
	}

	if (left == str.size()) return "";

	size_t right = str.size() - 1;
	while (right > left && std::isspace(static_cast<unsigned char>(str[right]))) {
		--right;
	}

	return str.substr(left, right - left + 1);
}

std::string readFileToString(const std::string& filePath) {
	std::ifstream in(filePath, std::ios::binary);
	if (!in.is_open()) return "";

	std::ostringstream oss;
	oss << in.rdbuf();
	return oss.str();
}

bool writeStringToFile(const std::string& filePath, const std::string& content) {
	std::ofstream out(filePath, std::ios::binary);
	if (!out.is_open()) return false;

	out.write(content.data(), static_cast<std::streamsize>(content.size()));
	return out.good();
}

bool writeBytesToFile(const std::string& filePath, const char* data, size_t size) {
	if (data == nullptr && size > 0) return false;

	std::ofstream out(filePath, std::ios::binary);
	if (!out.is_open()) return false;

	if (size > 0) {
		out.write(data, static_cast<std::streamsize>(size));
	}
	return out.good();
}

size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData) {
	outData.clear();

	std::ifstream in(filePath, std::ios::binary);
	if (!in.is_open()) return 0;

	in.seekg(0, std::ios::end);
	std::streampos endPos = in.tellg();
	if (endPos < 0) return 0;

	const size_t size = static_cast<size_t>(endPos);
	in.seekg(0, std::ios::beg);

	outData.resize(size);
	if (size > 0) {
		in.read(outData.data(), static_cast<std::streamsize>(size));
		if (!in.good() && !in.eof()) {
			outData.clear();
			return 0;
		}
	}
	return outData.size();
}

std::string getCurrentTime() {
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	std::tm tmBuf{};
#ifdef _WIN32
	localtime_s(&tmBuf, &t);
#else
	localtime_r(&t, &tmBuf);
#endif

	std::ostringstream oss;
	oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

bool fileExists(const std::string& filePath) {
	std::error_code ec;
	return std::filesystem::exists(filePath, ec) && std::filesystem::is_regular_file(filePath, ec);
}

bool createDirectoryIfNotExists(const std::string& dirPath) {
	if (dirPath.empty()) return false;

	std::error_code ec;
	if (std::filesystem::exists(dirPath, ec)) {
		return std::filesystem::is_directory(dirPath, ec);
	}

	return std::filesystem::create_directories(dirPath, ec);
}