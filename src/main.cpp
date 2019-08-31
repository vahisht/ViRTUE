/**
 * ViRTUE Example code
 * main.cpp
 * 
 * This code serve as an example of how to use the ViR2 extension for VR projects
 * 
 * Michal Kucera 07/2019, @VahishtHimself
 * DCGI FEE CTU (@CVUTFEL)
*/

#include <iostream>
#include <string>
#include <ViR2_opengl.hpp>
//#include <glm/glm.hpp>


int main(int argc, char *argv[])
{
	if (!ViR2::Init())
		return -1;

	while ( globalOpenglData.loop ) {
		ViR2::handleEvents();
		ViR2::draw();
	}

	ViR2::Cleanup();

	return 0;
}