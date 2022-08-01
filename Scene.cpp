#include "Scene.h"
#include "Utils.h"
#include "Shader.h"
#include "Camera.h"
#include "FrameBufferObject.h"
#include "Quad.h"
#include "Model.h"
#include "Skybox.h"
#include "Ground.h"
#include "Light.h"
#include "LTC.h"


Model reflectSphere, refractSphere;
Model cubeObj, directLightObj, pointLightObj, spotLightObj;
Model sphereObj, sunObj;
Model rectLightObj, cylinderLightObj, sphereLightObj;
Skybox skybox;
TheoryLight* pointLight;
TheoryLight* spotLight;
TheoryLight* directLight;
unsigned int LTC1TexMap, LTC2TexMap;
AreaLight* rectLight;
AreaLight* cylinderLight;
AreaLight* sphereLight;


float lastX = 1280.0f / 2.0f;
float lastY = 720.0f / 2.0;

glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, 15.0f);
glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp  = glm::vec3(0.0f, 1.0f, 0.0f);
Camera camera;

FrameBufferObject* gBuffer = nullptr;
FrameBufferObject* fBuffer = nullptr;
FrameBufferObject* depthFBO = nullptr;
FrameBufferObject* occFBO = nullptr;
glm::mat4 lightProj = glm::mat4(1.0f);
glm::mat4 lightView = glm::mat4(1.0f);
Shader* geometryPassShader = nullptr;
Shader* occlustionShader = nullptr;
std::map<std::string, Shader*> Name2Shader;
Quad* fsq = nullptr;
Quad* lscFsq = nullptr; // light scattering texture map
std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
std::default_random_engine generator;
std::vector<glm::vec3> ssaoKernel;
std::vector<glm::vec3> ssaoNoise;

#pragma region oldscene
void ReflectAndRefract_Init() {
	// reflect
	reflectSphere.Init(SPHERE);
	reflectSphere.BeginChangeModelMatrix();
	reflectSphere.SetPosition(-6.0f, 2.0f, 0.0f);
	reflectSphere.SetScale(2.0f, 2.0f, 2.0f);
	reflectSphere.EndChangeModelMatrix();
	reflectSphere.SetShader("Res/reflection.vs", "Res/reflection.fs");
	reflectSphere.SetTextureCube(TEX.c_str(), "Res/skybox1/");

	// refract
	refractSphere.Init(SPHERE);
	refractSphere.BeginChangeModelMatrix();
	refractSphere.SetPosition(6.0f, 2.0f, 0.0f);
	refractSphere.SetScale(2.0f, 2.0f, 2.0f);
	refractSphere.EndChangeModelMatrix();
	refractSphere.SetShader("Res/refraction.vs", "Res/refraction.fs");
	refractSphere.SetTextureCube(TEX.c_str(), "Res/skybox1/");
}
void ReflectAndRefract_Render() {
	reflectSphere.EnableShadow(false);
	reflectSphere.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	refractSphere.EnableShadow(false);
	refractSphere.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void DirectLight_Init(UI* ui) {
	directLight = new TheoryLight(LIGHT_DIRECTION, glm::vec3(0.0f, 0.0f, 0.0f), IMVEC42VEC4(ui->mDirColor), glm::vec3(-1.0f, 1.0f, 0.0f));

	directLightObj.Init(SPHERE);
	directLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	directLightObj.BeginChangeModelMatrix();
	directLightObj.SetScale(0.5f);
	directLightObj.SetPosition(directLight->mPosition);
	directLightObj.EndChangeModelMatrix();

}
void DirectLight_Render(UI* ui) {
	directLight->mLightClr = IMVEC42VEC4(ui->mDirColor);
	directLightObj.mShader->SetVec4("U_lightColor", glm::vec4(directLight->mLightClr, 1.0f));
	directLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void PointLight_Init(UI* ui) {
	pointLight = new TheoryLight(LIGHT_POINT, glm::vec3(8.0f, 0.0f, 8.0f), IMVEC42VEC4(ui->mPointColor));

	pointLightObj.Init(SPHERE);
	pointLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	pointLightObj.BeginChangeModelMatrix();
	pointLightObj.SetScale(0.5f);
	pointLightObj.SetPosition(pointLight->mPosition);
	pointLightObj.EndChangeModelMatrix();

}
void PointLight_Render(UI* ui) {
	pointLight->mLightClr = IMVEC42VEC4(ui->mPointColor);
	pointLightObj.mShader->SetVec4("U_lightColor", glm::vec4(pointLight->mLightClr, 1.0f));
	pointLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void SpotLight_Init(UI* ui) {
	spotLight = new TheoryLight(LIGHT_SPOT, glm::vec3(8.0f, 2.5f, 3.0f), 
		IMVEC42VEC4(ui->mSpotColor) , glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(32.0f, 2.0f, 0.0f));

	spotLightObj.Init(SPHERE);
	spotLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	spotLightObj.BeginChangeModelMatrix();
	spotLightObj.SetScale(0.5f);
	spotLightObj.SetPosition(spotLight->mPosition);
	spotLightObj.EndChangeModelMatrix();
}
void SpotLight_Render(UI* ui) {
	spotLight->mLightClr = IMVEC42VEC4(ui->mSpotColor);
	spotLightObj.mShader->SetVec4("U_lightColor", glm::vec4(spotLight->mLightClr, 1.0f));
	spotLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void InitAreaLight() {
	LTC1TexMap = CreateTexture2DFromLTC(LTC1);
	LTC2TexMap = CreateTexture2DFromLTC(LTC2);
}

void RectLight_Init(UI* ui) {
	rectLight = new RectLight(LIGHT_RECT, glm::vec3(4.0f, 2.0f, -6.0f), glm::vec3(0.0f, 0.34f, 0.57f),
		4.0f, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		2.5f, 1.5f);

	rectLightObj.Init(QUAD);
	rectLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	rectLightObj.BeginChangeModelMatrix();
	rectLightObj.SetScale(5.0f, 3.0f, 1.0f);
	rectLightObj.SetPosition(rectLight->mCenter);
	rectLightObj.EndChangeModelMatrix();
}
void RectLight_Render(UI* ui) {
	rectLightObj.mShader->SetVec4("U_lightColor", glm::vec4(rectLight->mLightClr, 1.0f));
	rectLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}


void CylinderLight_Init(UI* ui) {
	cylinderLight = new CylinderLight(LIGHT_CYLINDER, glm::vec3(0.0f, 2.0f, 6.0f), glm::vec3(1.0f), 4.0f,
		glm::vec3(1.0f, 0.0f, 0.0f), 15.0f, 0.6f);

	cylinderLightObj.Init(CYLINDER);
	
	cylinderLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	cylinderLightObj.BeginChangeModelMatrix();
	cylinderLightObj.SetRotate(90.0f, 0.0f, 0.0f, 1.0f);
	float tmpLength = dynamic_cast<CylinderLight*>(cylinderLight)->mLength;
	cylinderLightObj.SetScale(tmpLength, 0.5f, 0.5f);
	cylinderLightObj.SetPosition(cylinderLight->mCenter);
	cylinderLightObj.EndChangeModelMatrix();
}
void CylinderLight_Render(UI* ui) {
	cylinderLightObj.mShader->SetVec4("U_lightColor", glm::vec4(cylinderLight->mLightClr, 1.0f));
	cylinderLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void SphereLight_Init(UI* ui) {
	sphereLight = new SphereLight(LIGHT_SPHERE, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.2f, 0.4f, 0.3f), 20.0f,
		glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		1.0f, 1.0f, 1.0f);

	sphereLightObj.Init(SPHERE);

	sphereLightObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	sphereLightObj.BeginChangeModelMatrix();
	sphereLightObj.SetScale(1.0f);
	sphereLightObj.SetPosition(sphereLight->mCenter);
	sphereLightObj.EndChangeModelMatrix();
}
void SphereLight_Render(UI* ui) {
	sphereLightObj.mShader->SetVec4("U_lightColor", glm::vec4(sphereLight->mLightClr, 1.0f));
	sphereLightObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}



#pragma endregion oldscene

void Sphere_Init(UI* ui) {
	sphereObj.Init(TORUSKNOT);
	sphereObj.SetShader("Res/obj_vs.glsl", "Res/obj_fs.glsl");
	sphereObj.BeginChangeModelMatrix();
	sphereObj.SetScale(1.0f);
	sphereObj.SetPosition(0.0f, 5.0f, 0.0f);
	sphereObj.EndChangeModelMatrix();
}
void Sphere_Render(UI* ui, bool bGbuffer = false) {

	if (bGbuffer) {
		Name2Shader.insert(std::pair("sphere", sphereObj.mShader));
		sphereObj.mShader = geometryPassShader;
	}
	else {
		std::map<std::string, Shader*>::iterator iter = Name2Shader.find("tor");
		if (iter != Name2Shader.end()) {
			sphereObj.mShader = Name2Shader["tor"];
			sphereObj.SetTexture("gPosition", gBuffer->GetBuffer("position")); 
			sphereObj.SetTexture("gNormal", gBuffer->GetBuffer("normal"));
			sphereObj.SetTexture("gTex", gBuffer->GetBuffer("tex"));
		}
		else
			std::cout << "Can not find sphere" << std::endl;
	}


	sphereObj.EnableShadow(false);
	sphereObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void Sun_Init(UI* ui) {

	directLight = new TheoryLight(LIGHT_DIRECTION, glm::vec3(0.0f, 10.0f, 0.0f), IMVEC42VEC4(ui->mDirColor), glm::vec3(-1.0f, -1.0f, 0.0f));
	spotLight = new TheoryLight(LIGHT_SPOT, glm::vec3(0.0f, 10.0f, 0.0f),
		IMVEC42VEC4(ui->mSpotColor), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(16.0f, 2.0f, 0.0f));
	sunObj.Init(SPHERE);
	sunObj.SetShader("Res/illuminator.vs", "Res/illuminator.fs");
	sunObj.BeginChangeModelMatrix();
	sunObj.SetScale(1.0f);
	sunObj.SetPosition(directLight->mPosition);
	sunObj.EndChangeModelMatrix();

	

}
void Sun_Render(UI* ui) {
	
	directLight->mLightClr = IMVEC42VEC4(ui->mDirColor);
	spotLight->mLightClr = IMVEC42VEC4(ui->mSpotColor);
	if (ui->mbDlight) {
		sunObj.mShader->SetVec4("U_lightColor", glm::vec4(directLight->mLightClr, 1.0f));
	}
	else 
		sunObj.mShader->SetVec4("U_lightColor", glm::vec4(spotLight->mLightClr, 1.0f));
	sunObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());

}

void Cube_Init(UI* ui) {
	cubeObj.Init(CUBE);
	cubeObj.SetShader("Res/light_vs.glsl", "Res/light_fs.glsl");
	cubeObj.BeginChangeModelMatrix();
	cubeObj.SetPosition(0.0f, -2.0f, 0.0f);
	cubeObj.SetScale(18.0f, 0.5f, 18.0f);
	cubeObj.EndChangeModelMatrix();
}
void Cube_Render(UI* ui) {
#pragma region theorylight

	cubeObj.EnableShadow(true);
	cubeObj.mLightProjMatrix = glm::value_ptr(lightProj);
	cubeObj.mLightViewMatrix = glm::value_ptr(lightView);
	cubeObj.SetTexture("U_ShadowMap", depthFBO->GetBuffer("depth"));
	cubeObj.mbTexture = false; // set texture will automaticall set btexture to true
	//cubeObj.SetTexture("U_Texture", "Res/wall.jpg");
	if (ui->mbDlight)
		cubeObj.SetDirLight(directLight);
	else
		cubeObj.CloseLight(LIGHT_DIRECTION);
	if (ui->mbPlight)
		cubeObj.SetPointLight(pointLight);
	else
		cubeObj.CloseLight(LIGHT_POINT);
	if (ui->mbSlight)
		cubeObj.SetSpotLight(spotLight, true);
	else
		cubeObj.CloseLight(LIGHT_SPOT);
#pragma endregion theorylight

#pragma region arealight
	/*cubeObj.SetTexture(LTC1MAP.c_str(), LTC1TexMap);
	cubeObj.SetTexture(LTC2MAP.c_str(), LTC2TexMap);
	if (ui->mbRectAreaLight)
		//cubeObj.SetRectAreaLight(rectLight);
	else
		cubeObj.CloseLight(LIGHT_RECT);
	if (ui->mbCylinderAreaLight)
		cubeObj.SetCylinderAreaLight(cylinderLight);
	else
		cubeObj.CloseLight(LIGHT_CYLINDER);
	if (ui->mbSphereAreaLight)
		cubeObj.SetSphereAreaLight(sphereLight);
	else
		cubeObj.CloseLight(LIGHT_SPHERE);*/
#pragma endregion arealight

	if (ui->mbLightInfo) {
		cubeObj.PrintLightInfo();
		ui->mbLightInfo = false;
	}


	cubeObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
}

void InitGBuffer() {
	gBuffer = new FrameBufferObject;
	gBuffer->AttachColorBuffer("position", GL_COLOR_ATTACHMENT0, 1280, 720, GL_NEAREST, true);
	gBuffer->AttachColorBuffer("normal", GL_COLOR_ATTACHMENT1, 1280, 720, GL_NEAREST, true);
	gBuffer->AttachColorBuffer("tex", GL_COLOR_ATTACHMENT2, 1280, 720, GL_NEAREST, true);
	gBuffer->AttachDepthBuffer("depth", 1280, 720);
	gBuffer->Finish();

	fBuffer = new FrameBufferObject;
	fBuffer->AttachColorBuffer("color", GL_COLOR_ATTACHMENT0, 1280, 720);
	fBuffer->AttachDepthBuffer("depth", 1280, 720);
	fBuffer->Finish();

	occFBO = new FrameBufferObject;
	occFBO->AttachColorBuffer("occTexture", GL_COLOR_ATTACHMENT0, 1280, 720);
	occFBO->AttachDepthBuffer("depth", 1280, 720);
	occFBO->Finish();

	depthFBO = new FrameBufferObject;
	depthFBO->AttachColorBuffer("color", GL_COLOR_ATTACHMENT0, 1280, 720);
	depthFBO->AttachDepthBuffer("depth", 1280, 720);
	depthFBO->Finish();

	geometryPassShader = new Shader;
	geometryPassShader->Init("Res/gbuffer_vs.glsl", "Res/gbuffer_fs.glsl");

	occlustionShader = new Shader;
	occlustionShader->Init("Res/gbuffer_vs.glsl", "Res/occlusion_fs.glsl");

	fsq = new Quad;
	fsq->Init();
	fsq->mShader->Init("Res/quad_vs.glsl", "Res/quad_fs.glsl");

	lscFsq = new Quad;
	lscFsq->Init();
	lscFsq->mShader->Init("Res/post_pass_vs.glsl", "Res/post_pass_fs.glsl");
	
}
void InitSampleKernel() {
	for (int i = 0; i < 64; i++) {
		glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}
}
unsigned int InitNoiseTexture() {
	for (int i = 0; i < 16; i++) {
		glm::vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f);
		ssaoNoise.push_back(noise);
	}
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	return noiseTexture;
}

bool CheckShader(std::string str) {
	std::map<std::string, Shader*>::iterator iter = Name2Shader.find(str);
	if (iter != Name2Shader.end())
		return true;
	else
		return false;
}

void Scene::Init(UI* ui) {

	myUI = ui;
	camera.Init(cameraPos, targetPos, cameraUp);
	camera.SetMoveSpeed(20.0f);

	ReflectAndRefract_Init();
	Cube_Init(myUI);
	Sphere_Init(myUI);
	Sun_Init(myUI);
	//// Theory Light
	//DirectLight_Init(myUI);
	//PointLight_Init(myUI);
	//SpotLight_Init(myUI);

	//// Area Light
	//InitAreaLight();
	////RectLight_Init(myUI);
	//CylinderLight_Init(myUI);
	//SphereLight_Init(myUI);
	InitGBuffer();
	skybox.Init("Res/skybox1/", "bmp");

}
void Scene::Render() {
	camera.SwitchTo3D(1280.0f, 720.0f);
	camera.Update(GetFrameTime());

	lightProj = glm::perspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
	lightView = glm::lookAt(spotLight->mPosition, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	depthFBO->Bind();
	sphereObj.EnableShadow(false);
	sphereObj.Render(lightView, lightProj, spotLight->mPosition);
	depthFBO->Unbind();

	gBuffer->Bind();
	ReflectAndRefract_Render();
	Sphere_Render(myUI, true);
	Cube_Render(myUI);
	gBuffer->Unbind();

	occFBO->Bind();
	skybox.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	// draw light source
	Sun_Render(myUI);
	// draw obj source with black color as a occlusion texture
	// two sphere - reflect & refract
	if (!CheckShader("reflect")) {
		Name2Shader.insert(std::pair("reflect", reflectSphere.mShader));
		reflectSphere.mShader = occlustionShader;
	}
	else
		reflectSphere.mShader = occlustionShader;
	if (!CheckShader("refract")) {
		Name2Shader.insert(std::pair("refract", refractSphere.mShader));
		refractSphere.mShader = occlustionShader;
	}
	else
		refractSphere.mShader = occlustionShader;

	// sphere
	if (!CheckShader("tor")) {
		Name2Shader.insert(std::pair("tor", sphereObj.mShader));
		sphereObj.mShader = occlustionShader;
	}
	else
		sphereObj.mShader = occlustionShader;
	// cube
	if (!CheckShader("floor")) {
		Name2Shader.insert(std::pair("floor", cubeObj.mShader));
		cubeObj.mShader = occlustionShader;
	}
	else
		cubeObj.mShader = occlustionShader;
	// no render shadow
	cubeObj.EnableShadow(false);
	sphereObj.EnableShadow(false);
	reflectSphere.EnableShadow(false);
	refractSphere.EnableShadow(false);
	// render
	reflectSphere.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	refractSphere.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	sphereObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	cubeObj.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	occFBO->Finish();

	cubeObj.mShader = Name2Shader["floor"];
	sphereObj.mShader = Name2Shader["tor"];
	reflectSphere.mShader = Name2Shader["reflect"];
	refractSphere.mShader = Name2Shader["refract"];
	
	fBuffer->Bind();
	skybox.Render(camera.GetViewMatrix(), camera.GetProjMatrix(), camera.GetPosition());
	ReflectAndRefract_Render();
	Cube_Render(myUI);
	Sphere_Render(myUI, false); 
#pragma region theory
	if (myUI->mbDlight)
		//DirectLight_Render(myUI);
		Sun_Render(myUI);
	if (myUI->mbPlight)
		PointLight_Render(myUI);
	if (myUI->mbSlight)
		Sun_Render(myUI);
#pragma endregion theory
	fBuffer->Unbind();



#pragma region area
	//if (myUI->mbRectAreaLight)
	//	//RectLight_Render(myUI);
	//if (myUI->mbCylinderAreaLight)
	//	CylinderLight_Render(myUI);
	//if (myUI->mbSphereAreaLight)
	//	SphereLight_Render(myUI);
#pragma endregion area

	camera.SwitchTo2D(1280.0f, 720.0f);
	// 遮罩贴图
	fsq->ColorBuffer();
	fsq->mShader->SetTexture("U_Texture", occFBO->GetBuffer("occTexture"));
	fsq->RenderToLeftTop();
	 
	// 光的视角下的深度贴图
	/*fsq->DepthBuffer();
	fsq->mShader->SetTexture("U_Texture", depthFBO->GetBuffer("depth"));
	fsq->RenderToLeftBottom();*/


	/*glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);*/

	

	// 光的散射贴图
	lscFsq->mShader->SetTexture("U_Texture", occFBO->GetBuffer("occTexture"));
	lscFsq->mShader->SetVec4("lightPos", glm::vec4(spotLight->mPosition, 1.0f));
	lscFsq->mShader->SetFloat("exposure", myUI->mExpousre);
	lscFsq->mShader->SetFloat("decay", myUI->mDecay);
	lscFsq->mShader->SetFloat("density", myUI->mDensity);
	lscFsq->mShader->SetFloat("weight", myUI->mWeight);
	lscFsq->mShader->SetFloat("samples", (float)myUI->mSamplers);
	lscFsq->RenderToRightTop();

	// 要叠加的
	fsq->ColorBuffer();
	fsq->mShader->SetTexture("U_Texture", fBuffer->GetBuffer("color"));
	fsq->RenderToRightBottom();

	

	
	
	

	// 摄像机视角下的物体的深度贴图
	fsq->DepthBuffer();
	fsq->mShader->SetTexture("U_Texture", gBuffer->GetBuffer("depth"));
	fsq->RenderToLeftBottom();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	

#pragma region camera interaction
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		camera.mbLeft = true;
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		camera.mbLeft = false;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		camera.mbRight = true;
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		camera.mbRight = false;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		camera.mbForward = true;
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		camera.mbForward = false;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		camera.mbBackward = true;
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		camera.mbBackward = false;
#pragma endregion camera interaction
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		spotLight->mPosition.x--;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		spotLight->mPosition.x++;
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		spotLight->mPosition.y--;
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		spotLight->mPosition.y++;
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
		spotLight->mPosition.z--;
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
		spotLight->mPosition.z++;
	glm::mat4 tmp = glm::mat4(1.0f);
	tmp = glm::translate(tmp, spotLight->mPosition);
	sunObj.mModelMatrix = tmp;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		camera.mbMouseLeft = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		camera.mbMouseLeft = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	auto& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	float angleX = xoffset / 500.0f;
	float angleY = yoffset / 500.0f;
	if (camera.mbMouseLeft) {
		camera.Yaw(glm::radians(-angleX));
		camera.Pitch(glm::radians(-angleY));
	}
	
}