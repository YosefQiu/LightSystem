#pragma once
#include "ggl.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include "Light.h"

#include <mutex>


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

std::once_flag flag_lightInfo;
enum GEOMETRY :int {
	CUBE = 0, SPHERE, CYLINDER, QUAD, RING, CIRCLE, TORUSKNOT};
class Model
{
public:
    VertexBuffer* mVertexBuffer;
    Shader* mShader;
    glm::mat4 mModelMatrix = glm::mat4(1.0f);
    glm::mat4 mbLight = glm::mat4(0.0f);
    glm::mat4 mbShadow = glm::mat4(0.0f);
    float* mLightViewMatrix;
    float* mLightProjMatrix;
    std::vector<glm::mat4> mModelChangeVecs;
    std::vector<VertexBuffer> mMeshes;
    std::vector<Texture> mTextureLoaded;
    std::string mDirectory;
    bool mbModel = false;
    bool mbTexture = false;
    bool mbMTL = false;
    bool mbAssimp = false;
public:
    Model() {}
    virtual ~Model() {
       /* delete[] mLightViewMatrix;
        delete[] mLightProjMatrix;*/
        delete mShader;
        mShader = nullptr;
    }
public:
    virtual void Init(GEOMETRY type, bool mtl = false, bool bAssimp = true) {
        std::string path = "Res/Geometry/";
        switch (type){
        case CUBE:
            path += "Cube.obj"; break;
        case SPHERE:
            path += "Sphere.obj"; break; 
        case CYLINDER:
            path += "Cylinder.obj"; break;
        case QUAD:
            path += "Quad.obj"; break;
        case RING:
            path += "Ring.obj"; break;
        case CIRCLE:
            path += "Circle.obj"; break;
        case TORUSKNOT:
            path += "TorusKnot.obj"; break;
        }
        Init(path.c_str(), mtl, bAssimp);
    }
    virtual void Init(const char* modelPath, bool mtl = false, bool bAssimp = true) {
        mbAssimp = bAssimp;
        if (bAssimp)
            LoadByAssimp(modelPath, mtl);
        else
            LoadByMyself(modelPath);
    }
    virtual void LoadByAssimp(const char* modelPath, bool mtl = false) {
        mbMTL = mtl;
        LoadModel(modelPath);
        mShader = nullptr;
    }
    virtual void LoadByMyself(const char* modelPath) {
        struct FloatData {
            float v[3];
        };
        struct VertexDefine {
            int posIndex;
            int texcoordIndex;
            int normalIndex;
        };
        int nFileSize = 0;
        unsigned char* fileContent = LoadFileContent(modelPath, nFileSize);
        if (fileContent == nullptr) {
            return;
        }
        std::vector<FloatData> positions, texcoords, normals;
        std::vector<VertexDefine> vertexes;
        std::stringstream ssFileContent((char*)fileContent);
        std::string temp;
        char szOneLine[256];
        while (!ssFileContent.eof()) {
            memset(szOneLine, 0, 256);
            ssFileContent.getline(szOneLine, 256);
            if (strlen(szOneLine) > 0) {
                if (szOneLine[0] == 'v') {
                    std::stringstream ssOneLine(szOneLine);
                    if (szOneLine[1] == 't') {
                        ssOneLine >> temp;
                        FloatData floatData;
                        ssOneLine >> floatData.v[0];
                        ssOneLine >> floatData.v[1];
                        texcoords.push_back(floatData);
                    }
                    else if (szOneLine[1] == 'n') {
                        ssOneLine >> temp;
                        FloatData floatData;
                        ssOneLine >> floatData.v[0];
                        ssOneLine >> floatData.v[1];
                        ssOneLine >> floatData.v[2];
                        normals.push_back(floatData);
                    }
                    else {
                        ssOneLine >> temp;
                        FloatData floatData;
                        ssOneLine >> floatData.v[0];
                        ssOneLine >> floatData.v[1];
                        ssOneLine >> floatData.v[2];
                        positions.push_back(floatData);
                    }
                }
                else if (szOneLine[0] == 'f') {
                    std::stringstream ssOneLine(szOneLine);
                    ssOneLine >> temp;
                    std::string vertexStr;
                    for (int i = 0; i < 3; i++) {
                        ssOneLine >> vertexStr;
                        size_t pos = vertexStr.find_first_of('/');
                        std::string posIndexStr = vertexStr.substr(0, pos);
                        size_t pos2 = vertexStr.find_first_of('/', pos + 1);
                        std::string texcoordIndexStr = vertexStr.substr(pos + 1, pos2 - 1 - pos);
                        std::string normalIndexStr = vertexStr.substr(pos2 + 1, vertexStr.length() - 1 - pos2);
                        VertexDefine vd;
                        vd.posIndex = atoi(posIndexStr.c_str());
                        vd.texcoordIndex = atoi(texcoordIndexStr.c_str());
                        vd.normalIndex = atoi(normalIndexStr.c_str());
                        vertexes.push_back(vd);
                    }
                }
            }
        }
        delete fileContent;
        int vertexCount = (int)vertexes.size();
        mVertexBuffer = new VertexBuffer;
        mVertexBuffer->Init(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            float* temp = positions[vertexes[i].posIndex - 1].v;
            mVertexBuffer->SetPosition(i, temp[0], temp[1], temp[2]);
            temp = texcoords[vertexes[i].texcoordIndex - 1].v;
            mVertexBuffer->SetTexcoord(i, temp[0], temp[1]);
            temp = normals[vertexes[i].normalIndex - 1].v;
            mVertexBuffer->SetNormal(i, temp[0], temp[1], temp[2]);
        }
        mShader = nullptr;
    }
    virtual void SetShader(const char* vs, const char* fs, const char* gs = nullptr, const char* tcs = nullptr, const char* tes = nullptr, bool bDebug = false) {
        mShader = new Shader;
        mShader->Init(vs, fs, gs, tcs, tes, bDebug);
    }
    virtual void Render(glm::mat4& viewMatrix, glm::mat4& projMatrix, glm::vec3 cameraPos) {
        if (mbAssimp)
            RenderWithEBO(viewMatrix, projMatrix, cameraPos);
        else
            RenderWithoutEBO(viewMatrix, projMatrix, cameraPos);
    }
    virtual void RenderWithoutEBO(glm::mat4& viewMatrix, glm::mat4& projMatrix, glm::vec3 cameraPos) {
        if (mShader == nullptr) {
            std::cout << "[ERROR] No set shader..." << std::endl;
            return;
        }

        glEnable(GL_DEPTH_TEST);

        mVertexBuffer->Bind();


        glm::mat4 it = glm::inverseTranspose(mModelMatrix);

        mShader->Bind(mModelMatrix, viewMatrix, projMatrix);
        mShader->SetMat4(IT_MODEL.c_str(), it);
        SetCommonTexture();

        int itLocation = glGetUniformLocation(mShader->mProgram, LIGHTVIEWMATRIX.c_str());
        if (itLocation >= 0 && mLightViewMatrix != nullptr) {
            glUniformMatrix4fv(itLocation, 1, GL_FALSE, mLightViewMatrix);
        }

        itLocation = glGetUniformLocation(mShader->mProgram, LIGHTPROJMATRIX.c_str());
        if (itLocation >= 0 && mLightProjMatrix != nullptr) {
            glUniformMatrix4fv(itLocation, 1, GL_FALSE, mLightProjMatrix);
        }

        glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer->mVertexCount);

        mVertexBuffer->Unbind();
        mShader->Unbind();
    }
    virtual void RenderWithEBO(glm::mat4& viewMatrix, glm::mat4& projMatrix, glm::vec3 cameraPos) {
        for (int i = 0; i < mMeshes.size(); i++) {
			if (mShader == nullptr) {
				std::cout << "[ERROR] No set shader..." << std::endl;
				return;
			}

			glEnable(GL_DEPTH_TEST);
            mMeshes[i].NewBind();
			
			glm::mat4 it = glm::inverseTranspose(mModelMatrix);

			mShader->Bind(mModelMatrix, viewMatrix, projMatrix);
			mShader->SetMat4(IT_MODEL.c_str(), it);
            SetCommonTexture();

			int itLocation = glGetUniformLocation(mShader->mProgram, LIGHTVIEWMATRIX.c_str());
			if (itLocation >= 0 && mLightViewMatrix != nullptr) {
				glUniformMatrix4fv(itLocation, 1, GL_FALSE, mLightViewMatrix);
			}

			itLocation = glGetUniformLocation(mShader->mProgram, LIGHTPROJMATRIX.c_str());
			if (itLocation >= 0 && mLightProjMatrix != nullptr) {
				glUniformMatrix4fv(itLocation, 1, GL_FALSE, mLightProjMatrix);
			}

            if (mbMTL) {
				for (unsigned int j = 0; i < mMeshes[i].mTextures.size(); j++) {
					std::string number;
					std::string name = mMeshes[i].mTextures[j].type;
					mShader->SetTexture(name.c_str(), mMeshes[i].mTextures[j].id);
				}
            }
            
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mMeshes[i].mIndices.size()), GL_UNSIGNED_INT, 0);

			mVertexBuffer->Unbind();
			mShader->Unbind();
        }
    }
    virtual void SetPosition(float x, float y, float z) {
        if (mbModel) {
            glm::mat4 tmp = glm::mat4(1.0f);
            tmp = glm::translate(tmp, glm::vec3(x, y, z));
            mModelChangeVecs.push_back(tmp);
        }
        else {
            std::cout << "[ERROR] Lack of key begin function..." << std::endl;
        }
    }
    virtual void SetPosition(glm::vec3 pos) {
        SetPosition(pos.x, pos.y, pos.z);
    }
    virtual void SetScale(float x, float y, float z) {
        if (mbModel) {
            glm::mat4 tmp = glm::mat4(1.0f);
            tmp = glm::scale(tmp, glm::vec3(x, y, z));
            mModelChangeVecs.push_back(tmp);
        }
        else {
            std::cout << "[ERROR] Lack of key begin function..." << std::endl;
        }
    }
    virtual void SetScale(float x) {
        SetScale(x, x, x);
    }
    virtual void SetRotate(float angle, float x, float y, float z) {
        if (mbModel) {
            glm::mat4 tmp = glm::mat4(1.0f);
            tmp = glm::rotate(tmp, glm::radians(angle), glm::vec3(x, y, z));
            mModelChangeVecs.push_back(tmp);
        }
        else {
            std::cout << "[ERROR] Lack of key begin function..." << std::endl;
        }

    };
    virtual void SetTexture(const char* sampler2DName, const char* image) {
        mbTexture = true;
        mShader->SetTexture(sampler2DName, image);
    }
    virtual void SetTexture(const char* sampler2DName, unsigned int image) {
        mbTexture = true;
        mShader->SetTexture(sampler2DName, image);
    }
    virtual void SetTextureCube(const char* sampleCubeName, const char* image) {
        mbTexture = true;
        mShader->SetTextureCube(sampleCubeName, image);
    }
    virtual void EnableShadow(bool bshadow) {
        if (!bshadow) {
            mLightProjMatrix = nullptr;
            mLightViewMatrix = nullptr;
        }
    }
    virtual void BeginChangeModelMatrix() {
        mbModel = true;
    }
    virtual void EndChangeModelMatrix() {
        if (mbModel) {
            for (int i = 0; i < mModelChangeVecs.size(); i++) {
                mModelMatrix = mModelChangeVecs[i] * mModelMatrix;
            }
            mbModel = false;
        }
        else {
            std::cout << "[ERROR] Lack of key begin function..." << std::endl;
        }
    }
    // Set Material
    virtual void SetAmbientMat(float r, float g, float b, float a = 1.0f) {
        mShader->Use();
        mShader->SetVec4((MAT + AMBI).c_str(), r, g, b, a);
    }
    virtual void SetAmbientMat(glm::vec3 color) {
        SetAmbientMat(color.r, color.g, color.b, 1.0f);
    }
	virtual void SetAmbientMat(glm::vec4 color) {
		SetAmbientMat(color.r, color.g, color.b, color.a);
	}
    virtual void SetDiffuseMat(float r, float g, float b, float a = 1.0f) {
		mShader->Use();
		mShader->SetVec4((MAT + DIFF).c_str(), r, g, b, a);
    }
	virtual void SetDiffuseMat(glm::vec3 color) {
        SetDiffuseMat(color.r, color.g, color.b, 1.0f);
	}
	virtual void SetDiffuseMat(glm::vec4 color) {
        SetDiffuseMat(color.r, color.g, color.b, color.a);
	}
    virtual void SetSpecularMat(float r, float g, float b, float a = 1.0f) {
		mShader->Use();
		mShader->SetVec4((MAT + SPEC).c_str(), r, g, b, a);
    }
	virtual void SetSpecularMat(glm::vec3 color) {
        SetSpecularMat(color.r, color.g, color.b, 1.0f);
	}
	virtual void SetSpecularMat(glm::vec4 color) {
        SetSpecularMat(color.r, color.g, color.b, color.a);
	}
    virtual void SetShininessMat(float r) {
        mShader->Use();
        mShader->SetFloat((MAT + SHIN).c_str(), r);
    }
    virtual void SetCommonTexture() {
        mShader->Use();
        mShader->SetMat4(WHICHLIGHT.c_str(), mbLight);
        mShader->SetMat4(WHICHSHADOW.c_str(), mbShadow);
        mShader->SetBool(BTEX.c_str(), mbTexture);
        SetAmbientMat(0.1f, 0.1f, 0.1f, 1.0f);
        SetDiffuseMat(1.0f, 1.0f, 1.0f, 1.0f);
        SetSpecularMat(0.23f, 0.23f, 0.23f, 1.0f);
        SetShininessMat(128.0f);
    }
    // Set Light
	virtual void LightInfo()  {
		std::cout << "************************************************************************************************" << std::endl;
		std::cout << "** Information about the Light" << std::endl;
		std::cout << "** The first column of elements represents the type of light(Theory, Area, Volume, IBL)" << std::endl;
		std::cout << "** The remaining three column represent whether the corresponding lighting is enable" << std::endl;
        display_4x4(mbLight);
        if (mbShadow != glm::mat4(0.0f))
            display_4x4(mbShadow);
        std::cout << "************************************************************************************************" << std::endl;
	}
    void PrintLightInfo() {
        //std::call_once(flag_lightInfo, [&]() {
            //LightInfo(); });
        LightInfo();
    }
    virtual void SetDirLight(TheoryLight* light, bool bShadow = false) {
        mShader->Use();
        mShader->SetVec4((DLIGHT + AMBI).c_str(), light->mLightAmbient);
        mShader->SetVec4((DLIGHT + COLO).c_str(), light->mLightClr);
        mShader->SetVec4((DLIGHT + DIRC).c_str(), light->mDirection);
        mbLight[0][0] = 1.0f;
        mbLight[0][1] = 1.0f;
        if (bShadow)
            mbShadow[0][1] = 1.0f;
    }
    virtual void SetPointLight(TheoryLight* light, bool bShadow = false) {
		mShader->Use();
		mShader->SetVec4((PLIGHT + AMBI).c_str(), light->mLightAmbient);
		mShader->SetVec4((PLIGHT + COLO).c_str(), light->mLightClr);
		mShader->SetVec4((PLIGHT + POSI).c_str(), light->mPosition);
		mbLight[0][0] = 1.0f;
		mbLight[0][2] = 1.0f;
		if (bShadow)
			mbShadow[0][2] = 1.0f;
    }
    virtual void SetSpotLight(TheoryLight* light, bool bShadow = false) {
		mShader->Use();
		mShader->SetVec4((FLIGHT + AMBI).c_str(), light->mLightAmbient);
		mShader->SetVec4((FLIGHT + COLO).c_str(), light->mLightClr);
		mShader->SetVec4((FLIGHT + POSI).c_str(), light->mPosition);
        mShader->SetVec4((FLIGHT + DIRC).c_str(), glm::vec4(light->mDirection, light->mOutter));
        mShader->SetVec4((FLIGHT + OPTI).c_str(), light->mLightOpt);
		mbLight[0][0] = 1.0f;
		mbLight[0][3] = 1.0f;
		if (bShadow)
			mbShadow[0][3] = 1.0f;
    }
    virtual void SetRectAreaLight(AreaLight* light, bool bShadow = false) {
        mShader->Use();
        RectLight* tmp = dynamic_cast<RectLight*>(light);
        mShader->SetVec4((RALIGHT + COLO).c_str(), tmp->mLightClr);
        mShader->SetFloat((RALIGHT + INTI).c_str(), tmp->mIntensity);
        for (int i = 0; i < tmp->mPoints.size(); i++) {
            mShader->SetVec4(("U_RALight.U_Points[" + std::to_string(i) + "]").c_str(), tmp->mPoints[i]);
        }
        mShader->SetFloat((MAT + ROUG).c_str(), 0.25f);
        mbLight[1][0] = 1.0f;
        mbLight[1][1] = 1.0f;
        if (bShadow)
            mbShadow[1][1] = 1.0f;
    }
    virtual void SetCylinderAreaLight(AreaLight* light, bool bShadow = false) {
        mShader->Use();
        CylinderLight* tmp = dynamic_cast<CylinderLight*>(light);
        mShader->SetVec4((TALIGHT + COLO).c_str(), tmp->mLightClr);
        mShader->SetFloat((TALIGHT + INTI).c_str(), tmp->mIntensity);
        for (int i = 0; i < tmp->mPoints.size(); i++) {
            mShader->SetVec4(("U_TALight.U_Points[" + std::to_string(i) + "]").c_str(), tmp->mPoints[i]);
        }
        mShader->SetFloat("U_TALight.U_Radius", tmp->mRadius);
        mShader->SetFloat((MAT + ROUG).c_str(), 0.25f);
        mbLight[1][0] = 1.0f;
        mbLight[1][2] = 1.0f;
        if (bShadow)
            mbShadow[1][2] = 1.0f;
    }
    virtual void SetSphereAreaLight(AreaLight* light, bool bShadow = false) {
        mShader->Use();
        SphereLight* tmp = dynamic_cast<SphereLight*>(light);
        mShader->SetVec4((SALIGHT + COLO).c_str(), tmp->mLightClr);
        mShader->SetFloat((SALIGHT + INTI).c_str(), tmp->mIntensity);
        for (int i = 0; i < tmp->mPoints.size(); i++) {
            mShader->SetVec4(("U_SALight.U_Points[" + std::to_string(i) + "]").c_str(), tmp->mPoints[i].x, tmp->mPoints[i].y, tmp->mPoints[i].z, 1.0f);
        }
        mShader->SetFloat((MAT + ROUG).c_str(), 0.25f);
        mbLight[1][0] = 1.0f;
        mbLight[1][3] = 1.0f;
        if (bShadow)
            mbShadow[1][3] = 1.0f;
    }
    
    
    virtual void CloseLight(LightType type) {
        switch (type)
        {
        case LIGHT_DIRECTION:
            mbLight[0][1] = 0.0f;
            break;
        case LIGHT_POINT:
            mbLight[0][2] = 0.0f;
            break;
        case LIGHT_SPOT:
            mbLight[0][3] = 0.0f;
            break;
        case LIGHT_SPHERE:
            mbLight[1][3] = 0.0f;
            break;
        case LIGHT_RECT:
            mbLight[1][1] = 0.0f;
            break;
        case LIGHT_DISK:
            break;
        case LIGHT_CYLINDER:
            mbLight[1][2] = 0.0f;
            break;
        default:
            break;
        }
        if (mbLight[0][1] == 0.0f
            && mbLight[0][2] == 0.0f && mbLight[0][3] == 0.0f) {
            mbLight[0][0] = 0.0f;
        }
        if (mbLight[1][1] == 0.0f
            && mbLight[1][2] == 0.0f && mbLight[1][3] == 0.0f) {
            mbLight[1][0] = 0.0f;
        }
    }
private:
    virtual void LoadModel(const char* modelPath) {
         // read file via ASSIMP
         Assimp::Importer importer;

         int nFileSize = 0;
         unsigned char* fileConent = LoadFileContent(modelPath, nFileSize);
         if (fileConent == nullptr)
             return;

         const aiScene* scene = importer.ReadFileFromMemory(fileConent, nFileSize, \
             aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

         // check for errors
         if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
             std::cout << "[ERROR] ASSIMP:: " << importer.GetErrorString() << std::endl;
         }

         // retrieve the directory path of the filepath
         std::string str = std::string(modelPath);
         mDirectory = str.substr(0, str.find_last_of('/'));

         // process ASSIMP's root node recursively
         ProcessNode(scene->mRootNode, scene);
         
     }
	virtual void ProcessNode(aiNode* node, const aiScene* scene) {

		// process each mesh located at the current node
		for (int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			mMeshes.push_back(ProcessMesh(mesh, scene));
		}
		// after processed all of the meshes, recursively process each of the childeren nodes
		for (int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene);
		}
	}
	virtual VertexBuffer ProcessMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		// mesh's vertices
		for (int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			// position
			vertex.Position[0] = mesh->mVertices[i].x;
			vertex.Position[1] = mesh->mVertices[i].y;
			vertex.Position[2] = mesh->mVertices[i].z;
			vertex.Position[3] = 1.0f;
			// normal
			if (mesh->HasNormals()) {
				vertex.Normal[0] = mesh->mNormals[i].x;
				vertex.Normal[1] = mesh->mNormals[i].y;
				vertex.Normal[2] = mesh->mNormals[i].z;
				vertex.Normal[3] = 1.0f;
			}
			// texture
			if (mesh->mTextureCoords[0]) {
				// UV
				vertex.Texcoord[0] = mesh->mTextureCoords[0][i].x;
				vertex.Texcoord[1] = mesh->mTextureCoords[0][i].y;
				vertex.Texcoord[2] = 0.0f;
				vertex.Texcoord[3] = 0.0f;
				// tangent
				vertex.Tangent[0] = mesh->mTangents[i].x;
				vertex.Tangent[1] = mesh->mTangents[i].y;
				vertex.Tangent[2] = mesh->mTangents[i].z;
				vertex.Tangent[3] = 1.0f;
				// bitangent
				vertex.Bitangent[0] = mesh->mBitangents[i].x;
				vertex.Bitangent[1] = mesh->mBitangents[i].y;
				vertex.Bitangent[2] = mesh->mBitangents[i].z;
				vertex.Bitangent[3] = 1.0f;
			}
			else {
				// UV
				vertex.Texcoord[0] = 0.0f;
				vertex.Texcoord[1] = 0.0f;
				vertex.Texcoord[2] = 0.0f;
				vertex.Texcoord[3] = 0.0f;
				// tangent
				vertex.Tangent[0] = 0.0f;
				vertex.Tangent[1] = 0.0f;
				vertex.Tangent[2] = 0.0f;
				vertex.Tangent[3] = 1.0f;
				// bitangent
				vertex.Bitangent[0] = 0.0f;
				vertex.Bitangent[1] = 0.0f;
				vertex.Bitangent[2] = 0.0f;
				vertex.Bitangent[3] = 1.0f;
			}

			vertices.push_back(vertex);
		}

		// mesh's faces
		for (int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices 
			for (int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// process material
		if (mbMTL) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> ambiMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, (MAT + AMBI));
			textures.insert(textures.end(), ambiMaps.begin(), ambiMaps.end());
			std::vector<Texture> diffMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, (MAT + DIFF));
			textures.insert(textures.end(), diffMaps.begin(), diffMaps.end());
			std::vector<Texture> specMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, (MAT + SPEC));
			textures.insert(textures.end(), specMaps.begin(), specMaps.end());
			std::vector<Texture> normMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, (MAT + NORM));
			textures.insert(textures.end(), normMaps.begin(), normMaps.end());
			std::vector<Texture> heigMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, (MAT + HEIG));
			textures.insert(textures.end(), heigMaps.begin(), heigMaps.end());
		}
		return VertexBuffer(vertices, indices, textures);
	}
	virtual std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
         std::vector<Texture> textures;
         for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
             aiString str;
             mat->GetTexture(type, i, &str);
             bool skip = false;
             for (int j = 0; j < mTextureLoaded.size(); j++) {
                 if (std::strcmp(mTextureLoaded[j].path.data(), str.C_Str()) == 0) {
                     textures.push_back(mTextureLoaded[j]);
                     skip = true;
                     break;
                 }
             }
             if (!skip) {
                 Texture texture;
                 std::string path = this->mDirectory + '/';
                 path = path + str.C_Str();
                 texture.id = CreateTexture2DFromIMG(path.c_str());
                 texture.path = path;
                 texture.type = typeName;
                 textures.push_back(texture);
                 mTextureLoaded.push_back(texture);
             }
         }
         return textures;
     }
};