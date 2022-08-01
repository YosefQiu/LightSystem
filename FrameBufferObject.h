#pragma once
#include "ggl.h"

class FrameBufferObject {
public:
    int* mViewport;
    unsigned int mFrameBufferObject;
    int mPrevFrameBuffer;
    std::map<std::string, GLuint> mBuffers;
    std::vector<GLenum> mDrawBuffers;
public:
    FrameBufferObject() {
        glGenFramebuffers(1, &mFrameBufferObject);
    }
public:
    void AttachColorBuffer(const char* bufferName, GLenum attachment, int width, int height, int filter = GL_LINEAR, bool bFloat = false) {
        GLuint colorBuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        if (bFloat) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        }
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, colorBuffer, 0);
        mDrawBuffers.push_back(attachment);
        mBuffers.insert(std::pair<std::string, GLuint>(bufferName, colorBuffer));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void AttachDepthBuffer(const char* bufferName, int width, int height) {
        GLuint depthMap;
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        mBuffers.insert(std::pair<std::string, GLuint>(bufferName, depthMap));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void Finish() {
        int nCount = (int)mDrawBuffers.size();
        if (nCount > 0) {
            GLenum* buffers = new GLenum[nCount];
            int i = 0;
            while (i < nCount) {
                buffers[i] = mDrawBuffers[i];
                i++;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
            glDrawBuffers(nCount, buffers);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    void Bind() {
        mViewport = new int[4];
        glGetIntegerv(GL_VIEWPORT, mViewport);
        glViewport(mViewport[0], mViewport[1], mViewport[2], mViewport[3]);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mPrevFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, mPrevFrameBuffer);
        glGetIntegerv(GL_VIEWPORT, mViewport);
        glViewport(mViewport[0], mViewport[1], mViewport[2], mViewport[3]);
    }
    unsigned int GetBuffer(const char* bufferName) {
        auto iter = mBuffers.find(bufferName);
        if (iter != mBuffers.end()) {
            return iter->second;
        }
        return 0;
    }
};
