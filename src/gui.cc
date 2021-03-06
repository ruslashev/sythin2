#include "gui.hh"
#include "constants.hh"

static void ImGuiRenderDrawLists(ImDrawData *draw_data)
{
	Gui *gui = (Gui*)ImGui::GetIO().UserData;
	GLint last_program, last_texture;
	glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	// Setup orthographic projection matrix
	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;
	const float ortho_projection[4][4] =
	{
		{  2.0f/width, 0.0f,          0.0f,     0.0f },
		{  0.0f,       2.0f/-height,  0.0f,     0.0f },
		{  0.0f,       0.0f,         -1.0f,     0.0f },
		{ -1.0f,       1.0f,          0.0f,     1.0f },
	};
	glUseProgram(gui->shaderHandle);
	glUniform1i(gui->attribLocationTex, 0);
	glUniformMatrix4fv(gui->attribLocationProjMtx, 1, GL_FALSE,
			&ortho_projection[0][0]);
	glBindVertexArray(gui->vaoHandle);

	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = nullptr;

		glBindBuffer(GL_ARRAY_BUFFER, gui->vboHandle);
		glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)cmd_list->VtxBuffer.size()*sizeof(ImDrawVert),
				(GLvoid*)&cmd_list->VtxBuffer.front(),
				GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui->elementsHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				(GLsizeiptr)cmd_list->IdxBuffer.size()*sizeof(ImDrawIdx),
				(GLvoid*)&cmd_list->IdxBuffer.front(),
				GL_STREAM_DRAW);

			for (const ImDrawCmd *pcmd = cmd_list->CmdBuffer.begin();
					pcmd != cmd_list->CmdBuffer.end();
					pcmd++)
			{
				if (pcmd->UserCallback) {
					pcmd->UserCallback(cmd_list, pcmd);
				} else {
					glBindTexture(GL_TEXTURE_2D,
							(GLuint)(intptr_t)pcmd->TextureId);
					glScissor((int)pcmd->ClipRect.x,
							(int)(height - pcmd->ClipRect.w),
							(int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
							(int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
							GL_UNSIGNED_SHORT, idx_buffer_offset);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
	}

	// Restore modified state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(last_program);
	glDisable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

Gui::Gui()
{
	int err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		throw;
	}

	// editingBuffer = new char [16*1024];

	fontTexture = 0;
	shaderHandle = 0, vertHandle = 0, fragHandle = 0;
	attribLocationTex = 0, attribLocationProjMtx = 0;
	attribLocationPosition = 0, attribLocationUV = 0, attribLocationColor = 0;
	vboHandle = 0, vaoHandle = 0, elementsHandle = 0;

	waveOpen = true;
	settingsOpen = false;

	mousePosX = 0;
	mousePosY = 0;
	mousePressed[0] = false;
	mousePressed[1] = false;

	ImGuiIO& io = ImGui::GetIO();
	io.RenderDrawListsFn = ImGuiRenderDrawLists;
	io.DisplaySize = ImVec2(Globals.windowWidth, Globals.windowHeight);
	io.IniFilename = NULL;
	io.UserData = this;
	io.KeyRepeatDelay = 5.5;
	io.KeyRepeatRate = 1.1;

	io.KeyMap[ImGuiKey_Tab] = sf::Keyboard::Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = sf::Keyboard::Left;
	io.KeyMap[ImGuiKey_RightArrow] = sf::Keyboard::Right;
	io.KeyMap[ImGuiKey_UpArrow] = sf::Keyboard::Up;
	io.KeyMap[ImGuiKey_DownArrow] = sf::Keyboard::Down;
	io.KeyMap[ImGuiKey_Home] = sf::Keyboard::Home;
	io.KeyMap[ImGuiKey_End] = sf::Keyboard::End;
	io.KeyMap[ImGuiKey_Delete] = sf::Keyboard::Delete;
	io.KeyMap[ImGuiKey_Backspace] = sf::Keyboard::BackSpace;
	io.KeyMap[ImGuiKey_Enter] = sf::Keyboard::Return;
	io.KeyMap[ImGuiKey_Escape] = sf::Keyboard::Escape;
	io.KeyMap[ImGuiKey_Escape] = sf::Keyboard::Escape;
	io.KeyMap[ImGuiKey_A] = sf::Keyboard::A;
	io.KeyMap[ImGuiKey_C] = sf::Keyboard::C;
	io.KeyMap[ImGuiKey_V] = sf::Keyboard::V;
	io.KeyMap[ImGuiKey_X] = sf::Keyboard::X;
	io.KeyMap[ImGuiKey_Y] = sf::Keyboard::Y;
	io.KeyMap[ImGuiKey_Z] = sf::Keyboard::Z;

	const GLchar *vertex_shader =
		"#version 120\n"
		"uniform mat4 ProjMtx;\n"
		"attribute vec2 Position;\n"
		"attribute vec2 UV;\n"
		"attribute vec4 Color;\n"
		"varying vec2 Frag_UV;\n"
		"varying vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 120\n"
		"uniform sampler2D Texture;\n"
		"varying vec2 Frag_UV;\n"
		"varying vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
		"}\n";

	shaderHandle = glCreateProgram();
	vertHandle = glCreateShader(GL_VERTEX_SHADER);
	fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertHandle, 1, &vertex_shader, 0);
	glShaderSource(fragHandle, 1, &fragment_shader, 0);
	glCompileShader(vertHandle);
	glCompileShader(fragHandle);
	glAttachShader(shaderHandle, vertHandle);
	glAttachShader(shaderHandle, fragHandle);
	glLinkProgram(shaderHandle);

	checkShaderCompileSuccess(vertHandle);
	checkShaderCompileSuccess(fragHandle);
	checkProgramLinkSuccess(shaderHandle);

	attribLocationTex = glGetUniformLocation(shaderHandle, "Texture");
	attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
	attribLocationPosition = glGetAttribLocation(shaderHandle, "Position");
	attribLocationUV = glGetAttribLocation(shaderHandle, "UV");
	attribLocationColor = glGetAttribLocation(shaderHandle, "Color");

	glGenBuffers(1, &vboHandle);
	glGenBuffers(1, &elementsHandle);

	glGenVertexArrays(1, &vaoHandle);
}

void Gui::checkShaderCompileSuccess(int shader)
{
	GLint status, type;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv(shader, GL_SHADER_TYPE, &type);
	if (status == GL_FALSE) {
		char buffer[1024];
		glGetShaderInfoLog(shader, 1024, NULL, buffer);
		printf("Failed to compile %s shader:\n%s",
				type == GL_VERTEX_SHADER ? "vertex" : "fragment",
				buffer);
		throw;
	}
}

void Gui::checkProgramLinkSuccess(int program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		char buffer[1024];
		glGetProgramInfoLog(vertHandle, 1024, NULL, buffer);
		printf("Failed to link shaders:\n%s", buffer);
		throw;
	}
}

void Gui::MainMenuBar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Sythin2")) {
			ImGui::MenuItem("Show Demo window", NULL, &Globals.showDemo);
			if (ImGui::MenuItem("Quit", NULL))
				Globals.quit = true;
			ImGui::EndMenu();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
				ImVec2(Constants.gui.menuBar.modeSpacing, 3));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 0));

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				Constants.gui.menuBar.modePlayingActive);
		if (Globals.mode == GlobalsHolder::Mode_Playing) {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modePlayingActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modePlayingActive);
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modePlayingIdle);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modePlayingHovered);
		}
		if (ImGui::Button("Playing"))
			Globals.mode = GlobalsHolder::Mode_Playing;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				Constants.gui.menuBar.modeWritingActive);
		if (Globals.mode == GlobalsHolder::Mode_Writing) {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modeWritingActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modeWritingActive);
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modeWritingIdle);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modeWritingHovered);
		}
		if (ImGui::Button("Writing"))
			Globals.mode = GlobalsHolder::Mode_Writing;

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				Constants.gui.menuBar.modeRepeatingActive);
		if (Globals.mode == GlobalsHolder::Mode_Repeating) {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modeRepeatingActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modeRepeatingActive);
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button,
					Constants.gui.menuBar.modeRepeatingIdle);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					Constants.gui.menuBar.modeRepeatingHovered);
		}
		if (ImGui::Button("Replaying"))
			Globals.mode = GlobalsHolder::Mode_Repeating;

		ImGui::PopStyleColor(3*3);

		ImGui::PopStyleVar(3);

		ImGui::EndMainMenuBar();
	}
}

void Gui::TabBar()
{
	bool opened = true;

	ImVec2 windowSize(Constants.gui.width,
			40); // hardcode for now
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		// ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("TabBar", &opened, windowSize, Constants.gui.alpha,
			windowFlags);

	ImVec2 windowPos(
			Globals.windowWidth - Constants.gui.width - Constants.padding,
			Constants.padding + Constants.gui.menuBarGuiOffset);
	ImGui::SetWindowPos("TabBar", windowPos, ImGuiSetCond_Always);

	ImGui::PushStyleColor(ImGuiCol_ButtonActive, Constants.gui.tabs.active);
	if (Globals.tab == GlobalsHolder::Tab_Wave) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants.gui.tabs.active);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants.gui.tabs.active);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants.gui.tabs.idle);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants.gui.tabs.hovered);
	}
	if (ImGui::Button("Wave")) {
		Globals.tab = GlobalsHolder::Tab_Wave;
		settingsOpen = false;
		waveOpen = true;
	}
	ImGui::SameLine();
	if (Globals.tab == GlobalsHolder::Tab_Settings) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants.gui.tabs.active);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants.gui.tabs.active);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants.gui.tabs.idle);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants.gui.tabs.hovered);
	}
	if (ImGui::Button("Settings")) {
		Globals.tab = GlobalsHolder::Tab_Settings;
		settingsOpen = true;
		waveOpen = false;
	}
	ImGui::PopStyleColor(2*2+1);

	ImGui::End();
}

bool Gui::BeginSettingsWindow()
{
	if (!settingsOpen)
		return false;
	ImVec2 windowSize(Constants.gui.width,
			Globals.windowHeight - Constants.padding*2 -
			Constants.gui.menuBarGuiOffset - 40 - Constants.padding);
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Settings", &settingsOpen,
			windowSize, Constants.gui.alpha, windowFlags);

	ImVec2 windowPos(
			Globals.windowWidth - Constants.gui.width - Constants.padding,
			Constants.padding + Constants.gui.menuBarGuiOffset +
			40 + Constants.padding);
	ImGui::SetWindowPos("Settings", windowPos, ImGuiSetCond_Always);
	return true;
}

void Gui::WaveWindow(bool *shouldCompile)
{
	if (!waveOpen)
		return;
	ImVec2 windowSize(Globals.windowWidth - Constants.padding -
			Constants.gui.width - Constants.padding - Constants.padding,
			400);
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse;

	ImVec2 windowPos(
			Constants.padding,
			Constants.padding + Constants.gui.menuBarGuiOffset);
	ImGui::SetWindowPos("Wave", windowPos, ImGuiSetCond_Always);

	ImGui::Begin("Wave", &waveOpen,
			windowSize, Constants.gui.alpha, windowFlags);

	for (size_t i = 0; i < 16*1024; i++)
		editingBuffer[i] = '\0';

	WaveWindowFileOps();

	ImGui::SameLine();

	*shouldCompile = false;
	if (ImGui::Button("Compile"))
		*shouldCompile = true;
	ImGui::PopStyleVar();

	ImGui::InputTextMultiline("##source", editingBuffer, 16*1024,
			ImVec2(-1.0f, ImGui::GetTextLineHeight()*10),
			ImGuiInputTextFlags_AllowTabInput);

	ImGui::End();
}

void Gui::WaveWindowFileOps()
{
	static bool failedToSave = false, failedToRead = false;
	static bool once = true;

	if (once) {
		once = false;
		memset(editingBuffer, 0, 16*1024);
		FILE *f = fopen("wave.lua", "rb");
		if (!f) {
			f = fopen("wave.lua", "wb");
			fputs(Constants.defaultWaveScript, f);
			fclose(f);
			f = fopen("wave.lua", "rb");
			if (!f) {
				failedToSave = true;
				failedToRead = false;
			}
		}
		fseek(f, 0, SEEK_END);
		size_t fileSize = ftell(f);
		rewind(f);
		if (fread(editingBuffer, 1, fileSize, f) != fileSize) {
			failedToSave = false;
			failedToRead = true;
		}
		fclose(f);
	}
	if (failedToSave) {
		ImGui::TextColored(ImVec4(1, 0.3, 0.3, 1),
				"Failed to save new file \"wave.lua\" (this shouldn't happen)");
		ImGui::End();
		return;
	}
	if (failedToRead) {
		ImGui::TextColored(ImVec4(1, 0.3, 0.3, 1),
				"Failed to read file \"wave.lua\"");
		ImGui::End();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
	if (ImGui::Button("File"))
		ImGui::OpenPopup("file");
	static bool newPopup = false, reopenPopup = false;
	if (ImGui::BeginPopup("file")) {
		if (ImGui::Selectable("New"))
			newPopup = true;
		if (ImGui::Selectable("Save")) {
			FILE *f = fopen("wave.lua", "wb");
			if (!f) {
				failedToSave = true;
				failedToRead = false;
			}
			fputs(editingBuffer, f);
			fclose(f);
		}
		if (ImGui::Selectable("Reopen"))
			reopenPopup = true;
		ImGui::EndPopup();
	}
	if (newPopup) {
		ImGui::OpenPopup("Start over?");
		newPopup = false;
	}
	if (ImGui::BeginPopupModal("Start over?", NULL,
				ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Your changes will be overwritten and discarded.\n"
				"This operation cannot be undone!\n");
		// ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120,0))) {
			strncpy(editingBuffer, Constants.defaultWaveScript, 16*1024);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120,0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	if (reopenPopup) {
		ImGui::OpenPopup("Reopen file?");
		reopenPopup = false;
	}
	if (ImGui::BeginPopupModal("Discard changes?", NULL,
				ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Your changes will be overwritten and discarded.\n"
				"This operation cannot be undone!\n");
		// ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120,0))) {
			once = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120,0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Gui::CreateFontTexture(ImFont *imFont)
{
	font = imFont;

	ImGuiIO& io = ImGui::GetIO();

	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);

	io.Fonts->TexID = (void*)(intptr_t)fontTexture;
}

void Gui::Update(int dt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = dt/1000.;
	io.MousePos = ImVec2(mousePosX, mousePosY);
	io.MouseDown[0] = mousePressed[0];
	io.MouseDown[1] = mousePressed[1];
}

void Gui::Draw()
{
	glBindVertexArray(vaoHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glEnableVertexAttribArray(attribLocationPosition);
	glEnableVertexAttribArray(attribLocationUV);
	glEnableVertexAttribArray(attribLocationColor);
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
	glVertexAttribPointer(attribLocationPosition, 2, GL_FLOAT, GL_FALSE,
			sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(attribLocationUV, 2, GL_FLOAT, GL_FALSE,
			sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
			sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ImGui::Render();
}

Gui::~Gui()
{
	// delete [] editingBuffer;

	if (vaoHandle) glDeleteVertexArrays(1, &vaoHandle);
	if (vboHandle) glDeleteBuffers(1, &vboHandle);
	if (elementsHandle) glDeleteBuffers(1, &elementsHandle);
	vaoHandle = vboHandle = elementsHandle = 0;

	glDetachShader(shaderHandle, vertHandle);
	glDeleteShader(vertHandle);
	vertHandle = 0;

	glDetachShader(shaderHandle, fragHandle);
	glDeleteShader(fragHandle);
	fragHandle = 0;

	glDeleteProgram(shaderHandle);
	shaderHandle = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->ClearInputData();
	io.Fonts->ClearTexData();

	if (fontTexture) {
		glDeleteTextures(1, &fontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		fontTexture = 0;
	}
	ImGui::Shutdown();
}

