#include "constants.hh"
#include "conv.hh"
#include "note.hh"
#include "note_atlas.hh"
#include "font.hh"
#include "fontloader.hh"

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "../bzip2-1.0.6/bzlib.h"
#include <memory>
#include "../imgui/imgui.h"

class MainLoop
{
public:
	sf::RenderWindow window;
	sf::Event event;
	sf::Time simulatedTime;
	sf::Clock clock;
	MainLoop() {
		sf::ContextSettings settings;
		settings.antialiasingLevel = Constants.antialiasing;
		window.create(sf::VideoMode(Globals.windowWidth, Globals.windowHeight),
				"sythin2",
				sf::Style::Titlebar | sf::Style::Close,
				settings);
		window.setKeyRepeatEnabled(false);
	};
	bool Update() {
		window.clear(Constants.backgroundColor);
		return window.isOpen();
	}
	void Display() {
		window.display();
	}
};

int main()
{
	MainLoop ml;

	sf::Font font;
	std::unique_ptr<char> fontFileBuffer;
	if (!loadEmbeddedFont(&font, &fontFileBuffer,
				_CommeLight_ttf.data, _CommeLight_ttf.size))
		return 1;

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(Globals.windowWidth, Globals.windowHeight);
	io.RenderDrawListsFn = ImImpl::ImImpl_RenderDrawLists;

	sf::Texture noteNamesAtlas;
	if (!note_atlas::CreateNoteTexture(font, &noteNamesAtlas))
		return 1;

	Note notes[3][12];
	for (int r = 0; r < 3; r++)
		for (int i = 0; i < 12; i++) {
			notes[r][i].SetHue((i/12.0)*360);
			int x = Constants.padding + i*(Constants.rectangle.size + Constants.padding);
			int y = Globals.windowHeight - Constants.padding - Constants.rectangle.size -
				(3-r-1)*(Constants.rectangle.size + Constants.padding);
			notes[r][i].SetPosition(x, y);
			notes[r][i].SetTexture(&noteNamesAtlas);
		}

	notes[0][ 0].key = sf::Keyboard::Num1;
	notes[0][ 1].key = sf::Keyboard::Num2;
	notes[0][ 2].key = sf::Keyboard::Num3;
	notes[0][ 3].key = sf::Keyboard::Num4;
	notes[0][ 4].key = sf::Keyboard::Num5;
	notes[0][ 5].key = sf::Keyboard::Num6;
	notes[0][ 6].key = sf::Keyboard::Num7;
	notes[0][ 7].key = sf::Keyboard::Num8;
	notes[0][ 8].key = sf::Keyboard::Num9;
	notes[0][ 9].key = sf::Keyboard::Num0;
	notes[0][10].key = sf::Keyboard::Dash;
	notes[0][11].key = sf::Keyboard::Equal;
	notes[0][ 0].SetNoteName(conv::C,  4);
	notes[0][ 1].SetNoteName(conv::Cs, 4);
	notes[0][ 2].SetNoteName(conv::D,  4);
	notes[0][ 3].SetNoteName(conv::Ds, 4);
	notes[0][ 4].SetNoteName(conv::E,  4);
	notes[0][ 5].SetNoteName(conv::F,  4);
	notes[0][ 6].SetNoteName(conv::Fs, 4);
	notes[0][ 7].SetNoteName(conv::G,  4);
	notes[0][ 8].SetNoteName(conv::Gs, 4);
	notes[0][ 9].SetNoteName(conv::A,  4);
	notes[0][10].SetNoteName(conv::As, 4);
	notes[0][11].SetNoteName(conv::B,  4);

	notes[1][ 0].key = sf::Keyboard::Q;
	notes[1][ 1].key = sf::Keyboard::W;
	notes[1][ 2].key = sf::Keyboard::E;
	notes[1][ 3].key = sf::Keyboard::R;
	notes[1][ 4].key = sf::Keyboard::T;
	notes[1][ 5].key = sf::Keyboard::Y;
	notes[1][ 6].key = sf::Keyboard::U;
	notes[1][ 7].key = sf::Keyboard::I;
	notes[1][ 8].key = sf::Keyboard::O;
	notes[1][ 9].key = sf::Keyboard::P;
	notes[1][10].key = sf::Keyboard::LBracket;
	notes[1][11].key = sf::Keyboard::RBracket;
	notes[1][ 0].SetNoteName(conv::C,  3);
	notes[1][ 1].SetNoteName(conv::Cs, 3);
	notes[1][ 2].SetNoteName(conv::D,  3);
	notes[1][ 3].SetNoteName(conv::Ds, 3);
	notes[1][ 4].SetNoteName(conv::E,  3);
	notes[1][ 5].SetNoteName(conv::F,  3);
	notes[1][ 6].SetNoteName(conv::Fs, 3);
	notes[1][ 7].SetNoteName(conv::G,  3);
	notes[1][ 8].SetNoteName(conv::Gs, 3);
	notes[1][ 9].SetNoteName(conv::A,  3);
	notes[1][10].SetNoteName(conv::As, 3);
	notes[1][11].SetNoteName(conv::B,  3);

	notes[2][ 0].key = sf::Keyboard::A;
	notes[2][ 1].key = sf::Keyboard::S;
	notes[2][ 2].key = sf::Keyboard::D;
	notes[2][ 3].key = sf::Keyboard::F;
	notes[2][ 4].key = sf::Keyboard::G;
	notes[2][ 5].key = sf::Keyboard::H;
	notes[2][ 6].key = sf::Keyboard::J;
	notes[2][ 7].key = sf::Keyboard::K;
	notes[2][ 8].key = sf::Keyboard::L;
	notes[2][ 9].key = sf::Keyboard::SemiColon;
	notes[2][10].key = sf::Keyboard::Quote;
	notes[2][11].key = sf::Keyboard::Return;
	notes[2][ 0].SetNoteName(conv::C,  2);
	notes[2][ 1].SetNoteName(conv::Cs, 2);
	notes[2][ 2].SetNoteName(conv::D,  2);
	notes[2][ 3].SetNoteName(conv::Ds, 2);
	notes[2][ 4].SetNoteName(conv::E,  2);
	notes[2][ 5].SetNoteName(conv::F,  2);
	notes[2][ 6].SetNoteName(conv::Fs, 2);
	notes[2][ 7].SetNoteName(conv::G,  2);
	notes[2][ 8].SetNoteName(conv::Gs, 2);
	notes[2][ 9].SetNoteName(conv::A,  2);
	notes[2][10].SetNoteName(conv::As, 2);
	notes[2][11].SetNoteName(conv::B,  2);

	while (ml.Update()) {
		sf::Time realTime = ml.clock.getElapsedTime();
		while (ml.simulatedTime < realTime) {
			while (ml.window.pollEvent(ml.event)) {
				if (ml.event.type == sf::Event::Closed)
					ml.window.close();
				if (ml.event.type == sf::Event::KeyPressed)
					for (int r = 0; r < 3; r++)
						for (int i = 0; i < 12; i++)
							if (ml.event.key.code == notes[r][i].key) {
								notes[r][i].keyPressed = true;
								notes[r][i].KeyPressed();
							}
				if (ml.event.type == sf::Event::KeyReleased)
					for (int r = 0; r < 3; r++)
						for (int i = 0; i < 12; i++)
							if (ml.event.key.code == notes[r][i].key) {
								notes[r][i].keyPressed = false;
								notes[r][i].KeyReleased();
							}
			}
			for (int r = 0; r < 3; r++)
				for (int i = 0; i < 12; i++)
					notes[r][i].Update();
			ml.simulatedTime += sf::milliseconds(Constants.updateMilliseconds);
		}

		ml.window.pushGLStates();

		for (int r = 0; r < 3; r++)
			for (int i = 0; i < 12; i++)
				notes[r][i].Draw(&ml.window);

		ml.window.popGLStates();

		ml.Display();
	}

	return 0;
}

