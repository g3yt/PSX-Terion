//A little program to automatically create .png.txt files

#include <iostream>
#include <fstream>

int vramx, vramy, palx, paly, bpp;
char animname[30];
char curfile[30];
int animlength;

int main()
{
    std::ofstream dafile;
	
	//Get user input
	std::cout << "disclamer: this program will create a lot of files in the current directory you are in" << std::endl;
    std::cout << "input a vram x value" << std::endl;
    std::cin >> vramx;
    std::cout << "input a vram y value" << std::endl;
    std::cin >> vramy;
    std::cout << "input a pallete x value" << std::endl;
    std::cin >> palx;
    std::cout << "input a pallete y value" << std::endl;
    std::cin >> paly;
    std::cout << "input a bpp value (must be 4 or 8)" << std::endl;
    std::cin >> bpp;
    std::cout << "input the animation name" << std::endl;
    std::cin >> animname;
    std::cout << "input the amount of animations (starts from 0)" << std::endl;
    std::cin >> animlength;
    
    if (!(bpp == 4 || bpp == 8))
    {
    	std::cout << "invalid bpp!" << std::endl;
    	return 0;
    }

    //Write the files
    for (int i = 0; i <= animlength; i++)
    {
		sprintf(curfile, "%s%d.png.txt", animname, i);
    	dafile.open(curfile); // opens the file
    	dafile << vramx << " " << vramy << " " << palx << " " << paly << " " << bpp << std::endl; //Place the values in the file
    	if (curfile == NULL)
			std::cout << "failed to create " << curfile << std::endl;
    	else
			std::cout << "created " << curfile << std::endl;

    	dafile.close();
    }

    return 0;
}