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


struct AppData {
	ViR2::Shader testingShader, passthroughShader, skyboxShader;
	ViR2::MeshGeometry testingGeometry, fullscreenQuad;

	ViR2::RenderObject testingObject;
	ViR2::FullScreenQuadObject fullscreen;
	ViR2::SkyboxObject skybox;
} appData;



bool initAppData() {
	std::cout << "Initializing shaders" << std::endl;
	appData.testingShader = ViR2::Shader("shader/shader.vert", "shader/shader.frag");
	appData.passthroughShader = ViR2::Shader("shader/pass_tex.vert", "shader/pass_tex.frag");
	appData.skyboxShader = ViR2::Shader("shader/skybox.vert", "shader/skybox.frag");

	appData.testingShader.init();
	appData.passthroughShader.init();
	appData.skyboxShader.init();

	appData.testingGeometry = ViR2::MeshGeometry("models/teddy.obj", &appData.testingShader);
	appData.fullscreenQuad = ViR2::MeshGeometry(screenCoords, sizeof(screenCoords), &appData.passthroughShader);

	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.skybox = ViR2::SkyboxObject(&appData.fullscreenQuad, &appData.skyboxShader, "data/skybox/nightsky");

	appData.testingObject.move(glm::vec3(-3.0f, 1.0f, 0.0f));

	return true;
}

int main(int argc, char *argv[])
{
	if (!ViR2::Init())
		return -1;

	initAppData();

	while ( globalOpenglData.loop ) {
		ViR2::handleEvents();
		/*ViR2::draw();*/

		{
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

#ifndef _NON_VR_VERSION_ONLY

			//////////////////////////////////////////////
			// Render left eye...

			glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.l_frame_buffer);
			glViewport(0, 0, globalVRData.render_width, globalVRData.render_height);
			glEnable(GL_DEPTH_TEST);

			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			appData.skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));

			ViR2::drawVRdevices(globalVRData.projection_matrix_left, globalVRData.view_matrix_left);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//////////////////////////////////////////////
			// Render right eye...

			glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.r_frame_buffer);
			glViewport(0, 0, globalVRData.render_width, globalVRData.render_height);
			glEnable(GL_DEPTH_TEST);

			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			appData.skybox.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));

			ViR2::drawVRdevices(globalVRData.projection_matrix_right, globalVRData.view_matrix_right);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Submit render buffers to HMD
			ViR2::submitRenderedToHMD();

			/////////////////////////////////////////////
			// Render what companion window will show
			// In this case we render what left eye is actually seeing
			ViR2::renderEyeToScreen();

			ViR2::checkOpenGLErrors();

#else 
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, globalOpenglData.w_width, globalOpenglData.w_height);
			glEnable(GL_DEPTH_TEST);

			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			appData.skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f));

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

			// Swap our buffers to make our changes visible
			SDL_GL_SwapWindow(globalOpenglData.mainWindow);
		}
	}

	ViR2::Cleanup();

	return 0;
}