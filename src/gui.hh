#ifndef GUI_HH
#define GUI_HH

#include <GL/glew.h>
#include "../imgui/imgui.h"
#include <memory>

class Gui
{
	ImFont *font;

	void checkShaderCompileSuccess(int shader);
	void checkProgramLinkSuccess(int program);
public:
	GLuint fontTexture;
	int shaderHandle, vertHandle, fragHandle;
	int attribLocationTex, attribLocationProjMtx;
	int attribLocationPosition, attribLocationUV, attribLocationColor;
	unsigned int vboHandle, vaoHandle, elementsHandle;

	// bool generalOptionsOpen, wave

	int mousePosX;
	int mousePosY;
	bool mousePressed[2];

	Gui();
	~Gui();

	void BeginWindow();
	void MainMenuBar();
	void Tabs();
	void CreateFontTexture(ImFont *imFont);
	void Update(int dt);
	void Draw();
};

#endif

