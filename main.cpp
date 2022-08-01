#include "ggl.h"
#include "Window.h"
#include "stb_image/stb_image.h"
#include "Scene.h"


#define SCR_WIDTH	1280
#define SCR_HEIGHT	720
const char* SCR_WND_NAME = "OPENGL_LIGHT";

std::unique_ptr<GLWindow> wnd = nullptr;
std::unique_ptr<Scene> scene  = nullptr;
std::unique_ptr<UI>		myUI  = nullptr;
void GL_Interaction() {
	glfwSetFramebufferSizeCallback(wnd->GetWnd(), framebuffer_size_callback);
	glfwSetKeyCallback(wnd->GetWnd(), key_callback);;
	glfwSetCursorPosCallback(wnd->GetWnd(), mouse_callback);
	glfwSetMouseButtonCallback(wnd->GetWnd(), mouse_button_callback);
}


int main(int argc, char* argv[]) {
	// create and init window
	wnd.reset(new GLWindow(SCR_WIDTH, SCR_HEIGHT, SCR_WND_NAME));
	if (!wnd->Init(true)) {
		wnd->Close();
		return -1;
	}
	// Set interaction
	wnd->InitInteraction(GL_Interaction);
	
	//UI
	myUI.reset(new UI(wnd->GetWnd()));
	myUI->ImguiInit();

	scene.reset(new Scene());
	scene->Init(myUI.get());
	
	while (!wnd->IsClose()) {
		myUI->ImguiCreateWindow();
		wnd->Update(0.0f, 0.34f, 0.57f);

		scene->Render();
		myUI->ImguiRender();
		wnd->SwapBuffer();
		wnd->PollEvents();
	}
	wnd->Close();
	return 0;
}