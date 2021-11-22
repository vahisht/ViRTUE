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

//#define GOLEM
//#define BUNNY
//#define DRAGONHEAD
//#define FROG
//#define HEAD
//#define MOVINGCASTLE
//#define PUMPKIN
#define SKULL
//#define TOYTRAIN


struct AppData {
	ViR2::Shader testingShader, passthroughShader, skyboxShader;
	ViR2::MeshGeometry testingGeometry, fullscreenQuad, testingGeometry2;

	ViR2::RenderObject testingObject, testingObject2;
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

// ------------------------------- TESTING GEOMETRIES -------------------------------
#ifdef GOLEM
	appData.testingGeometry = ViR2::MeshGeometry("models/golem.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-1.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3((-3.14 / 2.0), 0.0f, 0.0f));
#endif
#ifdef BUNNY
	appData.testingGeometry = ViR2::MeshGeometry("models/bunny.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-4.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, (3.14 / 2.0), 0.0f));
#endif
#ifdef DRAGONHEAD
	appData.testingGeometry = ViR2::MeshGeometry("models/dragonhead.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-4.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, 1.0, 0.0f));
#endif
#ifdef FROG
	appData.testingGeometry = ViR2::MeshGeometry("models/frog2.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-3.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, (5.0 / 2.0), 0.0f));
#endif
#ifdef HEAD
	appData.testingGeometry = ViR2::MeshGeometry("models/head.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-2.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, (3.14 / 2.0), 0.0f));
#endif
#ifdef MOVINGCASTLE
	appData.testingGeometry = ViR2::MeshGeometry("models/movingcastle2.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-2.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, 3.0, 0.0f));
#endif
#ifdef PUMPKIN
	appData.testingGeometry = ViR2::MeshGeometry("models/pumpkin.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-2.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, (3.14 / 2.0), 0.0f));
#endif
#ifdef SKULL
	appData.testingGeometry = ViR2::MeshGeometry("models/skull.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-2.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, (3.14 / 2.0), 0.0f));
#endif
#ifdef TOYTRAIN
	appData.testingGeometry = ViR2::MeshGeometry("models/toytrain.obj", &appData.testingShader);
	appData.testingObject = ViR2::RenderObject(&appData.testingGeometry, &appData.testingShader);
	appData.testingObject.move(glm::vec3(-2.0f, 1.0f, 0.0f));
	appData.testingObject.rotate(glm::vec3(0.0f, 0.0f, 0.0f));
#endif


// ----------------------------- TESTING GEOMETRIES END -----------------------------

	appData.testingGeometry2 = ViR2::MeshGeometry("models/sphere.obj", &appData.testingShader);
	appData.testingObject2 = ViR2::RenderObject(&appData.testingGeometry2, &appData.testingShader);
	appData.testingObject2.move(glm::vec3(-5.0f, 1.0f, 2.0f));

	appData.fullscreenQuad = ViR2::MeshGeometry(screenCoords, sizeof(screenCoords), &appData.passthroughShader);

	appData.skybox = ViR2::SkyboxObject(&appData.fullscreenQuad, &appData.skyboxShader, "data/skybox/nightsky");


	return true;
}

int main(int argc, char *argv[])
{
	if (!ViR2::Init())
		return -1;

	initAppData();

	ViR2::refreshDisplayedTexture();

	while ( globalOpenglData.loop ) {
		ViR2::handleEvents();
		/*ViR2::draw();*/

		{
			//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#ifndef _NON_VR_VERSION_ONLY

			//////////////////////////////////////////////
			// Render left eye...

			//std::cout << "Left render..." << std::endl;

			//glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.l_frame_buffer);
			glViewport(0, 0, globalVRData.render_width, globalVRData.render_height);
			glEnable(GL_DEPTH_TEST);
			GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, bufs);

			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//appData.skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f), globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix));
			//appData.testingObject2.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f), globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix));

			//ViR2::drawVRdevices(globalVRData.projection_matrix_left, globalVRData.view_matrix_left);

			ViR2::checkOpenGLErrors();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//////////////////////////////////////////////
			// Render right eye...

			//std::cout << "Right render..." << std::endl;

			//glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.r_frame_buffer);
			glViewport(0, 0, globalVRData.render_width, globalVRData.render_height);
			glEnable(GL_DEPTH_TEST);
			glDrawBuffers(2, bufs);
			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//appData.skybox.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f), globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix));
			//appData.testingObject2.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f), globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix));

			//ViR2::drawVRdevices(globalVRData.projection_matrix_right, globalVRData.view_matrix_right);

			ViR2::checkOpenGLErrors();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Submit render buffers to HMD
			//std::cout << "Submit ... " << std::endl;
			//ViR2::submitRenderedToHMD();
			ViR2::renderImagesToHMD();

			ViR2::checkOpenGLErrors();
			/////////////////////////////////////////////
			// Render what companion window will show
			// In this case we render what left eye is actually seeing
			//std::cout << "EyeToScreen ... " << std::endl;
			ViR2::renderEyeToScreen();

			ViR2::checkOpenGLErrors();

#else 
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.l_frame_buffer);
			glViewport(0, 0, globalOpenglData.w_width, globalOpenglData.w_height);
			glEnable(GL_DEPTH_TEST);

			GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, bufs);

			// Make our background white
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//appData.skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f));
			appData.testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f), globalVRData.view_matrix_right);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			ViR2::renderEyeToScreen();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			ViR2::checkOpenGLErrors();
#endif

			// Swap our buffers to make our changes visible
			SDL_GL_SwapWindow(globalOpenglData.mainWindow);
		}
	}

	//std::cout << globalVRData.

	ViR2::Cleanup();

	return 0;
}