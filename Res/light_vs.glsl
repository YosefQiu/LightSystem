#version 330 core
in vec4 position;
in vec4 color;
in vec4 texcoord;
in vec4 normal;

//mvp
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

//inverse and transpose model matrix
uniform mat4 IT_ModelMatrix;

//calc shadow
uniform mat4 LightViewMatrix;
uniform mat4 LightProjectionMatrix;

//world space
out vec4 V_Color;
out vec4 V_Normal;
out vec4 V_WorldPos;
out vec4 V_Texcoord;
out vec4 V_LightSpaceFragPos;


void main()
{
    V_Color = color;
    V_Normal = IT_ModelMatrix * normal;
    V_WorldPos = ModelMatrix * position;
    V_Texcoord = texcoord;
    
    V_LightSpaceFragPos = LightProjectionMatrix * LightViewMatrix * V_WorldPos;
    
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * position;
}
