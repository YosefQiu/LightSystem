#pragma once
#include "ggl.h"
#include "Utils.h"

class Camera
{
public:
    glm::vec3 mPosition;
    glm::vec3 mTarget;
    glm::vec3 mUp;
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjMatrix = glm::mat4(1.0f);
    float mMoveSpeed;
    bool mbForward, mbBackward, mbLeft, mbRight, mbMouseLeft = false;
public:
    Camera() {
        mPosition = glm::vec3(0.0f, 4.0f, 0.0f);
        mTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        mUp = glm::vec3(0.0f, 1.0f, 0.0f);
        mMoveSpeed = 1.0f;
    }
public:
    void Init(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) {
        mPosition = pos;
        mTarget = target;
        mUp = up;
    }
    void Update(float delta) {
        glm::vec3 viewDir = mTarget - mPosition;
        glm::normalize(viewDir);
        glm::vec3 rightDir = glm::cross(viewDir, mUp);
        glm::normalize(rightDir);
        float factor = mMoveSpeed * delta;
        if (mbForward) {
            glm::vec3 distance = viewDir * factor;
            mPosition = mPosition + distance;
            mTarget = mTarget + distance;
        }
        if (mbBackward) {
            glm::vec3 distance = viewDir * factor;
            mPosition = mPosition - distance;
            mTarget = mTarget - distance;
        }
        if (mbLeft) {
            glm::vec3 distance = rightDir * factor;
            mPosition = mPosition + distance;
            mTarget = mTarget + distance;
        }
        if (mbRight) {
            glm::vec3 distance = rightDir * factor;
            mPosition = mPosition - distance;
            mTarget = mTarget - distance;
        }
        mViewMatrix = glm::lookAt(mPosition, mTarget, mUp);
    }
    void Pitch(float angle) {
        glm::vec3 viewDir = mTarget - mPosition;
        glm::normalize(viewDir);
        glm::vec3 rightDir = glm::cross(viewDir, mUp);
        glm::normalize(rightDir);
        RotateView(angle, rightDir.x, rightDir.y, rightDir.z);
    }
    void Yaw(float angle) {
        RotateView(angle, mUp.x, mUp.y, mUp.z);
    }
    void RotateView(float angle, float x, float y, float z) {
        glm::vec3 viewDir = mTarget - mPosition;
        glm::normalize(viewDir);
        glm::vec3 newDir = glm::vec3(0.0f, 0.0f, 0.0f);
        float C = cosf(angle);
        float S = sinf(angle);
        glm::vec3 tmpX = glm::vec3(C + x * x * (1 - C), x * y * (1 - C) - z * S, x * z * (1 - C) + y * S);
        glm::vec3 tmpY = glm::vec3(x * y * (1 - C) + z * S, C + y * y * (1 - C), y * z * (1 - C) - x * S);
        glm::vec3 tmpZ = glm::vec3(x * z * (1 - C) - y * S, y * z * (1 - C) + x * S, C + z * z * (1 - C));
        newDir.x = glm::dot(tmpX, viewDir);
        newDir.y = glm::dot(tmpY, viewDir);
        newDir.z = glm::dot(tmpZ, viewDir);
        mTarget = mPosition + newDir;
    }
    void SetMoveSpeed(float speed) {
        mMoveSpeed = speed;
    }
    void ChangePosition(glm::vec3 pos) {
        Init(pos, mTarget);
    }
    void ChangeTarget(glm::vec3 target) {
        Init(mPosition, target);
    }
    glm::mat4& GetViewMatrix() {
        return mViewMatrix;
    }
    glm::mat4& GetProjMatrix() {
        return mProjMatrix;
    }
    glm::vec3& GetPosition() {
        return mPosition;
    }
    void SwitchTo2D(float width, float height) {
        mProjMatrix = glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -1000.0f, 1000.0f);
    }
    void SwitchTo3D(float width, float height) {
        mProjMatrix = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
    }
};