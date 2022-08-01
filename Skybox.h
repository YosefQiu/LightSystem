#pragma once
#include "ggl.h"
#include "Model.h"

class Skybox
{
public:
	Model* mSkybox;
public:
	void Init(const char* path, const char* type = "jpg") {
        std::vector<std::string> skyboxImg;
        skyboxImg.push_back((std::string)path + "back."   + (std::string)type);
        skyboxImg.push_back((std::string)path + "front."  + (std::string)type);
        skyboxImg.push_back((std::string)path + "left."   + (std::string)type);
        skyboxImg.push_back((std::string)path + "right."  + (std::string)type);
        skyboxImg.push_back((std::string)path + "bottom." + (std::string)type);
        skyboxImg.push_back((std::string)path + "top."    + (std::string)type);
        
        mSkybox = new Model;
        mSkybox->Init(CUBE, false, false);
        mSkybox->SetShader("Res/skybox.vs", "Res/skybox.fs");
        mSkybox->mShader->SetTextureCube("U_Texture", CreateTexture2DFromCUBE(skyboxImg[0].c_str(), skyboxImg[1].c_str(), skyboxImg[2].c_str(), skyboxImg[3].c_str(), skyboxImg[4].c_str(), skyboxImg[5].c_str()));
	}

    void Render(glm::mat4& view, glm::mat4& proj, glm::vec3 cameraPos) {
        mSkybox->mShader->SetVec4(CAMPOS.c_str(), glm::vec4(cameraPos, 1.0f));
        glDisable(GL_DEPTH_TEST);
        mSkybox->EnableShadow(false);
        mSkybox->mVertexBuffer->Bind();
        if (mSkybox->mShader->mProgram > 0) {
            glm::mat4 tmp = glm::mat4(1.0f);
            mSkybox->mModelMatrix = glm::translate(tmp, cameraPos);
            mSkybox->mShader->Bind(glm::value_ptr((mSkybox->mModelMatrix)), glm::value_ptr(view), glm::value_ptr(proj));
            //mSkybox->Render(view, proj, cameraPos);
            glDrawArrays(GL_TRIANGLES, 0, mSkybox->mVertexBuffer->mVertexCount);
            mSkybox->mVertexBuffer->Unbind();
        }
        glEnable(GL_DEPTH_TEST);
    }
};
