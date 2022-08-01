#pragma once

#include "ggl.h"
#include "Utils.h"

struct Vertex {
    float Position[4];
    float Color[4];
    float Texcoord[4];
    float Normal[4];
    float Tangent[4];
    float Bitangent[4];
};
struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class VertexBuffer
{
public:
    Vertex* mVertexes;
    std::vector<Vertex> mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<Texture> mTextures;
    unsigned int mVBO;
    unsigned int mVAO;
    unsigned int mEBO;
    int mBufferSize;
    int mIndicesSize;
    int mVertexCount;
public:
    VertexBuffer() {}
    VertexBuffer(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->mVertices = vertices;
        this->mIndices = indices;
        this->mTextures = textures;
        mBufferSize = sizeof(Vertex) * vertices.size();
        mIndicesSize = sizeof(unsigned int) * indices.size();
        mVAO = CreateArrayObject();
        mVBO = CreateBufferObject(GL_ARRAY_BUFFER, mBufferSize, GL_STATIC_DRAW, nullptr);
        mEBO = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, mIndicesSize, GL_STATIC_DRAW, nullptr);
    }
    void Init(int size) {
        mVertexes = new Vertex[size];
        mBufferSize = sizeof(Vertex) * size;
        mVertexCount = size;
        mVAO = CreateArrayObject();
        mVBO = CreateBufferObject(GL_ARRAY_BUFFER, mBufferSize, GL_STATIC_DRAW, nullptr);
    }
    void SetPosition(int index, float x, float y, float z, float w = 1.0f) {
        mVertexes[index].Position[0] = x;
        mVertexes[index].Position[1] = y;
        mVertexes[index].Position[2] = z;
        mVertexes[index].Position[3] = w;
    }
    void SetColor(int index, float r, float g, float b, float a = 1.0f) {
        mVertexes[index].Color[0] = r;
        mVertexes[index].Color[1] = g;
        mVertexes[index].Color[2] = b;
        mVertexes[index].Color[3] = a;
    }
    void SetNormal(int index, float x, float y, float z) {
        mVertexes[index].Normal[0] = x;
        mVertexes[index].Normal[1] = y;
        mVertexes[index].Normal[2] = z;
        mVertexes[index].Normal[3] = 1.0f;
    }
    void SetTexcoord(int index, float u, float v) {
        mVertexes[index].Texcoord[0] = u;
        mVertexes[index].Texcoord[1] = v;
        mVertexes[index].Texcoord[2] = 0.0f;
        mVertexes[index].Texcoord[3] = 0.0f;
    }
    void SetTangent(int index, float x, float y, float z) {
        mVertexes[index].Tangent[0] = x;
        mVertexes[index].Tangent[1] = y;
        mVertexes[index].Tangent[2] = z;
        mVertexes[index].Tangent[3] = 0.0f;
    }
    void SetBitangent(int index, float x, float y, float z) {
        mVertexes[index].Bitangent[0] = x;
        mVertexes[index].Bitangent[1] = y;
        mVertexes[index].Bitangent[2] = z;
        mVertexes[index].Bitangent[3] = 0.0f;
    }
    void Bind() {
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mBufferSize, mVertexes);
    }
    void NewBind() {
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mBufferSize, &mVertices[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mIndicesSize, &mIndices[0]);
    }
    void Unbind() {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};