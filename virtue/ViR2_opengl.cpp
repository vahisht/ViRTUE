#include <ViR2_opengl.hpp>

GlobalOpenglData globalOpenglData = GlobalOpenglData(); // Init OpenGL global data structure
GlobalVRData globalVRData = GlobalVRData(); // Init OpenGL global data structure


std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t requiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (requiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[requiredBufferLen];
	requiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, requiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


std::string GetTrackedDeviceClassString(vr::ETrackedDeviceClass td_class) {

	std::string str_td_class = "Unknown class";

	switch (td_class)
	{
	case vr::TrackedDeviceClass_Invalid:			// = 0, the ID was not valid.
		str_td_class = "Invalid";
		break;
	case vr::TrackedDeviceClass_HMD:				// = 1, Head-Mounted Displays
		str_td_class = "HMD";
		break;
	case vr::TrackedDeviceClass_Controller:			// = 2, Tracked controllers
		str_td_class = "Controller";
		break;
	case vr::TrackedDeviceClass_GenericTracker:		// = 3, Generic trackers, similar to controllers
		str_td_class = "Generic Tracker";
		break;
	case vr::TrackedDeviceClass_TrackingReference:	// = 4, Camera and base stations that serve as tracking reference points
		str_td_class = "Tracking Reference";
		break;
	case vr::TrackedDeviceClass_DisplayRedirect:	// = 5, Accessories that aren't necessarily tracked themselves, but may redirect video output from other tracked devices
		str_td_class = "Display Redirecd";
		break;
	}

	return str_td_class;
}

bool setup_render_model(vr::TrackedDeviceIndex_t tracked_device) {

	if (tracked_device >= vr::k_unMaxTrackedDeviceCount)
		return false;

	std::string render_model_name = GetTrackedDeviceString(globalVRData.vr_context, tracked_device, vr::Prop_RenderModelName_String);

	vr::RenderModel_t *model;
	vr::EVRRenderModelError error;

	std::cout << "Starting loading render model's model (" << render_model_name.c_str() << ")" << std::endl;
	while (1)
	{
		error = vr::VRRenderModels()->LoadRenderModel_Async(render_model_name.c_str(), &model);
		if (error != vr::VRRenderModelError_Loading)
			break;

		SDL_Delay(1);
	}
	std::cout << "Render model's model succesfully loaded" << std::endl;

	if (error != vr::VRRenderModelError_None)
	{
		std::cout << "Unable to load render model " << render_model_name.c_str() << ". Error code: " << vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error);
		vr::VRRenderModels()->FreeRenderModel(model);
		return false;
	}


	vr::RenderModel_TextureMap_t *rm_texture;
	std::cout << "Starting loading render model's texture (" << render_model_name.c_str() << ")" << std::endl;
	while (1)
	{
		error = vr::VRRenderModels()->LoadTexture_Async(model->diffuseTextureId, &rm_texture);
		if (error != vr::VRRenderModelError_Loading)
			break;

		SDL_Delay(1);
	}
	std::cout << "Render model's texture succesfully loaded" << std::endl;

	if (error != vr::VRRenderModelError_None)
	{
		std::cout << "Unable to load render texture id " << model->diffuseTextureId << " for render model " << render_model_name.c_str();
		vr::VRRenderModels()->FreeRenderModel(model);
		return false;
	}

	globalVRData.VR_devices_models[tracked_device] = ViR2::MeshGeometry(model, rm_texture, &globalOpenglData.testingShader);
	globalVRData.VR_devices[tracked_device] = new ViR2::RenderObject(&globalVRData.VR_devices_models[tracked_device], &globalOpenglData.testingShader);


	vr::VRRenderModels()->FreeRenderModel(model);
	vr::VRRenderModels()->FreeTexture(rm_texture);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "Error " << err << " with render model " << render_model_name.c_str() << std::endl;
	}

	return true;

}

bool setup_frame_buffer(GLsizei width, GLsizei height, GLuint &frame_buffer, GLuint &render_buffer_depth_stencil, GLuint &tex_color_buffer, GLuint& aux_tex_color_buffer) {



	// Create and bind the frame buffer
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	// Create texture where we are going to bulk the contents of the frame buffer
	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Attach the image to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0); // The second parameter implies that you can have multiple color attachments. A fragment shader can output
																									  // different data to any of these by linking 'out' variables to attachments with the glBindFragDataLocation function

	glGenTextures(1, &aux_tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, aux_tex_color_buffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Attach the image to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, aux_tex_color_buffer, 0);

	// Create the render buffer to host the depth and stencil buffers
	// Although we could do this by creating another texture, it is more efficient to store these buffers in a Renderbuffer Object, because we're only interested in reading the color buffer in a shader
	/*glGenRenderbuffers(1, &render_buffer_depth_stencil);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_depth_stencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Attach the render buffer to the framebuffer
	// NOTE: Even if you don't plan on reading from this depth_attachment, an off screen buffer that will be rendered to should have a depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_depth_stencil);*/

	glGenTextures(1, &render_buffer_depth_stencil);
	glBindTexture(GL_TEXTURE_2D, render_buffer_depth_stencil);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, render_buffer_depth_stencil, 0);

	// Check whether the frame buffer is complete (at least one buffer attached, all attachmentes are complete, all attachments same number multisamples)
	(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? std::cout << "The frame buffer is complete!" << std::endl : std::cout << "The frame buffer is invalid, please re-check. Code: " << glCheckFramebufferStatus(frame_buffer) << std::endl;

	return true;
}

glm::mat4 get_projection_matrix(const vr::Hmd_Eye eye)
{
	vr::HmdMatrix44_t steamvr_proj_matrix = globalVRData.vr_context->GetProjectionMatrix(eye, 0.1f, 15.0f);
	std::cout << "Focal length: " << steamvr_proj_matrix.m[0][0] << std::endl;

	return glm::mat4(steamvr_proj_matrix.m[0][0], steamvr_proj_matrix.m[1][0], steamvr_proj_matrix.m[2][0], steamvr_proj_matrix.m[3][0],
		steamvr_proj_matrix.m[0][1], steamvr_proj_matrix.m[1][1], steamvr_proj_matrix.m[2][1], steamvr_proj_matrix.m[3][1],
		steamvr_proj_matrix.m[0][2], steamvr_proj_matrix.m[1][2], steamvr_proj_matrix.m[2][2], steamvr_proj_matrix.m[3][2],
		steamvr_proj_matrix.m[0][3], steamvr_proj_matrix.m[1][3], steamvr_proj_matrix.m[2][3], steamvr_proj_matrix.m[3][3]);
}

glm::mat4 get_view_matrix(const vr::Hmd_Eye eye)
{
	vr::HmdMatrix34_t steamvr_eye_view_matrix = globalVRData.vr_context->GetEyeToHeadTransform(eye);

	glm::mat4 view_matrix = glm::mat4(steamvr_eye_view_matrix.m[0][0], steamvr_eye_view_matrix.m[1][0], steamvr_eye_view_matrix.m[2][0], 0.0f,
		steamvr_eye_view_matrix.m[0][1], steamvr_eye_view_matrix.m[1][1], steamvr_eye_view_matrix.m[2][1], 0.0f,
		steamvr_eye_view_matrix.m[0][2], steamvr_eye_view_matrix.m[1][2], steamvr_eye_view_matrix.m[2][2], 0.0f,
		steamvr_eye_view_matrix.m[0][3], steamvr_eye_view_matrix.m[1][3], steamvr_eye_view_matrix.m[2][3], 1.0f);

	return glm::inverse(view_matrix);
}


int initSDL() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // Enable to tell SDL that the old, deprecated GL code are disabled, only the newer versions can be used
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	/*SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);*/

	globalOpenglData.mainWindow = SDL_CreateWindow("Hello world VR", globalOpenglData.w_pos_x, globalOpenglData.w_pos_y, globalOpenglData.w_width, globalOpenglData.w_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (globalOpenglData.mainWindow == NULL) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	globalOpenglData.mainContext = SDL_GL_CreateContext(globalOpenglData.mainWindow);
	if (globalOpenglData.mainContext == NULL) {
		SDL_Log("Unable to create GL Context: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	if (SDL_GL_SetSwapInterval(1) < 0) // Configure VSync
	{
		SDL_Log("Warning: Unable to set VSync!: %s", SDL_GetError());
		return -1;
	}

	glewExperimental = GL_TRUE;		// Enable to tell OpenGL that we want to use OpenGL 3.0 stuff and later
	GLenum nGlewError = glewInit(); // Initialize glew
	if (nGlewError != GLEW_OK)
	{
		SDL_Log("Error initializing GLEW! %s", glewGetErrorString(nGlewError));
		return -1;
	}
	else { std::cout << "Glew init OK!" << std::endl; }

	std::string strWindowTitle = "openvr_main - " + globalVRData.m_strDriver + " " + globalVRData.m_strDisplay;
	SDL_SetWindowTitle(globalOpenglData.mainWindow, strWindowTitle.c_str());

	return 0;
}


int initOpenGL() {

	int success = 0;
	GLenum error = GL_NO_ERROR;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if ((error = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "Error initializing OpenGL!" << gluErrorString(error) << std::endl;
		return -1;
	}

	glViewport(0, 0, globalOpenglData.w_width, globalOpenglData.w_height);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	{

		globalOpenglData.passthroughShader = ViR2::Shader("shader/pass_tex.vert", "shader/pass_tex.frag");
		globalOpenglData.passthroughShader.init();
		globalOpenglData.fullscreenQuad = ViR2::MeshGeometry(screenCoords, sizeof(screenCoords), &globalOpenglData.passthroughShader);
		globalOpenglData.fullscreen = ViR2::FullScreenQuadObject(&globalOpenglData.fullscreenQuad, &globalOpenglData.passthroughShader, &(globalVRData.l_tex_color_buffer));
		//globalOpenglData.fullscreen = ViR2::FullScreenQuadObject(&globalOpenglData.fullscreenQuad, &globalOpenglData.passthroughShader, &(globalVRData.l_render_buffer_depth_stencil));
	
	}

	return success;
}

int initOpenVR() {

	if (vr::VR_IsHmdPresent())
	{
		std::cout << "An HMD was successfully found in the system" << std::endl;

		if (vr::VR_IsRuntimeInstalled()) {
			/*const char* runtime_path;//= vr::VR_RuntimePath();
			vr::VR_GetRuntimePath(runtime_path,);*/
			std::cout << "Runtime correctly installed" << std::endl;
		}
		else
		{
			std::cout << "Runtime was not found, quitting app" << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << "No HMD was found in the system, quitting app" << std::endl;
		return -1;
	}

	// Load the SteamVR Runtime
	vr::HmdError err;
	globalVRData.vr_context = VR_Init(&err, vr::EVRApplicationType::VRApplication_Scene);
	globalVRData.vr_context == NULL ? std::cout << "Error while initializing VRSystem. Error code is " << VR_GetVRInitErrorAsSymbol(err) << std::endl : std::cout << "VRSystem successfully initialized" << std::endl;

	// Load render models. Afaik this is used just to load generic render models for this implementation,
	// like base stations, controllers, sensors, etc.
	globalVRData.vr_render_models = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &err);
	if (!globalVRData.vr_render_models)
	{
		vr::VR_Shutdown();
		std::cout << "Couldn't load generic render models" << std::endl;
		return -1;
	}
	else
		std::cout << "Render models loaded successfully" << std::endl;

	for (uint32_t td = vr::k_unTrackedDeviceIndex_Hmd; td < vr::k_unMaxTrackedDeviceCount; td++) {

		if (globalVRData.vr_context->IsTrackedDeviceConnected(td))
		{
			vr::ETrackedDeviceClass tracked_device_class = globalVRData.vr_context->GetTrackedDeviceClass(td);

			std::cout << "Tracking device " << td << " is connected: ";
			std::cout << "Device type: " << GetTrackedDeviceClassString(tracked_device_class) << ". Name: " << GetTrackedDeviceString(globalVRData.vr_context, td, vr::Prop_TrackingSystemName_String) << std::endl;

			if (tracked_device_class == vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference) globalVRData.base_stations_count++;

			if (td == vr::k_unTrackedDeviceIndex_Hmd)
			{
				// Fill variables used for obtaining the device name and serial ID (used later for naming the SDL window)
				globalVRData.m_strDriver = GetTrackedDeviceString(globalVRData.vr_context, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
				globalVRData.m_strDisplay = GetTrackedDeviceString(globalVRData.vr_context, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

				std::string strWindowTitle = "openvr_main - " + globalVRData.m_strDriver + " " + globalVRData.m_strDisplay;
				SDL_SetWindowTitle(globalOpenglData.mainWindow, strWindowTitle.c_str());
			}
			else
				if (!setup_render_model(td))
					return -1;
		}
	}

	// Check whether both base stations are found
	/*if (globalVRData.base_stations_count < 2)
	{
		std::cout << "There was a problem identifying the base stations, please check they are powered on" << std::endl;
		return -1;
	}*/

	if (!(globalVRData.vr_compositor = vr::VRCompositor()))
	{
		std::cout << "Compositor initialization failed. See log file for details" << std::endl;
		return -1;
	}
	else
		std::cout << "Compositor successfully initialized" << std::endl;

	// Setup left and right eye frame buffers
	globalVRData.vr_context->GetRecommendedRenderTargetSize(&globalVRData.render_width, &globalVRData.render_height);
	std::cout << "Recommended render targer size is " << globalVRData.render_width << "x" << globalVRData.render_height << std::endl;

	std::cout << "Setting up left eye frame buffer..." << std::endl;
	if (!setup_frame_buffer(globalVRData.render_width, globalVRData.render_height, globalVRData.l_frame_buffer, globalVRData.l_render_buffer_depth_stencil, globalVRData.l_tex_color_buffer, globalVRData.l_aux_tex_color_buffer)) return -1;
	std::cout << "Left eye frame buffer setup completed!" << std::endl;

	std::cout << "Setting up right eye frame buffer..." << std::endl;
	if (!setup_frame_buffer(globalVRData.render_width, globalVRData.render_height, globalVRData.r_frame_buffer, globalVRData.r_render_buffer_depth_stencil, globalVRData.r_tex_color_buffer, globalVRData.r_aux_tex_color_buffer)) return -1;
	std::cout << "Right eye frame buffer setup completed!" << std::endl;

	globalVRData.projection_matrix_left = get_projection_matrix(vr::Hmd_Eye::Eye_Left);
	globalVRData.projection_matrix_right = get_projection_matrix(vr::Hmd_Eye::Eye_Right);
	globalVRData.view_matrix_left = get_view_matrix(vr::Hmd_Eye::Eye_Left);
	globalVRData.view_matrix_right = get_view_matrix(vr::Hmd_Eye::Eye_Right);

	return 0;
}

bool ViR2::Init()
{
	globalVRData.vr_context = NULL;
	globalVRData.vr_render_models = NULL;
	globalVRData.vr_compositor = NULL;

	globalOpenglData.w_width = 1116; globalOpenglData.w_height = 1234;
	globalOpenglData.w_pos_x = 100; globalOpenglData.w_pos_y = 100;

	globalVRData.base_stations_count = 0;

	globalVRData.view_matrix_left = globalVRData.view_matrix_right = glm::mat4(1.0);
	globalVRData.projection_matrix_left = globalVRData.projection_matrix_right = glm::mat4(1.0);

	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; nDevice++)
	{
		globalVRData.tracked_device_pose_matrix[nDevice] = glm::mat4(1.0);
	}

	globalVRData.m_strDriver = "No Driver";
	globalVRData.m_strDisplay = "No Display";

	if (initSDL() == -1) return -1;
	if (initOpenGL() == -1) return -1;
#ifndef _NON_VR_VERSION_ONLY
	if (initOpenVR() == -1) return -1;
#else
	globalVRData.projection_matrix_left = glm::perspective( glm::radians(60.0f), float(globalOpenglData.w_width)/float(globalOpenglData.w_height), 0.1f, 100.f );
	globalVRData.view_matrix_left = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-3.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	std::string strWindowTitle = "openvr_main - NON-VR version";
	SDL_SetWindowTitle(globalOpenglData.mainWindow, strWindowTitle.c_str());

	std::cout << "Setting up left eye frame buffer..." << std::endl;
	if (!setup_frame_buffer(globalOpenglData.w_width, globalOpenglData.w_height, globalVRData.l_frame_buffer, globalVRData.l_render_buffer_depth_stencil, globalVRData.l_tex_color_buffer, globalVRData.l_aux_tex_color_buffer)) return -1;
	std::cout << "Left eye frame buffer setup completed!" << std::endl;
#endif
	return true;
}

bool ViR2::SetOpenGLAttributes()
{
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	return true;
}

GLuint ViR2::makeTexture(cv::Mat image) {
	GLuint texture;

	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	cv::flip(image, image, 0); // OpenCV stores top to bottom, but we need the image bottom to top for OpenGL.
	cv::cvtColor(image, image, cv::COLOR_BGR2RGB); // OpenCV uses BGR format, need to convert it to RGB for OpenGL.

	if (image.type() == CV_32FC3) {
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB32F,
			image.cols,
			image.rows,
			0,
			GL_RGB,
			GL_FLOAT,
			image.ptr());
	}
	else if (image.type() == CV_16UC3) {
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB16,
			image.cols,
			image.rows,
			0,
			GL_RGB,
			GL_UNSIGNED_SHORT,
			image.ptr());
	}
	else {
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB8,
			image.cols,
			image.rows,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			image.ptr());
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	ViR2::checkOpenGLErrors();

	return texture;
}

void process_vr_event(const vr::VREvent_t & event)
{
	std::string str_td_class = GetTrackedDeviceClassString(globalVRData.vr_context->GetTrackedDeviceClass(event.trackedDeviceIndex));

	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		// BUGGED
		std::cout << "Device " << event.trackedDeviceIndex << " attached (" << str_td_class << "). Setting up render model" << std::endl;

		// Load render models for the tracking device (when it's powered on duriing application execution)
		setup_render_model(event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		std::cout << "Device " << event.trackedDeviceIndex << " detached (" << str_td_class << "). Removing render model" << std::endl;

		ViR2::RenderObject* render_model = globalVRData.VR_devices[event.trackedDeviceIndex];
		globalVRData.VR_devices[event.trackedDeviceIndex] = NULL;

		delete render_model;
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		std::cout << "Device " << event.trackedDeviceIndex << " updated (" << str_td_class << ")" << std::endl;
	}
	break;
	case vr::VREvent_ButtonPress:
	{
		vr::VREvent_Controller_t controller_data = event.data.controller;
		//std::cout << "Pressed button " << globalVRData.vr_context->GetButtonIdNameFromEnum((vr::EVRButtonId)controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << std::endl;


		vr::VRControllerState_t controller_state;
		vr::TrackedDevicePose_t td_pose;
		if (globalVRData.vr_context->GetControllerStateWithPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, event.trackedDeviceIndex, &controller_state, sizeof(controller_state), &td_pose)) {
			if ((vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_Axis1) & controller_state.ulButtonPressed) != 0) {
				/*std::cout << "Trigger button pressed!" << std::endl;
				std::cout << "Pose information" << std::endl;
				std::cout << "  Tracking result: " << td_pose.eTrackingResult << std::endl;
				std::cout << "  Tracking velocity: (" << td_pose.vVelocity.v[0] << "," << td_pose.vVelocity.v[1] << "," << td_pose.vVelocity.v[2] << ")" << std::endl;*/
			}
		}
	}
	break;
	case vr::VREvent_ButtonUnpress:
	{
		vr::VREvent_Controller_t controller_data = event.data.controller;
		std::cout << "Unpressed button " << globalVRData.vr_context->GetButtonIdNameFromEnum((vr::EVRButtonId)controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << std::endl;
	}
	break;
	case vr::VREvent_ButtonTouch:
	{
		vr::VREvent_Controller_t controller_data = event.data.controller;
		std::cout << "Touched button " << globalVRData.vr_context->GetButtonIdNameFromEnum((vr::EVRButtonId)controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << std::endl;
	}
	break;
	case vr::VREvent_ButtonUntouch:
	{
		vr::VREvent_Controller_t controller_data = event.data.controller;
		std::cout << "Untouched button " << globalVRData.vr_context->GetButtonIdNameFromEnum((vr::EVRButtonId)controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << std::endl;
	}
	break;
	}
}

glm::mat4 convert_SteamVRMat_to_GLMMat(const vr::HmdMatrix34_t &matPose) {

	glm::mat4 pose_matrix = glm::mat4(matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0f,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0f,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0f,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f); 

	return pose_matrix;
}

void ViR2::handleEvents()
{
	if (globalVRData.vr_context != NULL)
	{
		// Process SteamVR events
		vr::VREvent_t vr_event;
		while (globalVRData.vr_context->PollNextEvent(&vr_event, sizeof(vr_event)))
		{
			process_vr_event(vr_event);
		}

		vr::VRCompositor()->WaitGetPoses(globalVRData.tracked_device_pose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; nDevice++)
		{
			if (globalVRData.tracked_device_pose[nDevice].bPoseIsValid)
			{
				globalVRData.tracked_device_pose_matrix[nDevice] = convert_SteamVRMat_to_GLMMat(globalVRData.tracked_device_pose[nDevice].mDeviceToAbsoluteTracking);
			}
		}

		if (globalVRData.tracked_device_pose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			globalVRData.hmd_pose_matrix = globalVRData.tracked_device_pose_matrix[vr::k_unTrackedDeviceIndex_Hmd];
			/*std::cout << globalVRData.hmd_pose_matrix[0][0] << " " << globalVRData.hmd_pose_matrix[0][1] << " " << globalVRData.hmd_pose_matrix[0][2] << " " << globalVRData.hmd_pose_matrix[0][3] << std::endl;
			std::cout << globalVRData.hmd_pose_matrix[1][0] << " " << globalVRData.hmd_pose_matrix[1][1] << " " << globalVRData.hmd_pose_matrix[1][2] << " " << globalVRData.hmd_pose_matrix[1][3] << std::endl;
			std::cout << globalVRData.hmd_pose_matrix[2][0] << " " << globalVRData.hmd_pose_matrix[2][1] << " " << globalVRData.hmd_pose_matrix[2][2] << " " << globalVRData.hmd_pose_matrix[2][3] << std::endl;
			std::cout << globalVRData.hmd_pose_matrix[3][0] << " " << globalVRData.hmd_pose_matrix[3][1] << " " << globalVRData.hmd_pose_matrix[3][2] << " " << globalVRData.hmd_pose_matrix[3][3] << std::endl;
			std::cout << "------" << std::endl;*/
		}
	}

	

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			globalOpenglData.loop = false;

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				globalOpenglData.loop = false;
				break;
			case SDLK_r: {
				// Cover with red and update
				//testingObject.rotate((-20.0f*M_PI)/180.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			case SDLK_g: {
				// Cover with green and update
				//testingObject.rotate((20.0f*M_PI) / 180.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			case SDLK_u: {
				globalVRData.algorithm_selected = (globalVRData.algorithm_selected + 1) % globalVRData.algorithms.size();
				refreshDisplayedTexture();
				break;
			}
			case SDLK_i: {
				globalVRData.style_selected = (globalVRData.style_selected + 1) % globalVRData.style.size();
				refreshDisplayedTexture();
				break;
			}
			case SDLK_o: {
				globalVRData.scene_selected = (globalVRData.scene_selected + 1) % globalVRData.scene.size();
				refreshDisplayedTexture();
				break;
			}
			case SDLK_j: {
				globalVRData.algorithm_selected--;
				if (globalVRData.algorithm_selected < 0) globalVRData.algorithm_selected = globalVRData.algorithms.size() - 1;
				refreshDisplayedTexture();
				break;
			}
			case SDLK_k: {
				globalVRData.style_selected--;
				if (globalVRData.style_selected < 0) globalVRData.style_selected = globalVRData.style.size() - 1;
				refreshDisplayedTexture();
				break;
			}
			case SDLK_l: {
				globalVRData.scene_selected--;
				if (globalVRData.scene_selected < 0) globalVRData.scene_selected = globalVRData.scene.size() - 1;
				refreshDisplayedTexture();
				break;
			}
			case SDLK_b: {
				break;
			}
			case SDLK_SPACE: {
				std::cout << "Saving screenshots..." << std::endl;

				cv::Mat screenshot = ViR2::get_ocv_img_from_gl_img(globalVRData.l_tex_color_buffer);
				cv::imwrite("left" + std::to_string(globalVRData.screenshot_count) + ".png", screenshot);
				screenshot = ViR2::get_ocv_img_from_gl_img(globalVRData.l_aux_tex_color_buffer);
				cv::imwrite("left_" + std::to_string(globalVRData.screenshot_count) + "_disparity.png", screenshot);

				screenshot = ViR2::get_ocv_img_from_gl_img(globalVRData.r_tex_color_buffer);
				cv::imwrite("right_" + std::to_string(globalVRData.screenshot_count) + ".png", screenshot);
				screenshot = ViR2::get_ocv_img_from_gl_img(globalVRData.r_aux_tex_color_buffer);
				cv::imwrite("right_" + std::to_string(globalVRData.screenshot_count) + "_disparity.png", screenshot);

				globalVRData.screenshot_count++;

				break;
			}
			default:
				break;
			}
		}
	}
}


void ViR2::drawVRdevices( glm::mat4 Pmat, glm::mat4 Vmat ) {
	//std::cout << vr::k_unMaxTrackedDeviceCount << std::endl;
	for (int rm_id = 0; rm_id < vr::k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (globalVRData.VR_devices[rm_id] != NULL)
		{
			globalVRData.VR_devices[rm_id]->draw(Pmat, Vmat * glm::inverse(globalVRData.hmd_pose_matrix), globalVRData.tracked_device_pose_matrix[rm_id], glm::mat4(1.0f));
		}
	}
}

void ViR2::submitRenderedToHMD() {
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)globalVRData.l_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)globalVRData.r_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

void ViR2::renderImagesToHMD() {

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)globalVRData.l_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)globalVRData.r_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

void ViR2::renderEyeToScreen() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, globalOpenglData.w_width, globalOpenglData.w_height);
	glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	globalOpenglData.fullscreen.draw(glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f));
}

void ViR2::checkOpenGLErrors() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "OpenGL error! : ";
		std::cout << err << std::endl;
	}
}


void ViR2::Cleanup()
{
	// Delete our OpengL context
	SDL_GL_DeleteContext(globalOpenglData.mainContext);

	// Destroy our window
	SDL_DestroyWindow(globalOpenglData.mainWindow);

	// Shutdown SDL 2
	SDL_Quit();
}

void ViR2::CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();
	}
}

void ViR2::PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}

cv::Mat ViR2::get_ocv_img_from_gl_depth(GLuint ogl_texture_id)
{
	glBindTexture(GL_TEXTURE_2D, ogl_texture_id);
	GLenum gl_texture_width, gl_texture_height;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);

	unsigned char* gl_texture_bytes = (unsigned char*)malloc(sizeof(unsigned char) * gl_texture_width * gl_texture_height);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, gl_texture_bytes);

	cv::Mat image = cv::Mat(gl_texture_height, gl_texture_width, CV_8UC1, gl_texture_bytes);
	cv::flip(image, image, 0); // OpenCV stores top to bottom, but we need the image bottom to top for OpenGL.

	return image;
}

cv::Mat ViR2::get_ocv_img_from_gl_img(GLuint ogl_texture_id)
{
	glBindTexture(GL_TEXTURE_2D, ogl_texture_id);
	GLenum gl_texture_width, gl_texture_height;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);

	unsigned char* gl_texture_bytes = (unsigned char*)malloc(sizeof(unsigned char) * gl_texture_width * gl_texture_height * 3);
	glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);

	cv::Mat image = cv::Mat(gl_texture_height, gl_texture_width, CV_8UC3, gl_texture_bytes);
	cv::flip(image, image, 0); // OpenCV stores top to bottom, but we need the image bottom to top for OpenGL.

	return image;
}

void ViR2::refreshDisplayedTexture() {
	std::cout << "Printing: " << globalVRData.algorithms[globalVRData.algorithm_selected] << " method, scene: " << globalVRData.scene[globalVRData.scene_selected] << ", style: " << globalVRData.style[globalVRData.style_selected] << std::endl;

	// Hijack the buffers
	cv::Mat image_left = cv::imread("screens/" + globalVRData.algorithms[globalVRData.algorithm_selected] + "/out_" + globalVRData.scene[globalVRData.scene_selected] + "_" + globalVRData.style[globalVRData.style_selected] + "/1.png");
	globalVRData.l_tex_color_buffer = ViR2::makeTexture(image_left);
	cv::Mat image_right = cv::imread("screens/" + globalVRData.algorithms[globalVRData.algorithm_selected] + "/out_" + globalVRData.scene[globalVRData.scene_selected] + "_" + globalVRData.style[globalVRData.style_selected] + "/2.png");
	globalVRData.r_tex_color_buffer = ViR2::makeTexture(image_right);
}