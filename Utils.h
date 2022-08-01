#pragma once
#include "ggl.h"
#ifdef _MSC_FULL_VER
#include <windows.h>
#pragma comment( lib,"winmm.lib" )
#elif __APPLE__
#include <sys/timeb.h>
#endif // _MSC_FULL_VER

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

inline unsigned char* LoadFileContent(const char* path, int& fileSize) {
	unsigned char* fileContent = nullptr;
	fileSize = 0;
	FILE* pFile;
	errno_t err = fopen_s(&pFile, path, "rb");
	if (err != 0) {
		perror("[ERROR] ");
		return nullptr;
	}
	if (err == 0) {
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		if (nLen > 0) {
			rewind(pFile);
			fileContent = new unsigned char[nLen + 1];
			fread_s(fileContent, nLen, nLen / (sizeof(unsigned char)), sizeof(unsigned char), pFile);
			fileContent[nLen] = '\0';
			fileSize = nLen;
		}
		fclose(pFile);
	}
	return fileContent;
}
inline float GetFrameTime() {
	static unsigned long long lastTime = 0, timeCurrent = 0;
#ifdef __APPLE__
	timeval current;
	gettimeofday(&current, nullptr);
	timeCurrent = current.tv_sec * 1000 + current.tv_usec / 1000;
#elif _MSC_FULL_VER
	timeCurrent = (double)clock() / (double)CLOCKS_PER_SEC;
#endif
	unsigned long frameTime = lastTime == 0 ? 0 : timeCurrent - lastTime;
	lastTime = timeCurrent;
	return float(frameTime) / 1000.0f;
}
inline unsigned char* DecodeBMP(unsigned char* bmpfile, int& width, int& height) {
	if (memcmp(bmpfile, "BM", 2) == 0) {
		int pixel_bgr_data_offset = *((int*)(bmpfile + 10));
		width = *((int*)(bmpfile + 18));
		height = *((int*)(bmpfile + 22));
		unsigned char* bgr_data = bmpfile + pixel_bgr_data_offset;
		for (int i = 0; i < width * height; i++) {
			int current_pixel_index = i * 3;
			unsigned char b = bgr_data[current_pixel_index + 0];
			unsigned char r = bgr_data[current_pixel_index + 2];
			bgr_data[current_pixel_index + 0] = r;
			bgr_data[current_pixel_index + 2] = b;
		}
		return bgr_data;
	}
	return nullptr;
}
inline unsigned char* DecodeIMG(unsigned char* imgfile, int& fileSize, int& width, int& height, int& channel, GLenum& format) {
	stbi_set_flip_vertically_on_load(true);
	unsigned char* img_data = stbi_load_from_memory(imgfile, fileSize, &width, &height, &channel, STBI_rgb_alpha);
	if (img_data) {
		if (channel == 1)
			format = GL_RED;
		else if (channel == 3)
			format = GL_RGB;
		else if (channel == 4)
			format = GL_RGBA;
	}
	else
		std::cout << "[ERROR] Load Image " << stbi_failure_reason() << std::endl;
	return img_data;
}
inline unsigned int CreateTexture2D(unsigned char* pixelData, int width, int height, GLenum type) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixelData);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}
inline unsigned int CreateTexture2DFromBMP(const char* path) {
	int fileSize = 0;
	unsigned char* fileContent = LoadFileContent(path, fileSize);
	if (fileContent == nullptr) {
		return 0;
	}
	int img_width, img_height;
	unsigned char* rgbData = DecodeBMP(fileContent, img_width, img_height);
	if (rgbData == nullptr) {
		delete[] fileContent;
		return 0;
	}
	unsigned int texture = CreateTexture2D(rgbData, img_width, img_height, GL_RGB);
	delete[] fileContent;
	return texture;
}
inline unsigned int CreateTexture2DFromLTC(const float* LTC) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_FLOAT, LTC);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}
inline unsigned int CreateTexture2DFromIMG(const char* path) {
	int fileSize = 0;
	unsigned char* fileContent = LoadFileContent(path, fileSize);
	if (fileContent == nullptr) {
		return 0;
	}
	int img_width = 0, img_height = 0, img_channels = 0;
	GLenum img_format = GL_RGB;
	unsigned char* img_data = DecodeIMG(fileContent, fileSize, img_width, img_height, img_channels, img_format);
	unsigned int texture = CreateTexture2D(img_data, img_width, img_height, img_format);
	delete[] img_data;
	return texture;
}
inline unsigned int CreateTexture2DFromCUBE(const char* front, const char* back, const char* left, const char* right,
	const char* top, const char* bottom) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	//int nFileSize = 0, img_width = 0, img_height = 0, img_channel = 0;
	//GLenum img_format = GL_RGB;
	//glGenerateMipmap(GL_TEXTURE_2D);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//// front and back
	//unsigned char* imgFileContent = LoadFileContent(front, nFileSize);
	//unsigned char* front_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, front_img);
	//delete imgFileContent;
	//
	//imgFileContent = LoadFileContent(back, nFileSize);
	//unsigned char* back_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, back_img);
	//delete imgFileContent;
	//
	//// left and right
	//imgFileContent = LoadFileContent(left, nFileSize);
	//unsigned char* left_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, left_img);
	//delete imgFileContent;
	//
	//imgFileContent = LoadFileContent(right, nFileSize);
	//unsigned char* right_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, right_img);
	//delete imgFileContent;
	//
	//// top and bottom
	//imgFileContent = LoadFileContent(top, nFileSize);
	//unsigned char* top_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, top_img);
	//delete imgFileContent;

	//imgFileContent = LoadFileContent(bottom, nFileSize);
	//unsigned char* bottom_img = DecodeIMG(imgFileContent, nFileSize, img_width, img_height, img_channel, img_format);
	//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, img_format, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, bottom_img);
	//delete imgFileContent;
	//return texture;
	int nFileSize = 0, width = 0, height = 0;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	unsigned char* bmpFileContent = LoadFileContent(front, nFileSize);
	unsigned char* pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	delete bmpFileContent;
	bmpFileContent = LoadFileContent(back, nFileSize);
	pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	delete bmpFileContent;
	bmpFileContent = LoadFileContent(left, nFileSize);
	pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	delete bmpFileContent;
	bmpFileContent = LoadFileContent(right, nFileSize);
	pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	delete bmpFileContent;
	bmpFileContent = LoadFileContent(top, nFileSize);
	pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	delete bmpFileContent;
	bmpFileContent = LoadFileContent(bottom, nFileSize);
	pixelData = DecodeBMP(bmpFileContent, width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	delete bmpFileContent;
	return texture;
	
	
}
inline unsigned int CreateTexture2DFromProcedure(int size) {
	unsigned char* img_data = new unsigned char[size * size * 4];
	float halfSize = (float)size / 2.0f;
	float maxDistance = sqrtf(halfSize * halfSize + halfSize * halfSize);
	float centerX = halfSize;
	float centerY = halfSize;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			int currentPixelOffset = (x + y * size) * 4;
			img_data[currentPixelOffset + 0] = 255;
			img_data[currentPixelOffset + 1] = 255;
			img_data[currentPixelOffset + 2] = 255;
			float deltaX = (float)x - centerX;
			float deltaY = (float)y - centerY;
			float distance = sqrtf(deltaX * deltaX + deltaY * deltaY);
			float alpha = powf(1.0f - distance / maxDistance, 8.0f);
			alpha = alpha > 1.0f ? 1.0f : alpha;
			img_data[currentPixelOffset + 3] = (unsigned char)(alpha * 255.0f);
		}
	}
	unsigned int texture = CreateTexture2D(img_data, size, size, GL_RGBA);
	delete[] img_data;
	return texture;
}

inline unsigned int CreateArrayObject() {
	unsigned int object;
	glGenVertexArrays(1, &object);
	glBindVertexArray(object);
	glBindVertexArray(0);
	return object;
}
inline unsigned int CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data) {
	unsigned int object;
	glGenBuffers(1, &object);
	glBindBuffer(bufferType, object);
	glBufferData(bufferType, size, data, usage);
	glBindBuffer(bufferType, 0);
	return object;
}

inline float lerp(float a, float b, float f) {
	return a + f * (b - a);
}

inline void display_4x4(glm::mat4 m4) {
	std::cout << "mat[4x4]" << std::endl;
	for (int col = 0; col < 4; ++col) {
		std::cout << "| " << '\t';
		for (int row = 0; row < 4; ++row) {
			std::cout << m4[col][row] << '\t';
		}
		std::cout << "| " << '\n';
	}
}
inline void display_vec3(glm::vec3 v3) {
	std::cout << "vec3" << "\n| " << v3.x << '\t'
		<< v3.y << '\t' << v3.z << '\t' << std::endl;
}
inline void display_vec3(float* v3) {
	std::cout << "vec3" << "\n| " << v3[0] << '\t'
		<< v3[1] << '\t' << v3[2] << '\t' << std::endl;
}
inline void display_vec4(glm::vec4 v4) {
	std::cout << "vec4" << "\n| " << v4.x << '\t'
		<< v4.y << '\t' << v4.z << '\t' << v4.w << '\t' 
		<< std::endl;
}
inline void display_vec4(float* v4) {
	std::cout << "vec4" << "\n| " << v4[0] << '\t'
		<< v4[1] << '\t' << v4[2] << '\t' << v4[3] << '\t'
		<< std::endl;	
}

// str func
inline const std::string Head(const std::string& str, int n) {
	assert(n >= 0);
	return str.substr(0, std::min(static_cast<size_t>(n), str.size()));
}
inline const std::string Tail(const std::string& str, int n) {
	assert(n >= 0);
	return str.substr(str.size() - n, n);
}
inline const std::string TailAfter(const std::string& str, char c) {
	auto idx = str.find_last_of(c);
	if (idx == std::string::npos)
		return "";
	return str.substr(idx + 1);
}
inline const std::vector<std::string> Spilt(const std::string& str, const std::string& separator) {
	std::vector<std::string> rst;

	if (separator.empty())
		return rst;

	size_t beginIdx = 0;
	while (true) {
		size_t targetIdx = str.find(separator, beginIdx);
		if (targetIdx == std::string::npos) {
			rst.push_back(str.substr(beginIdx, str.size() - beginIdx));
			break;
		}

		rst.push_back(str.substr(beginIdx, targetIdx - beginIdx));
		beginIdx = targetIdx + separator.size();
	}

	return rst;
}
inline inline const std::vector<std::string> Spilt(const std::string& str, char separator) {
	return Spilt(str, std::string(1, separator));
}
inline const std::string Join(const std::vector<std::string>& strs, const std::string& separator = "") {
	std::string rst;

	for (size_t i = 0; i < strs.size() - 1; i++) {
		rst += strs[i];
		rst += separator;
	}

	rst += strs.back();

	return rst;
}
inline const std::string Replace(const std::string& str, const std::string& orig, const std::string& target) {
	return Join(Spilt(str, orig), target);
}
inline const std::string DelTailAfter(const std::string& str, char c) {
	for (auto i = static_cast<int>(str.size()) - 1; i >= 0; i--) {
		if (str[i] == c)
			return str.substr(0, i);
	}

	return str;
}
inline const std::string Between(const std::string& str, char left, char right) {
	auto start = str.find_first_of(left, 0);
	if (start == std::string::npos)
		return "";

	auto end = str.find_last_of(right);
	if (end == std::string::npos || end == start)
		return "";

	return str.substr(start + 1, end - (start + 1));
}
inline const std::string LowerCase(const std::string& str) {
	std::string rst;
	for (auto c : str) {
		if ('A' <= c && c <= 'Z')
			c += 'a' - 'A';
		rst.push_back(c);
	}
	return rst;
}
inline const std::string UpperCase(const std::string& str) {
	std::string rst;
	for (auto c : str) {
		if ('a' <= c && c <= 'z')
			c += 'A' - 'a';
		rst.push_back(c);
	}
	return rst;
}
inline bool IsBeginWith(const std::string& str, const std::string& suffix) {
	return Head(str, static_cast<int>(suffix.size())) == suffix;
}
inline bool IsEndWith(const std::string& str, const std::string& postfix) {
	return Tail(str, static_cast<int>(postfix.size())) == postfix;
}

class CFILE {
public:
	enum Mode {
		READ,
		WRITE
	};

	CFILE(const std::string& fileName, Mode mode) {
		this->fileName = fileName;
		this->mode = mode;
		int err = fopen_s(&pFile, fileName.c_str(), ModeToStr(mode).c_str());
		if (err != 0)
			perror("[ERROR] ");
	}
	~CFILE() {
		if (pFile != NULL) {
			fclose(pFile);
			pFile = NULL;
		}
	}
public:
	bool Check(Mode mode) const {
		if (mode == this->mode && IsValid())
			return true;
		else
			return false;
	}
	int Printf(const char* format, ...) {
		if (!Check(WRITE)) {
			fprintf(stderr, "%s can't not write\n", fileName.c_str());
			return -1;
		}
		va_list args;
		va_start(args, format);
		int rst = vfprintf(pFile, format, args);
		va_end(args);
		return rst;
	}
	int Scanf(const char* format, ...) const {
		if (!Check(READ)) {
			fprintf(stderr, "%s can't not read\n", fileName.c_str());
			return -1;
		}
		va_list args;
		va_start(args, format);
		int rst = vfscanf(pFile, format, args);
		va_end(args);
		return rst;
	}
	std::string ReadLine() const {
		if (!Check(READ)) {
			fprintf(stderr, "%s can't not read\n", fileName.c_str());
			return "";
		}
		char buffer[1024] = "";
		fgets(buffer, 1024, pFile);
		return buffer;
	}
	std::string ReadAll() const {
		if (!Check(READ)) {
			fprintf(stderr, "%s can't not read\n", fileName.c_str());
			return "";
		}
		std::string rst;
		while (!IsEnd())
			rst += ReadLine();
		return rst;
	}
	std::vector<std::string> ReadAllLines() const {
		std::vector<std::string> rst;

		if (!Check(READ)) {
			fprintf(stderr, "%s can't not read\n", fileName.c_str());
			return rst;
		}

		while (!IsEnd())
			rst.push_back(ReadLine());

		return rst;
	}
	bool IsEnd() const {
		return feof(pFile);
	}
	bool IsValid() const {
		return pFile != NULL;
	}
	void Close() {
		if (!IsValid())
			return;

		fclose(pFile);
		pFile = NULL;
	}

public: // static
	static const std::string ReadAll(const std::string& fileName) {
		CFILE file(fileName, Mode::READ);
		return file.ReadAll();
	}
	static const std::vector<std::string> ReadAllLines(const std::string& fileName) {
		CFILE file(fileName, Mode::READ);
		return file.ReadAllLines();
	}
	static bool IsSame(const std::string& fileName0, const std::string& fileName1) {
		auto str0 = ReadAll(fileName0);
		auto str1 = ReadAll(fileName1);
		return str0 == str1;
	}

private:
	std::string ModeToStr(Mode mode) {
		switch (mode)
		{
		case READ:
			return "r";
			break;
		case WRITE:
			return "w";
			break;
		default:
			return "";
			break;
		}
	}
private:
	FILE* pFile;
	Mode mode;
	std::string fileName;
};
