#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <random>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif // WIN32


#include <glm/glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <stdlib.h>
#include <math.h>
#include <string.h>


#define Pi 3.1415926

// light type
const std::string DLIGHT = "U_DLight.";
const std::string PLIGHT = "U_PLight.";
const std::string FLIGHT = "U_FLight.";
const std::string SLIGHT = "U_SLight.";
const std::string TALIGHT = "U_TALight.";
const std::string RALIGHT = "U_RALight.";
const std::string SALIGHT = "U_SALight.";

const std::string AMBI = "U_Ambient";
const std::string DIFF = "U_Diffuse";
const std::string SPEC = "U_Specular";
const std::string NORM = "U_Normal";
const std::string HEIG = "U_Height";
const std::string DIRC = "U_LightDir";
const std::string POSI = "U_LightPos";
const std::string OPTI = "U_LightOpt";
const std::string SHIN = "U_Shininess";
const std::string ROUG = "U_Roughness";
const std::string INTI = "U_Intensity";
const std::string COLO = "U_LightColor";

const std::string WHICHLIGHT = "U_ChooseLight";
const std::string WHICHSHADOW = "U_ChooseLightShadow";

const std::string TEX = "U_Texture";
const std::string BTEX = "U_bTexture";
const std::string MAT = "U_Material.";

const std::string CAMPOS = "U_CameraPos";
const std::string SHADOW = "U_ShadowMap";
const std::string LTC1MAP = "U_LTC1Map";
const std::string LTC2MAP = "U_LTC2Map";

const std::string MODELMATRIX = "ModelMatrix";
const std::string VIEWMATRIX = "ViewMatrix";
const std::string PROJMATRIX = "ProjectionMatrix";
const std::string IT_MODEL = "IT_ModelMatrix";
const std::string LIGHTVIEWMATRIX = "LightViewMatrix";
const std::string LIGHTPROJMATRIX = "LightProjectionMatrix";

const std::string POSITION = "position";
const std::string COLOR = "color";
const std::string TEXCOORD = "texcoord";
const std::string NORMAL = "normal";
const std::string TANGENT = "tangent";
const std::string BITANGENT = "bitangent";

