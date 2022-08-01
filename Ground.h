#pragma once
#include "ggl.h"
#include "Model.h"

class Ground : public Model
{
public:
	void Init() {
		mVertexBuffer = new VertexBuffer;
		mVertexBuffer->Init(1600);

		Vertex* vertex = new Vertex[1600];
		for (int z = 0; z < 20; z++) {
			float zStart = -100.0f + z * 10.0f;
			for (int x = 0; x < 20; x++) {
				int offset = (x + z * 20) * 4;
				float xStart = -100.0f + x * 10.0f;

				mVertexBuffer->SetPosition(offset + 0, xStart, -1.0f, zStart);
				mVertexBuffer->SetPosition(offset + 1, xStart + 10.0f, -1.0f, zStart);
				mVertexBuffer->SetPosition(offset + 2, xStart, -1.0f, zStart - 10.0f);
				mVertexBuffer->SetPosition(offset + 3, xStart + 10.0f, -1.0f, zStart - 10.0f);

				mVertexBuffer->SetNormal(offset + 0, 0.0f, 1.0f, 0.0f);
				mVertexBuffer->SetNormal(offset + 1, 0.0f, 1.0f, 0.0f);
				mVertexBuffer->SetNormal(offset + 2, 0.0f, 1.0f, 0.0f);
				mVertexBuffer->SetNormal(offset + 3, 0.0f, 1.0f, 0.0f);


				if ((z % 2) ^ (x % 2)) {
					mVertexBuffer->SetColor(offset + 0, 0.1f, 0.1f, 0.1f);
					mVertexBuffer->SetColor(offset + 1, 0.1f, 0.1f, 0.1f);
					mVertexBuffer->SetColor(offset + 2, 0.1f, 0.1f, 0.1f);
					mVertexBuffer->SetColor(offset + 3, 0.1f, 0.1f, 0.1f);
				}
				else {
					mVertexBuffer->SetColor(offset + 0, 0.8f, 0.8f, 0.8f);
					mVertexBuffer->SetColor(offset + 1, 0.8f, 0.8f, 0.8f);
					mVertexBuffer->SetColor(offset + 2, 0.8f, 0.8f, 0.8f);
					mVertexBuffer->SetColor(offset + 3, 0.8f, 0.8f, 0.8f);
				}
			}
		}
		mShader = new Shader;
		mShader = nullptr;
	}
	void Render(glm::mat4& viewMatrix, glm::mat4& projMatrix) {
		if (mShader == nullptr) {
			std::cout << "ERROR:: No set shader..." << std::endl;
			return;
		}

		glEnable(GL_DEPTH_TEST);
		mVertexBuffer->Bind();
		mShader->SetVec4("U_LightPos", 0.0f, 1.0f, 1.0f, 0.0f);
		mShader->SetVec4("U_LightAmbient", 0.1f, 0.1f, 0.1f, 1.0f);
		mShader->SetVec4("U_LightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
		SetAmbientMaterial(0.1f, 0.1f, 0.1f, 1.0f);
		SetDiffuseMaterial(0.1f, 0.4f, 0.6f, 1.0f);
		glm::mat4 it = glm::inverseTranspose(mModelMatrix);
		mShader->Bind(glm::value_ptr(mModelMatrix), glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix));
		mShader->SetMat4("IT_ModelMatrix", it);
		for (int i = 0; i < 400; i++) {
			glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
		}

		mVertexBuffer->Unbind();
		mShader->Unbind();
	}
	void SetAmbientMaterial(float r, float g, float b, float a) {
		mShader->SetVec4("U_AmbientMaterial", r, g, b, a);
	}
	void SetDiffuseMaterial(float r, float g, float b, float a) {
		mShader->SetVec4("U_DiffuseMaterial", r, g, b, a);
	}
	void SetSpecularMaterial(float r, float g, float b, float a) {
		mShader->SetVec4("U_SpecularMaterial", r, g, b, a);
	};
};