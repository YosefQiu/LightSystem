#pragma once
#include "ggl.h"
#include "Utils.h"
#include "VertexBuffer.h"

struct UniformTexture {
	unsigned int mTexture;
	int mLocation;
	UniformTexture() {
		mLocation = -1;
		mTexture = 0;
	}
};

struct UniformTextureCube {
	unsigned int mTexture;
	int mLocation;
	UniformTextureCube() {
		mLocation = -1;
		mTexture = 0;
	}
};

struct UniformVec4 {
	int mLocation;
	float v[4];
	UniformVec4() {
		mLocation = -1;
		memset(v, 0, sizeof(float) * 4);
	}
};

struct UniformFloat {
	int mLocation;
	float v;
	UniformFloat() {
		mLocation = -1;
		v = 0;
	}
};

struct UniformBool {
	int mLocation;
	bool v;
	UniformBool() {
		mLocation = -1;
		v = false;
	}
};

struct UniformMat4 {
	int mLocation;
	glm::mat4 v;
	UniformMat4() {
		mLocation = -1;
		v = glm::mat4(1.0f);
	}
};

class Shader
{
public:
	unsigned int mProgram;
	int mPositionLocation, mColorLocation, mNormalLocation, mTexcoordLocation, mTangentLocation, mBitangentLocation, mModelMatrixLocation, mViewMatrixLocation, mProjectionMatrixLocation;
	std::map<std::string, UniformTexture*>		mUniformTextures;
	std::map<std::string, UniformTextureCube*>	mUniformTextureCubes;
	std::map<std::string, UniformVec4*>			mUniformVec4s;
	std::map<std::string, UniformFloat*>		mUniformFloats;
	std::map<std::string, UniformMat4*>			mUniformMat4s;
	std::map<std::string, UniformBool*>			mUniformBools;
public:
	void Init(const char* vertex_shader_file, const char* fragment_shader_file, 
		const char* geom_shader_file = nullptr, 
		const char* control_shader_file = nullptr, const char* evalua_shader_file = nullptr, bool bDebug = false) {
		unsigned int vsShader = -1, fsShader = -1, gsShader = -1, csShader = -1, esShader = -1;
		std::string shaderCode = PrewProcess(std::string(vertex_shader_file));
		vsShader = CompileShader(GL_VERTEX_SHADER, shaderCode.c_str());
		shaderCode.clear();
		shaderCode = PrewProcess(std::string(fragment_shader_file), bDebug);
		fsShader = CompileShader(GL_FRAGMENT_SHADER, shaderCode.c_str());
		shaderCode.clear();
		if (geom_shader_file != nullptr) {
			shaderCode = PrewProcess(std::string(geom_shader_file));
			gsShader = CompileShader(GL_GEOMETRY_SHADER, shaderCode.c_str());
			shaderCode.clear();
		}
		if (control_shader_file != nullptr) {
			shaderCode = PrewProcess(std::string(control_shader_file));
			csShader = CompileShader(GL_TESS_CONTROL_SHADER, shaderCode.c_str());
			shaderCode.clear();
		}
		if (evalua_shader_file != nullptr) {
			shaderCode = PrewProcess(std::string(evalua_shader_file));
			csShader = CompileShader(GL_TESS_EVALUATION_SHADER, shaderCode.c_str());
			shaderCode.clear();
		}
		mProgram = CreateProgram(vsShader, fsShader, gsShader, csShader, esShader);
		glDeleteShader(vsShader);
		glDeleteShader(fsShader);
		if (geom_shader_file != nullptr) {
			glDeleteShader(gsShader);
		}
		if (control_shader_file != nullptr) {
			glDeleteShader(csShader);
		}
		if (evalua_shader_file != nullptr) {
			glDeleteShader(esShader);
		}

		if (mProgram != 0) {
			mPositionLocation = glGetAttribLocation(mProgram, POSITION.c_str());
			mColorLocation = glGetAttribLocation(mProgram, COLOR.c_str());
			mNormalLocation = glGetAttribLocation(mProgram, NORMAL.c_str());
			mTexcoordLocation = glGetAttribLocation(mProgram, TEXCOORD.c_str());
			mTangentLocation = glGetAttribLocation(mProgram, TANGENT.c_str());
			mBitangentLocation = glGetAttribLocation(mProgram, BITANGENT.c_str());

			mModelMatrixLocation = glGetUniformLocation(mProgram, MODELMATRIX.c_str());
			mViewMatrixLocation = glGetUniformLocation(mProgram, VIEWMATRIX.c_str());
			mProjectionMatrixLocation = glGetUniformLocation(mProgram, PROJMATRIX.c_str());
		}
	}
	void Bind(glm::mat4& M, glm::mat4& V, glm::mat4& P) {
		Bind(glm::value_ptr(M), glm::value_ptr(V), glm::value_ptr(P));
	}
	void Bind(float* M, float* V, float* P) {
		glUseProgram(mProgram);
		if (mModelMatrixLocation >= 0)glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, M);
		if (mViewMatrixLocation >= 0)glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, V);
		if (mProjectionMatrixLocation >= 0)glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, P);
		int iIndex = 0;
		for (auto iter = mUniformTextures.begin(); iter != mUniformTextures.end(); ++iter) {
			glActiveTexture(GL_TEXTURE0 + iIndex);
			glBindTexture(GL_TEXTURE_2D, iter->second->mTexture);
			glUniform1i(iter->second->mLocation, iIndex++);
		}
		for (auto iter = mUniformTextureCubes.begin(); iter != mUniformTextureCubes.end(); ++iter) {
			glActiveTexture(GL_TEXTURE0 + iIndex);
			glBindTexture(GL_TEXTURE_CUBE_MAP, iter->second->mTexture);
			glUniform1i(iter->second->mLocation, iIndex++);
		}
		for (auto iter = mUniformVec4s.begin(); iter != mUniformVec4s.end(); ++iter) {
			glUniform4fv(iter->second->mLocation, 1, iter->second->v);
		}
		for (auto iter = mUniformFloats.begin(); iter != mUniformFloats.end(); ++iter) {
			glUniform1fv(iter->second->mLocation, 1, &(iter->second->v));
		}
		for (auto iter = mUniformBools.begin(); iter != mUniformBools.end(); ++iter) {
			glUniform1i(iter->second->mLocation, iter->second->v);
		}
		for (auto iter = mUniformMat4s.begin(); iter != mUniformMat4s.end(); ++iter) {
			glUniformMatrix4fv(iter->second->mLocation, 1, GL_FALSE, glm::value_ptr(iter->second->v));
		}
		if (mPositionLocation >= 0) {
			glEnableVertexAttribArray(mPositionLocation);
			glVertexAttribPointer(mPositionLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		}
		if (mColorLocation >= 0) {
			glEnableVertexAttribArray(mColorLocation);
			glVertexAttribPointer(mColorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 4));
		}
		if (mTexcoordLocation >= 0) {
			glEnableVertexAttribArray(mTexcoordLocation);
			glVertexAttribPointer(mTexcoordLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));
		}
		if (mNormalLocation >= 0) {
			glEnableVertexAttribArray(mNormalLocation);
			glVertexAttribPointer(mNormalLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12));
		}
		if (mTangentLocation >= 0) {
			glEnableVertexAttribArray(mTangentLocation);
			glVertexAttribPointer(mTangentLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 16));
		}
		if (mBitangentLocation >= 0) {
			glEnableVertexAttribArray(mBitangentLocation);
			glVertexAttribPointer(mBitangentLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 20));
		}
	}
	unsigned int SetTexture(const char* sampler2DName, unsigned int texture) {
		auto iter = mUniformTextures.find(sampler2DName);
		unsigned oldTexture = 0;
		if (iter == mUniformTextures.end()) {
			GLint location = glGetUniformLocation(mProgram, sampler2DName);
			if (location != -1) {
				UniformTexture* t = new UniformTexture;
				t->mLocation = location;
				t->mTexture = texture;
				mUniformTextures.insert(std::pair<std::string, UniformTexture*>(sampler2DName, t));
			}
		}
		else {
			oldTexture = iter->second->mTexture;
			iter->second->mTexture = texture;
		}
		return oldTexture;
	}
	void SetTexture(const char* sampler2DName, const char* image) {
		auto iter = mUniformTextures.find(sampler2DName);
		if (iter == mUniformTextures.end()) {
			GLint location = glGetUniformLocation(mProgram, sampler2DName);
			if (location != -1) {
				UniformTexture* t = new UniformTexture;
				t->mLocation = location;
				t->mTexture = CreateTexture2DFromIMG(image);
				mUniformTextures.insert(std::pair<std::string, UniformTexture*>(sampler2DName, t));
			}
		}
		else {
			glDeleteTextures(1, &iter->second->mTexture);
			iter->second->mTexture = CreateTexture2DFromIMG(image);
		}
	}
	void SetVec4(const char* name, float x, float y, float z, float w) {
		auto iter = mUniformVec4s.find(name);
		if (iter == mUniformVec4s.end()) {
			int location = glGetUniformLocation(mProgram, name);
			if (location != -1) {
				UniformVec4* v = new UniformVec4;
				v->v[0] = x;
				v->v[1] = y;
				v->v[2] = z;
				v->v[3] = w;
				v->mLocation = location;
				mUniformVec4s.insert(std::pair<std::string, UniformVec4*>(name, v));
			}
		}
		else {
			iter->second->v[0] = x;
			iter->second->v[1] = y;
			iter->second->v[2] = z;
			iter->second->v[3] = w;
		}
	}
	void SetVec4(const char* name, glm::vec4 src) {
		SetVec4(name, src.x, src.y, src.z, src.w);
	}
	void SetVec4(const char* name, glm::vec3 src) {
		SetVec4(name, glm::vec4(src, 1.0f));
	}
	void SetFloat(const char* name, float x) {
		auto iter = mUniformFloats.find(name);
		if (iter == mUniformFloats.end()) {
			int location = glGetUniformLocation(mProgram, name);
			if (location != -1) {
				UniformFloat* v = new UniformFloat;
				v->v = x;
				v->mLocation = location;
				mUniformFloats.insert(std::pair<std::string, UniformFloat*>(name, v));
			}
		}
		else {
			iter->second->v = x;
		}
	}
	void SetBool(const char* name, bool x) {
		auto iter = mUniformBools.find(name);
		if (iter == mUniformBools.end()) {
			int location = glGetUniformLocation(mProgram, name);
			if (location != -1) {
				UniformBool* v = new UniformBool;
				v->v = x;
				v->mLocation = location;
				mUniformBools.insert(std::pair<std::string, UniformBool*>(name, v));
			}
		}
		else {
			iter->second->v = x;
		}
	}
	void SetMat4(const char* name, glm::mat4 src) {
		auto iter = mUniformMat4s.find(name);
		if (iter == mUniformMat4s.end()) {
			int location = glGetUniformLocation(mProgram, name);
			if (location != -1) {
				UniformMat4* v = new UniformMat4;
				v->v = src;
				v->mLocation = location;
				mUniformMat4s.insert(std::pair<std::string, UniformMat4*>(name, v));
			}
		}
		else {
			iter->second->v = src;
		}
	}
	void SetTextureCube(const char* samplercubeName, unsigned int texture) {
		auto iter = mUniformTextureCubes.find(samplercubeName);
		unsigned int oldTexture = 0;
		if (iter == mUniformTextureCubes.end()) {
			int location = glGetUniformLocation(mProgram, samplercubeName);
			if (location != -1) {
				UniformTextureCube* ut = new UniformTextureCube;
				ut->mLocation = location;
				ut->mTexture = texture;
				mUniformTextureCubes.insert(std::pair<std::string, UniformTextureCube*>(samplercubeName, ut));
			}
		}
		else {
			oldTexture = iter->second->mTexture;
			iter->second->mTexture = texture;
		}
	}
	void SetTextureCube(const char* samplercubeName, const char* image, const char* imgType = "bmp") {
		std::vector<std::string> skyboxImg;
		skyboxImg.push_back((std::string)image + "back."	+ (std::string)imgType);
		skyboxImg.push_back((std::string)image + "front."	+ (std::string)imgType);
		skyboxImg.push_back((std::string)image + "left."	+ (std::string)imgType);
		skyboxImg.push_back((std::string)image + "right."	+ (std::string)imgType);
		skyboxImg.push_back((std::string)image + "bottom."	+ (std::string)imgType);
		skyboxImg.push_back((std::string)image + "top."		+ (std::string)imgType);

		unsigned int texture = CreateTexture2DFromCUBE(skyboxImg[0].c_str(), skyboxImg[1].c_str(), skyboxImg[2].c_str(), skyboxImg[3].c_str(), skyboxImg[4].c_str(), skyboxImg[5].c_str());
		if (texture != 0) {
			SetTextureCube(samplercubeName, texture);
		}
	}
	void Unbind() {
		glUseProgram(0);
	}
	void Use() {
		glUseProgram(mProgram);
	}
	void SetPara(std::function<void()> foo) {
		foo();
	}
private:
	unsigned int CompileShader(GLenum shaderType, const char* shaderCode) {
		unsigned int shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &shaderCode, nullptr);
		glCompileShader(shader);
		if (CheckError(shader, GL_COMPILE_STATUS) == -1) {
			glDeleteShader(shader);
			shader = 0;
		}
		return shader;
	}
	unsigned int CreateProgram(unsigned int vsShader, unsigned int fsShader, unsigned int gShader = -1, unsigned int cShader = -1, unsigned int eShader = -1) {
		unsigned int program = glCreateProgram();
		glAttachShader(program, vsShader);
		glAttachShader(program, fsShader);
		if (gShader != -1) {
			glAttachShader(program, gShader);
		}
		if (cShader != -1) {
			glAttachShader(program, cShader);
		}
		if (eShader != -1) {
			glAttachShader(program, eShader);
		}
		glLinkProgram(program);
		glDetachShader(program, vsShader);
		glDetachShader(program, fsShader);
		if (gShader != -1) {
			glDetachShader(program, gShader);
		}
		if (cShader != -1) {
			glDetachShader(program, cShader);
		}
		if (eShader != -1) {
			glDetachShader(program, eShader);
		}
		if (CheckError(program, GL_LINK_STATUS) == -1) {
			glDeleteProgram(program);
			program = 0;
		}
		return program;
	}
	unsigned int CheckError(unsigned int candidate, GLenum pname) {
		int err = 0;
		int szlogLen = 0;
		char* szLog = new char[1024];
		memset(szLog, 0, 1024);
		switch (pname) {
		case GL_COMPILE_STATUS:
		{
			err = GL_TRUE;
			glGetShaderiv(candidate, GL_COMPILE_STATUS, &err);
			if (err == GL_FALSE) {
				glGetShaderInfoLog(candidate, 1024, &szlogLen, szLog);
				std::cout << "ERROR::Compile Shader fail-->error log : " << szLog << std::endl;
				return -1;
			}
		}
		break;
		case GL_LINK_STATUS:
		{
			err = GL_TRUE;
			glGetProgramiv(candidate, GL_LINK_STATUS, &err);
			if (err == GL_FALSE) {
				glGetProgramInfoLog(candidate, 1024, &szlogLen, szLog);
				std::cout << "[ERROR] Create gpu program fail-->error log : " << szLog << std::endl;
				return -1;
			}
		}
		break;
		}
		return 0;
	}
	unsigned int GetUniform(const char* name) {
		int position = glGetUniformLocation(mProgram, name);
		if (position == -1)
		{
			std::cout << "[SHADER]ERROR::UNIFORM OF NAME: " << name << " SET FAILED..." << "\n -- --------------------------------------------------- --" << std::endl;
		}
		return position;
	}
	std::string PrewProcess(const std::string& path, bool bDebug = false) {
		CFILE shaderFile(path, CFILE::Mode::READ);
		std::vector<std::string> contents = shaderFile.ReadAllLines();

		std::string root = DelTailAfter(path, '/') + "/";
		std::string name = TailAfter(path, '/');

		std::string rst;

		for (const auto& line : contents) {
			if (IsBeginWith(line, "#include")) {
				std::string incPath = Between(line, '"', '"');
				std::string inc = PrewProcess(root + incPath);
				if (inc.empty()) {
					std::cout << "[WARNING] #include is empty or failed" << "\n"
						<< "\t" << "code: " << line.c_str()
						<< "\t" << "path: " << incPath.c_str() << std::endl;
					continue;
				}
				rst += inc;
			}
			else
				rst += line;
		}
		if (bDebug) {
			CFILE debugShadeFile(root + "_DEBUG_" + name, CFILE::Mode::WRITE);
			debugShadeFile.Printf("%s", rst.c_str());
			debugShadeFile.Close();
		}

		return rst;
	}
	std::string CheckShaderName(GLenum shaderType) {
		std::string str;
		switch (shaderType)
		{
		case GL_VERTEX_SHADER:
			str = "VERTEX_SHADER"; break;
		case GL_FRAGMENT_SHADER:
			str = "FRAGMENT_SHADER"; break;
		case GL_GEOMETRY_SHADER:
			str = "GEOMETRY_SHADER"; break;
		case GL_TESS_CONTROL_SHADER:
			str = "TESS_CONTROL_SHADER"; break;
		case GL_TESS_EVALUATION_SHADER:
			str = "TESS_EVALUATION_SHADER"; break;
		default:
			break;
		}
		return str;
	}
};
