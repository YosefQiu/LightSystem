#pragma once
#include "ggl.h"
#include "eigen/Eigen/Dense"

enum LightType : int {LIGHT_DIRECTION, LIGHT_POINT, LIGHT_SPOT, LIGHT_SPHERE, LIGHT_RECT, LIGHT_DISK, LIGHT_CYLINDER};

class TheoryLight
{
public:
	LightType mType;
	glm::vec3 mPosition;
	glm::vec3 mDirection;
	glm::vec3 mLightClr;
	glm::vec3 mLightAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 mLightOpt;
	float mConstantFactor;
	float mLinearFactor;
	float mExpFactor;
	float mOutter = 64.0f;
public:
	TheoryLight(LightType type, glm::vec3 pos, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3 direction = glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3 opt = glm::vec3(32.0f, 2.0f, 0.0f)) {
		mType = type;
		mLightClr = color;
		mPosition = pos;
		mDirection = direction;
		mLightOpt = opt;
	}
	void SetOutter(float x) {
		mOutter = x;
	}
	void SetOption(glm::vec3 x) {
		mLightOpt = x;
	}
	void SetOption(float x, float y, float z) {
		SetOption(glm::vec3(x, y, z));
	}
	void SetDirection(glm::vec3 x) {
		mDirection = x;
	}
	void SetDirection(float x, float y, float z) {
		SetDirection(glm::vec3(x, y, z));
	}
	void SetDirectionbyAngle(glm::vec3 x) {
		mDirection = glm::vec3(0.0f, 0.0f, 1.0f);
		mDirection = glm::rotateZ(mDirection, glm::radians(x.z));
		mDirection = glm::rotateX(mDirection, glm::radians(x.x));
		mDirection = glm::rotateY(mDirection, glm::radians(x.y));
	}
	void SetColor(glm::vec3 x) {
		mLightClr = x;
	}
	void SetColor(float x, float y, float z) {
		SetColor(glm::vec3(x, y, z));
	}
};

class AreaLight
{
public:
	LightType mType;
	glm::vec3 mCenter;
	glm::vec3 mLightClr;
	float mIntensity;
	std::vector<glm::vec3> mPoints;
public:
	AreaLight(LightType type, glm::vec3 center, glm::vec3 color, float intensity) {
		mType = type;
		mCenter = center;
		mLightClr = color;
		mIntensity = intensity;
	}
	virtual void UpdatePoints() = 0;
	virtual void Render() {}
};

class RectLight : public AreaLight
{
	glm::vec3 mDirX, mDirY;
	float mHalfX, mHalfY;
public:
	RectLight(LightType type, glm::vec3 center, glm::vec3 color, float intensity, glm::vec3 dirX, glm::vec3 dirY, float halfX, float halfY) :
		mDirX(dirX), mDirY(dirY), mHalfX(halfX), mHalfY(halfY), AreaLight(type, center, color, intensity) {
		UpdatePoints();
	}
	void UpdatePoints() {
		glm::vec3 ex = mHalfX * mDirX;
		glm::vec3 ey = mHalfY * mDirY;

		mPoints.clear();
		mPoints.push_back(mCenter - ex - ey);
		mPoints.push_back(mCenter + ex - ey);
		mPoints.push_back(mCenter + ex + ey);
		mPoints.push_back(mCenter - ex + ey);
	}
};
class DiskLight : public RectLight {};

class CylinderLight : public AreaLight
{
public:
	glm::vec3 mTangent;
	float mLength;
	float mRadius;
public:
	CylinderLight(LightType type, glm::vec3 center, glm::vec3 color, float intensity,
		glm::vec3 tangent, float length, float radius) : mTangent(tangent), mLength(length),
		mRadius(radius), AreaLight(type, center, color, intensity) {
		UpdatePoints();
	}
	void UpdatePoints() {
		mPoints.clear();
		mPoints.push_back(mCenter - 0.5f * mLength * mTangent);
		mPoints.push_back(mCenter + 0.5f * mLength * mTangent);
	}
};

class SphereLight : public AreaLight
{
public:
	glm::vec3 mDirX;
	glm::vec3 mDirY;
	glm::vec3 mDirZ;
	float mLengthX;
	float mLengthY;
	float mLengthZ;
public:
	SphereLight(LightType type, glm::vec3 center, glm::vec3 color, float intensity,
		glm::vec3 dirX, glm::vec3 dirY, glm::vec3 dirZ,
		float lengthX, float lengthY, float lengthZ) :
		mDirX(dirX), mDirY(dirY), mDirZ(dirZ),
		mLengthX(lengthX), mLengthY(lengthY), mLengthZ(lengthZ),
		AreaLight(type, center, color, intensity) {
		UpdatePoints();
	}
	void UpdatePoints() {
		auto origin = glm::vec3(mCenter.x, 0.0f, mCenter.z);
		auto local_center = mCenter - origin;

		auto A = glm::mat3(glm::normalize(mDirX), glm::normalize(mDirY), glm::normalize(mDirZ));
		auto diagonal = glm::mat3(
			mLengthX, 0.0f, 0.0f,
			0.0f, mLengthY, 0.0f,
			0.0f, 0.0f, mLengthZ);
		auto M = A * diagonal * glm::transpose(A);
		auto MinV = glm::inverse(M);
		auto Pb = MinV * local_center;

		auto theta = glm::asin(1.0f / glm::length(Pb));
		auto Pc = glm::cos(theta) * glm::cos(theta) * Pb;
		auto radius = glm::tan(theta) * glm::length(Pc);
		glm::vec3 C1, C2;
		buildOrthonormalBasis(glm::normalize(Pc), C1, C2);

		auto Pd = M * Pc;
		auto D1_ = M * radius * C1;
		auto D2_ = M * radius * C2;

		// ellipse principal axes
		Eigen::Matrix2f Q;
		Q << glm::dot(D1_, D1_), glm::dot(D1_, D2_), glm::dot(D1_, D2_), glm::dot(D2_, D2_);
		Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> eigensolver(Q);
		if (eigensolver.info() != Eigen::Success) {
			abort();
		}
		auto eigenvalues = eigensolver.eigenvalues();
		auto eigenvectors = eigensolver.eigenvectors();

		auto D1 = eigenvectors(0, 0) * D1_ + eigenvectors(1, 0) * D2_;
		auto D2 = eigenvectors(0, 1) * D1_ + eigenvectors(1, 1) * D2_;
		D1 = glm::normalize(D1);
		D2 = glm::normalize(D2);
		auto d1 = glm::sqrt(eigenvalues(0));
		auto d2 = glm::sqrt(eigenvalues(1));

		// update points
		auto ex = D1 * d1;
		auto ey = D2 * d2;

		mPoints.clear();
		mPoints.push_back(mCenter - ex - ey);
		mPoints.push_back(mCenter + ex - ey);
		mPoints.push_back(mCenter + ex + ey);
		mPoints.push_back(mCenter - ex + ey);
	}
private:
	void buildOrthonormalBasis(const glm::vec3 n, glm::vec3& b1, glm::vec3& b2) {
		if (n.z < -0.9999999f)
		{
			b1 = glm::vec3(0.0, -1.0, 0.0);
			b2 = glm::vec3(-1.0, 0.0, 0.0);
			return;
		}
		float a = 1.0f / (1.0f + n.z);
		float b = -n.x * n.y * a;
		b1 = glm::vec3(1.0f - n.x * n.x * a, b, -n.x);
		b2 = glm::vec3(b, 1.0f - n.y * n.y * a, -n.y);
	}
};