#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

static float aceleration = 0.5;
int maxpoints = 0;

class Object {
public:
	sf::Texture spriteTexture;
	std::unique_ptr<sf::Sprite> sprite;  // Smart pointer para gerenciar memória

	Object(std::string imgfile, float startXpos, float startYpos) {
		if (!spriteTexture.loadFromFile(imgfile)) {
			std::cerr << "Não foi possível carregar a imagem: " << imgfile << std::endl;
		}

		sprite = std::make_unique<sf::Sprite>(spriteTexture); // Inicializa com a textura
		sprite->setPosition({ startXpos, startYpos });
	}
};

class Pipe {
public:
	int pipe1Ypos;
	int pipe2Ypos;
	int PipeXpos;
	int PipeCenterYPos;
	int padding;
	bool Avaliable = true;
	bool Visible = true;

	bool direction = false;

	std::unique_ptr<Object> pipe1;
	std::unique_ptr<Object> pipe2;

	Pipe(int pipeXpos, int pipesYpos, int EspacePipes) {
		PipeCenterYPos = pipesYpos;
		pipe1Ypos = PipeCenterYPos + 150 + EspacePipes / 2;
		pipe2Ypos = PipeCenterYPos - 150 - EspacePipes / 2;
		PipeXpos = pipeXpos;

		pipe1 = std::make_unique<Object>("Sprites/pipe.png", PipeXpos, pipe1Ypos);
		pipe2 = std::make_unique<Object>("Sprites/pipe.png", PipeXpos, pipe2Ypos);

		pipe1->sprite->setOrigin({ 26, 160 });
		pipe2->sprite->setOrigin({ 26, 160 });

		pipe2->sprite->setRotation(sf::Angle(sf::degrees(180)));
	}

	void ChangePipePosition(int Xpos) {
		pipe1->sprite->setPosition({ static_cast<float>(Xpos),static_cast<float>(pipe1Ypos) });
		pipe2->sprite->setPosition({ static_cast<float>(Xpos),static_cast<float>(pipe2Ypos) });
	}

	void ChangePipeVerticalPositionAndPipePadding(int newYpos, int EspaceBetweenPipes) {
		padding = EspaceBetweenPipes / 2;
		pipe1Ypos = newYpos + 150 + padding;
		pipe2Ypos = newYpos - 150 - padding;

		ChangePipePosition(PipeXpos);
	}

	bool DetectPoint(int birdXpos) {
		
		if (PipeXpos > 120 and PipeXpos < 170 and Avaliable == true) {
			Avaliable = false;
			return true;
		}
		return false;
	}
};

class Button {
public:
	std::unique_ptr<Object> buttonSprite;
	Button(int ButtonXpos, int ButtonYpos, std::string buttonSpriteAdress, int originX, int originY) {
		buttonSprite = std::make_unique<Object>(buttonSpriteAdress, ButtonXpos, ButtonYpos);
		buttonSprite->sprite->setOrigin(sf::Vector2f(static_cast<float>(originX), static_cast<float>(originY)));

	}

	bool DetectButtonClick(sf::RenderWindow& window) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {

			sf::Vector2i mousePos = sf::Mouse::getPosition(window);

			sf::FloatRect buttonBounds = buttonSprite->sprite->getGlobalBounds();

			if (buttonBounds.contains(static_cast<sf::Vector2f> (mousePos))) {
				return true;
			}
		}
	
		return false;
	}
};

int main() {
	start:
	const int width = 608;
	const int height = 457;

	std::unique_ptr<sf::RenderWindow> window(new sf::RenderWindow(sf::VideoMode({ width, height }), "Jogo", sf::Style::Close | sf::Style::Titlebar));
	
	window->setFramerateLimit(40);

	float birdXpos = 150;
	float birdYpos = height / 2.0f;
	float birdYvel = 0;
	int flapCounter = 0;

	Object bird("Sprites/birdUp.png", birdXpos, birdYpos);
	bird.sprite->setOrigin({ 17, 12 });

	float backgroundVelocity = 3;
	float bg1Xpos = 0;
	float bg2Xpos = width;

	Object background1("Sprites/background.png", bg1Xpos, height);
	Object background2("Sprites/background.png", bg2Xpos, height);

	float pipeVelocity = 4;
	float pipeHardRockVerticalVelocity = 1;

	Pipe pipes1(1000, birdYpos + 100, 150);
	Pipe pipes2(1200, birdYpos + 130, 200); 
	Pipe pipes3(1400, birdYpos - 50, 170); 
	Pipe pipes4(1600, birdYpos - 130, 189); 
	Pipe pipes5(1800, birdYpos + 10, 165); 

	bool Dead = false;
	bool InMenu = false;
	bool InChooseMode = false;

	bool doubleTime = false;
	bool hardRock = false;
	bool hidden = false;

	int points = 0;

	Object chooseSign("Sprites/Modes/choosemode.png", 0, 0);
	Button doubleTimeButton(0, 0, "Sprites/Modes/doubletime.png", 71, 112);
	Button hardRockButton(0, 0, "Sprites/Modes/hardrock.png", 115, 122);
	Button hiddenButton(0, 0, "Sprites/Modes/hidden.png", 51, 115);
	Button defaultDiff(0, 0, "Sprites/Modes/default.png", 144, 39);

	chooseSign.sprite->setOrigin({91, 38});

	doubleTimeButton.buttonSprite->sprite->setScale({ 0.70, 0.70 });
	hardRockButton.buttonSprite->sprite->setScale({ 0.70, 0.70 });
	hiddenButton.buttonSprite->sprite->setScale({ 0.70, 0.70 });
	defaultDiff.buttonSprite->sprite->setScale({ 0.70, 0.70 });

	sf::SoundBuffer Flapbuffer;
	if (!Flapbuffer.loadFromFile("SoundEfects/wing.wav")) {
		return -1;
	}
	sf::Sound flapSound(Flapbuffer);
	flapSound.setVolume(50);

	sf::SoundBuffer PointBuffer;
	if (!PointBuffer.loadFromFile("SoundEfects/point.wav")) {
		return -1;
	}
	sf::Sound pointSound(PointBuffer);

	sf::SoundBuffer dieBuffer;
	if (!dieBuffer.loadFromFile("SoundEfects/die.wav")) {
		return -1;
	}
	sf::Sound dieSound(dieBuffer);

	sf::SoundBuffer hitBuffer;
	if (!hitBuffer.loadFromFile("SoundEfects/hit.wav")) {
		return -1;
	}
	sf::Sound hitSound(hitBuffer);

	sf::Texture dtBackground;
	if (!dtBackground.loadFromFile("Sprites/dtbackground.png")) {
		return -1;
	}

	sf::Texture hdBackground;
	if (!hdBackground.loadFromFile("Sprites/hdbackground.png")) {
		 return -1;
	}

	while (window->isOpen()) {

		InMenu = true;
		while (InMenu) {
			
			Button menuScreen(width / 2, height / 2, "Sprites/title.png", 92, 133);
			menuScreen.buttonSprite->sprite->setPosition({ width / 2, height / 2 });

			while (const std::optional event = window->pollEvent()) {
				if (event->is<sf::Event::Closed>()) {
					window->close();
					return 0;
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
						window->close();
						return 0;
					}
					else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
						InMenu = false;
						pointSound.play();
					}
				}
			}
			
			bg1Xpos -= backgroundVelocity;
			bg2Xpos -= backgroundVelocity;

			bool clicked = menuScreen.DetectButtonClick(*window);

			if (clicked == true) {
				InMenu = false;
			}
			if (bg1Xpos <= -608) {
				bg1Xpos = 0;
			}
			if (bg2Xpos <= 0) {
				bg2Xpos = width;
			}

			background1.sprite->setPosition({ bg1Xpos,0 });
			background2.sprite->setPosition({ bg2Xpos,0 });

			window->clear();

			window->draw(*background1.sprite); window->draw(*background2.sprite);

			window->draw(*menuScreen.buttonSprite->sprite);

			window->display();
		}
		
		InChooseMode = true;
		int ChooseModeDelay = 30;
		while (InChooseMode) {
			ChooseModeDelay -= 1;
			while (const std::optional event = window->pollEvent()) {
				if (event->is<sf::Event::Closed>()) {
					Dead = true;
					InChooseMode = false;
					window->close();
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
						Dead = true;
						InChooseMode = false;
						window->close();
					}
				}
			}
			
			chooseSign.sprite->setPosition({ width / 2, 100 });
			hiddenButton.buttonSprite->sprite->setPosition({ 100, 250 });
			hardRockButton.buttonSprite->sprite->setPosition({ 300, 250 });
			doubleTimeButton.buttonSprite->sprite->setPosition({ 500, 250 });
			defaultDiff.buttonSprite->sprite->setPosition({ 300, 400 });

			bg1Xpos -= backgroundVelocity;
			bg2Xpos -= backgroundVelocity;
			
			if (bg1Xpos <= -608) {
				bg1Xpos = 0;
			}
			if (bg2Xpos <= 0) {
				bg2Xpos = width;
			}

			background1.sprite->setPosition({ bg1Xpos,0 });
			background2.sprite->setPosition({ bg2Xpos,0 });

			bool hrClicked = hardRockButton.DetectButtonClick(*window);
			bool hdClicked = hiddenButton.DetectButtonClick(*window);
			bool dtClicked = doubleTimeButton.DetectButtonClick(*window);
			bool defaulClicked = defaultDiff.DetectButtonClick(*window);

			if (ChooseModeDelay > 0) {
				goto skipAfterDelay;
			}
			
			if (hrClicked) {
				hardRock = true;
				InChooseMode = false;
			}
			else if (dtClicked) {
				doubleTime = true;
				InChooseMode = false;
			}
			else if (hdClicked) {
				hidden = true;
				InChooseMode = false;
			}
			else if (defaulClicked) {
				InChooseMode = false;
			}

			skipAfterDelay:

			window->clear();

			window->draw(*background1.sprite); window->draw(*background2.sprite);

			window->draw(*chooseSign.sprite);
			window->draw(*hiddenButton.buttonSprite->sprite);
			window->draw(*hardRockButton.buttonSprite->sprite);
			window->draw(*doubleTimeButton.buttonSprite->sprite);
			window->draw(*defaultDiff.buttonSprite->sprite);

			window->display();
		}

		if (doubleTime == true) {
			pipeVelocity *= 1.5;
			backgroundVelocity *= 3;
			
			background1.sprite->setTexture(dtBackground);
			background2.sprite->setTexture(dtBackground);
		}

		if (hidden == true) {

			background1.sprite->setTexture(hdBackground);
			background2.sprite->setTexture(hdBackground);	
		}

		if (hardRock == true) {
			pipes1.ChangePipeVerticalPositionAndPipePadding(birdYpos + 100, 125);
			pipes2.ChangePipeVerticalPositionAndPipePadding(birdYpos + 130, 125);
			pipes3.ChangePipeVerticalPositionAndPipePadding(birdYpos - 50 , 125);
			pipes4.ChangePipeVerticalPositionAndPipePadding(birdYpos - 130, 125);
			pipes5.ChangePipeVerticalPositionAndPipePadding(birdYpos + 10 , 125);
		}
		while (Dead == false) {

			while (const std::optional event = window->pollEvent()) {
				auto Flap = [&birdYvel, &flapCounter, &bird, &flapSound]() {
					flapSound.play();
					birdYvel = -7;
					flapCounter = 10;
					bird.spriteTexture.loadFromFile("Sprites/birdDown.png");
					bird.sprite->setTexture(bird.spriteTexture);
				};
				
				if (event->is<sf::Event::Closed>()) {
					window->close();
					return 0;
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {

					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
						window->close();
						return 0;
					}
					else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
						Flap();
					}
				}

				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
					Flap();
				}
			}

			auto updatePipePosition = [&pipeVelocity, &hardRock](Pipe& pipe, int respawnXPos) {
				pipe.PipeXpos -= pipeVelocity;

				if (pipe.PipeXpos <= -20) {
					pipe.PipeXpos = respawnXPos;
					pipe.ChangePipePosition(pipe.PipeXpos);
					
					std::uniform_int_distribution<int> padding(100, 150);

					if (hardRock == true) {
						std::uniform_int_distribution<int> padding(100, 125);
					}

					int RespawnPadding = padding(gen);
					int RespawnYpos;

					if (RespawnPadding > 150) {
						std::uniform_int_distribution<int> ypos(200, 220);
						int RespawnYpos = ypos(gen);
						pipe.ChangePipeVerticalPositionAndPipePadding(RespawnYpos, RespawnPadding);
						pipe.Avaliable = true;
						pipe.Visible = true;
					}
					else {
						std::uniform_int_distribution<int> ypos(100, 350);
						int RespawnYpos = ypos(gen);
						pipe.ChangePipeVerticalPositionAndPipePadding(RespawnYpos, RespawnPadding);
						pipe.Avaliable = true;
						pipe.Visible = true;
					}
				}
				else {
					pipe.ChangePipePosition(pipe.PipeXpos);
				}
			};

			auto DetectPoints = [birdXpos, &points, &pointSound](Pipe& pipe) {
				bool detect = pipe.DetectPoint(birdXpos);

				if (detect == true) {
					points += 1;
					pointSound.play();
				}
			};

			auto DrawPipes = [&window, &pipes1, &pipes2, &pipes3, &pipes4, &pipes5]() {
				if (pipes1.Visible) {
					window->draw(*pipes1.pipe1->sprite);
					window->draw(*pipes1.pipe2->sprite);
				}
				if (pipes2.Visible) {
					window->draw(*pipes2.pipe1->sprite);
					window->draw(*pipes2.pipe2->sprite);
				}
				if (pipes3.Visible) {
					window->draw(*pipes3.pipe1->sprite);
					window->draw(*pipes3.pipe2->sprite);
				}
				if (pipes4.Visible) {
					window->draw(*pipes4.pipe1->sprite);
					window->draw(*pipes4.pipe2->sprite);
				}
				if (pipes5.Visible) {
					window->draw(*pipes5.pipe1->sprite);
					window->draw(*pipes5.pipe2->sprite);
				}
			};

			auto DeathAnimation = [&window, &DrawPipes, &background1, &background2, &bird, &birdYpos]() {
				int upCounter = 0;

				for (int i = 0; i < 80; i++) {

					bird.sprite->setRotation(sf::Angle(sf::degrees(90)));

					window->clear();

					window->draw(*background1.sprite); window->draw(*background2.sprite);

					window->draw(*bird.sprite);

					DrawPipes();

					window->display();
				}
			};

			auto DetectColisionWithPlayer = [&bird, &window, &Dead, &DeathAnimation, &hitSound, &dieSound](Pipe& pipes) {
				if (bird.sprite->getGlobalBounds().findIntersection(pipes.pipe1->sprite->getGlobalBounds()) or
					bird.sprite->getGlobalBounds().findIntersection(pipes.pipe2->sprite->getGlobalBounds())) {
					hitSound.play();
					DeathAnimation();
					Dead = true;
					dieSound.play();
				}
			};

			auto UpdateVisibilityWhenHiddenMod = [](Pipe& pipes) {
				if (pipes.PipeXpos < 300) {
					pipes.Visible = false;
				}

			};

			auto UpdatePoints = [&points, &window]() {
				if (points < 10) {
					Object pointCounter("Sprites/Numbers/0.png", width - 50, height - 50);
					std::string address = "Sprites/Numbers/";
					std::string type = ".png";
					std::string point = std::to_string(points);
					std::string image = address + point + type;
					sf::Texture texture;
					if (!texture.loadFromFile(image)) {
						return -1;
					}
					pointCounter.sprite->setTexture(texture);

					window->draw(*pointCounter.sprite);

				}
				else if (points >= 10) {
					Object pointCounterUn("Sprites/Numbers/0.png", width - 50, height - 50);
					std::string address = "Sprites/Numbers/";
					std::string type = ".png";

					int unidade = points % 10;
					std::string point = std::to_string(unidade);
					std::string image = address + point + type;
					sf::Texture texture;
					if (!texture.loadFromFile(image)) {
						return -1;
					}

					Object pointCounterDez("Sprites/Numbers/0.png", width - 74, height - 50);
					int dezena = (points / 10) % 10;
					std::string pointDez = std::to_string(dezena);
					std::string image2 = address + pointDez + type;
					sf::Texture texture2;
					if (!texture2.loadFromFile(image2)) {
						return -1;
					}

					pointCounterUn.sprite->setTexture(texture);
					pointCounterDez.sprite->setTexture(texture2);

					window->draw(*pointCounterUn.sprite); 	window->draw(*pointCounterDez.sprite);

				}
			};

			// ajeitar os calculos 
			auto UpdatePipeYPosWhenInHardRockMode = [&pipeHardRockVerticalVelocity](Pipe &pipe) {
				if (pipe.PipeCenterYPos > 350) {
					pipe.direction = true;
				}
				else if (pipe.PipeCenterYPos < 100) {
					pipe.direction = false;
				}

				if (pipe.direction) {
					pipe.PipeCenterYPos -= pipeHardRockVerticalVelocity;
				}
				else {
					pipe.PipeCenterYPos += pipeHardRockVerticalVelocity;
				}
				
				pipe.pipe1Ypos = pipe.PipeCenterYPos + 150 + pipe.padding;
				pipe.pipe2Ypos = pipe.PipeCenterYPos - 150 - pipe.padding;
				pipe.ChangePipePosition(pipe.PipeXpos);
			};

			auto UpdatePipe = [&updatePipePosition, &DetectColisionWithPlayer, &DetectPoints, &hidden, &hardRock, &UpdatePipeYPosWhenInHardRockMode, &UpdateVisibilityWhenHiddenMod](Pipe& pipe) {
				updatePipePosition(pipe, 1000);
				DetectColisionWithPlayer(pipe);
				DetectPoints(pipe);
				if (hidden == true) {
					UpdateVisibilityWhenHiddenMod(pipe);
				}
				else if (hardRock == true) {
					UpdatePipeYPosWhenInHardRockMode(pipe);
				}
			};

			UpdatePipe(pipes1);
			UpdatePipe(pipes2);
			UpdatePipe(pipes3);
			UpdatePipe(pipes4);
			UpdatePipe(pipes5);

			birdYvel += aceleration;
			birdYpos += birdYvel;
			bg1Xpos -= backgroundVelocity;
			bg2Xpos -= backgroundVelocity;
			flapCounter -= 1;

			bird.sprite->setRotation(sf::Angle(sf::degrees(birdYvel * 3)));

			if (flapCounter <= 0) {
				bird.spriteTexture.loadFromFile("Sprites/birdUp.png");
				bird.sprite->setTexture(bird.spriteTexture);
			}
			if (bg1Xpos <= -608) {
				bg1Xpos = 0;
			}
			if (bg2Xpos <= 0) {
				bg2Xpos = width;
			}

			bird.sprite->setPosition({ birdXpos,birdYpos });
			background1.sprite->setPosition({ bg1Xpos,0 });
			background2.sprite->setPosition({ bg2Xpos,0 });

			// render
			window->clear();

			// drawing
			window->draw(*background1.sprite); window->draw(*background2.sprite);

			window->draw(*bird.sprite);

			DrawPipes();

			UpdatePoints();

			window->display();
		}

		while (Dead == true) {
			Button gameOver(0, 0, "Sprites/gameover.png", 96, 21);
			gameOver.buttonSprite->sprite->setPosition({ width / 2, height / 2 });

			bool clicked = gameOver.DetectButtonClick(*window);

			while (const std::optional event = window->pollEvent()) {
				if (event->is<sf::Event::Closed>()) {
					window->close();
					return 0;
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
						window->close();
						return 0;
					}
					else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
						goto start;
					}
				}
			}

			bg1Xpos -= backgroundVelocity;
			bg2Xpos -= backgroundVelocity;

			if (bg1Xpos <= -608) {
				bg1Xpos = 0;
			}
			if (bg2Xpos <= 0) {
				bg2Xpos = width;
			}
			if (clicked == true) {
				goto start;
			}

			background1.sprite->setPosition({ bg1Xpos,0 });
			background2.sprite->setPosition({ bg2Xpos,0 });

			window->clear();

			window->draw(*background1.sprite); window->draw(*background2.sprite);

			window->draw(*gameOver.buttonSprite->sprite);

			window->display();
		}

		window->close();

	}

	return 0;
}