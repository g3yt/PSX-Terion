// A (very) simple program to calculate sprite width and height

#include <iostream>

int spritew, spriteh;
double spritescale;
double calculatedscale;

int main()
{
	//Get user input
	std::cout << "Type the sprite scale (example: 0.6)" << std::endl;
	std::cin >> spritescale; 

	std::cout << "Type the sprite width" << std::endl;
	std::cin >> spritew; 

	std::cout << "Type the sprite height" << std::endl;
	std::cin >> spriteh; 

	//Calculate stuff
	spritew *= spritescale / 4;
	spriteh *= spritescale / 4;

	std::cout << "image size is " << spritew << "x" << spriteh << std::endl;
}