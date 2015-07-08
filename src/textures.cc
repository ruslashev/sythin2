#include "textures.hh"
#include "constants.hh"
#include "conv.hh"

std::unique_ptr<sf::Texture> textures::CreateNoteTexture(sf::Font &font)
{
	const int precomputedOctaves = 5;
	struct noteSymbol { char letter, accidental; int octave; };
	std::vector<noteSymbol> noteNames;
	for (int i = 0; i < precomputedOctaves; i++) {
		noteNames.push_back({ 'A', ' ', i+1 });
		noteNames.push_back({ 'A', '#', i+1 });
		noteNames.push_back({ 'B', ' ', i+1 });
		noteNames.push_back({ 'C', ' ', i+1 });
		noteNames.push_back({ 'C', '#', i+1 });
		noteNames.push_back({ 'D', ' ', i+1 });
		noteNames.push_back({ 'D', '#', i+1 });
		noteNames.push_back({ 'E', ' ', i+1 });
		noteNames.push_back({ 'F', ' ', i+1 });
		noteNames.push_back({ 'F', '#', i+1 });
		noteNames.push_back({ 'G', ' ', i+1 });
		noteNames.push_back({ 'G', '#', i+1 });
	}

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(Constants.text.size);

	sf::Color textColor = conv::HSVtoRGB(0, 0, 10),
		background = conv::HSVtoRGB(0, 0, 97);

	sf::RenderTexture textRT;
	if (!textRT.create(1000, 200))
		throw;
	textRT.clear(background);

	int counterX = 0;

	for (auto &note : noteNames) {
		int textureSegmentWidth = 0;
		text.setString(note.letter);
		textureSegmentWidth += text.getLocalBounds().width;
		if (note.accidental != ' ') {
			text.setString(note.accidental);
			textureSegmentWidth += text.getLocalBounds().width;
			textureSegmentWidth += Constants.text.spacing;
		}
		text.setString(std::to_string(note.octave));
		textureSegmentWidth += text.getLocalBounds().width;
		textureSegmentWidth += Constants.text.spacing;
		int textureSegmentHeight = text.getLocalBounds().height;

		text.setString(note.letter);
		auto rect = text.getLocalBounds();
		text.setPosition(-rect.left, -rect.top);
		text.move(counterX, 0);

		text.setColor(textColor);
		textRT.draw(text);

		int xof = 0;
		if (note.accidental != ' ') {
			text.setString(note.accidental);
			auto thisRect = text.getLocalBounds();
			text.setPosition(-thisRect.left, -thisRect.top);
			xof = rect.width + Constants.text.spacing;
			text.move(xof, 0);
			text.move(counterX, 0);

			text.setColor(textColor);
			textRT.draw(text);

			rect = text.getLocalBounds();
		}

		text.setString(std::to_string(note.octave));
		auto thisRect = text.getLocalBounds();
		text.setPosition(-thisRect.left, -thisRect.top);
		text.move(xof + rect.width + Constants.text.spacing, 0);
		text.move(counterX, 0);

		text.setColor(textColor);
		textRT.draw(text);

		textRT.display();

		positions.push_back({ counterX,
				textureSegmentWidth, textureSegmentHeight });
		counterX += textureSegmentWidth;

		/*
		textSprite.setPosition(
				rectangleShape.getPosition().x +
				rectangleShape.getLocalBounds().width/2 -
				textSprite.getLocalBounds().width/2,
				rectangleShape.getPosition().y +
				rectangleShape.getLocalBounds().height/2 -
				textSprite.getLocalBounds().height/2);
		*/
	}
	std::unique_ptr<sf::Texture> noteNamesAtlas(new sf::Texture(textRT.getTexture()));
	return noteNamesAtlas;
}

// textures::Rect textures::LookupNotePosition(char letter, char accidental, int octave)
// {

// }

