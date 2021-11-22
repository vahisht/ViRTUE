#ifndef _ViR2_OPENGL
#define _ViR2_OPENGL


//#define _NON_VR_VERSION_ONLY

#define SDL_MAIN_HANDLED // SDL2 has it's on "main" function that would cause collision with our main during compilation
#define M_PI 3.14159265359


#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <gl/glew.h>
#include <openvr.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <render_object.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct GlobalOpenglData {
	std::string programName;

	// SDL_Window
	SDL_Window *mainWindow;

	// Opengl context handle
	SDL_GLContext mainContext;
	int w_width, w_height, w_pos_x, w_pos_y;

	bool loop;

	ViR2::Shader testingShader, passthroughShader;
	ViR2::MeshGeometry fullscreenQuad;
	ViR2::FullScreenQuadObject fullscreen;

	GlobalOpenglData() { loop = true; };
};
extern GlobalOpenglData globalOpenglData;


struct GlobalVRData {
	vr::IVRSystem* vr_context;
	vr::IVRRenderModels* vr_render_models;
	vr::IVRCompositor* vr_compositor;

	std::string m_strDriver;
	std::string m_strDisplay;
	float fNearZ, fFarZ;
	int base_stations_count;

	uint32_t render_width;
	uint32_t render_height;

	GLuint l_frame_buffer, l_tex_color_buffer, l_aux_tex_color_buffer, l_render_buffer_depth_stencil;
	GLuint r_frame_buffer, r_tex_color_buffer, r_aux_tex_color_buffer, r_render_buffer_depth_stencil;

	glm::mat4 projection_matrix_left, projection_matrix_right;			// Used to maintain the HMD projection matrix for each eye
	glm::mat4 view_matrix_left, view_matrix_right;						// Used to maintain the HMD view matrix for each eye (view matrix involves head position and eye-to-head position)

	glm::mat4 hmd_pose_matrix;

	ViR2::RenderObject* VR_devices[ vr::k_unMaxTrackedDeviceCount ];

	vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 tracked_device_pose_matrix[vr::k_unMaxTrackedDeviceCount];

	ViR2::MeshGeometry VR_devices_models[vr::k_unMaxTrackedDeviceCount];

	int screenshot_count;

	std::vector<std::string> algorithms;
	std::vector<std::string> style;
	std::vector<std::string> scene;

	int algorithm_selected = 0;
	int style_selected = 0;
	int scene_selected = 0;

	GlobalVRData() { 
		base_stations_count = 0;
		screenshot_count = 0;
		vr_context = NULL;
		vr_render_models = NULL;
		vr_compositor = NULL;

		algorithms.push_back("basic");
		algorithms.push_back("stereo");

		style.push_back("1");
		style.push_back("2");
		style.push_back("3");

		scene.push_back("1");
		scene.push_back("2");
		scene.push_back("3");



	};
};
extern GlobalVRData globalVRData;


const float screenCoords[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
};

namespace ViR2 {

	bool Init();
	bool SetOpenGLAttributes();
	void PrintSDL_GL_Attributes();
	void CheckSDLError(int line);
	void handleEvents();
	//void draw();
	void Cleanup();
	void drawVRdevices(glm::mat4 Pmat, glm::mat4 Vmat);

	void submitRenderedToHMD();
	void renderImagesToHMD();
	void renderEyeToScreen();

	void checkOpenGLErrors();

	cv::Mat get_ocv_img_from_gl_img(GLuint ogl_texture_id);
	cv::Mat get_ocv_img_from_gl_depth(GLuint ogl_texture_id);
	GLuint makeTexture(cv::Mat image);
	void refreshDisplayedTexture();

}

#endif // !_ViR2_OPENGL