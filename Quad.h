#pragma once

#include "ggl.h"
#include "Shader.h"
#include "VertexBuffer.h"

class Quad
{
public:
    VertexBuffer* mVertexBuffer;
    Shader* mShader;
public:
    void Init() {
        mVertexBuffer = new VertexBuffer;
        mVertexBuffer->Init(4);
        mVertexBuffer->SetTexcoord(0, 0.0f, 0.0f);
        mVertexBuffer->SetTexcoord(1, 1.0f, 0.0f);
        mVertexBuffer->SetTexcoord(2, 0.0f, 1.0f);
        mVertexBuffer->SetTexcoord(3, 1.0f, 1.0f);
        mShader = new Shader;
    }
    void Render() {
        glm::mat4 identity = glm::mat4(1.0f);

        mVertexBuffer->SetPosition(0, -1.0f, -1.0f, 0.0f);
        mVertexBuffer->SetPosition(1, 1.0f, -1.0f, 0.0f);
        mVertexBuffer->SetPosition(2, -1.0f, 1.0f, 0.0f);
        mVertexBuffer->SetPosition(3, 1.0f, 1.0f, 0.0f);
        mVertexBuffer->Bind();
        mShader->Bind(identity, identity, identity);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mVertexBuffer->Unbind();
    }
    void DepthBuffer() {
        mShader->SetBool("bDepth", true);
    }
    void ColorBuffer() {
        mShader->SetBool("bDepth", false);
    }
    void RenderToLeftTop() {
        glm::mat4 identity = glm::mat4(1.0f);

        mVertexBuffer->SetPosition(0, -1.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(1, 0.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(2, -1.0f, 1.0f, -1.0f);
        mVertexBuffer->SetPosition(3, 0.0f, 1.0f, -1.0f);
        mVertexBuffer->Bind();
        mShader->Bind(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mVertexBuffer->Unbind();
    }
    void RenderToRightTop() {
        glm::mat4 identity = glm::mat4(1.0f);

        mVertexBuffer->SetPosition(0, 0.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(1, 1.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(2, 0.0f, 1.0f, -1.0f);
        mVertexBuffer->SetPosition(3, 1.0f, 1.0f, -1.0f);
        mVertexBuffer->Bind();
        mShader->Bind(identity, identity, identity);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mVertexBuffer->Unbind();
    }
    void RenderToLeftBottom() {
        glm::mat4 identity = glm::mat4(1.0f);

        mVertexBuffer->SetPosition(0, -1.0f, -1.0f, -1.0f);
        mVertexBuffer->SetPosition(1, 0.0f, -1.0f, -1.0f);
        mVertexBuffer->SetPosition(2, -1.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(3, 0.0f, 0.0f, -1.0f);
        mVertexBuffer->Bind();
        mShader->Bind(identity, identity, identity);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mVertexBuffer->Unbind();
    }
    void RenderToRightBottom() {
        glm::mat4 identity = glm::mat4(1.0f);

        mVertexBuffer->SetPosition(0, 0.0f, -1.0f, -1.0f);
        mVertexBuffer->SetPosition(1, 1.0f, -1.0f, -1.0f);
        mVertexBuffer->SetPosition(2, 0.0f, 0.0f, -1.0f);
        mVertexBuffer->SetPosition(3, 1.0f, 0.0f, -1.0f);
        mVertexBuffer->Bind();
        mShader->Bind(identity, identity, identity);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mVertexBuffer->Unbind();
    }
};