#pragma once
#include "ggl.h"
#include "UI.h"

class Scene
{
public:
	void Init() {};
	void Init(UI* ui);
	void Init(std::function<void()> foo) {
		foo();
	}
	void Render();
	void Render(std::function<void()> foo) {
		foo();
	}
	UI* myUI = nullptr;
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);