#include <ViR2_opengl.hpp>

GlobalOpenglData globalOpenglData = GlobalOpenglData(); // Init OpenGL global data structure
GlobalVRData globalVRData = GlobalVRData(); // Init OpenGL global data structure

int w_width, w_height, w_pos_x, w_pos_y;

vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];
glm::mat4 tracked_device_pose_matrix[vr::k_unMaxTrackedDeviceCount];

ViR2::Shader testingShader, passthroughShader, skyboxShader;
ViR2::MeshGeometry testingGeometry, fullscreenQuad;

ViR2::MeshGeometry VR_devices_models[vr::k_unMaxTrackedDeviceCount];

ViR2::RenderObject testingObject;
ViR2::FullScreenQuadObject fullscreen;
ViR2::SkyboxObject skybox;

const float screenCoords[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
};


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

	VR_devices_models[tracked_device] = ViR2::MeshGeometry(model, rm_texture, &testingShader);
	globalVRData.VR_devices[tracked_device] = new ViR2::RenderObject(&VR_devices_models[tracked_device], &testingShader);


	vr::VRRenderModels()->FreeRenderModel(model);
	vr::VRRenderModels()->FreeTexture(rm_texture);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "Error " << err << " with render model " << render_model_name.c_str() << std::endl;
	}

	return true;

}

bool setup_frame_buffer(GLsizei width, GLsizei height, GLuint &frame_buffer, GLuint &render_buffer_depth_stencil, GLuint &tex_color_buffer) {



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

	// Create the render buffer to host the depth and stencil buffers
	// Although we could do this by creating another texture, it is more efficient to store these buffers in a Renderbuffer Object, because we're only interested in reading the color buffer in a shader
	glGenRenderbuffers(1, &render_buffer_depth_stencil);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_depth_stencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Attach the render buffer to the framebuffer
	// NOTE: Even if you don't plan on reading from this depth_attachment, an off screen buffer that will be rendered to should have a depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_depth_stencil);

	// Check whether the frame buffer is complete (at least one buffer attached, all attachmentes are complete, all attachments same number multisamples)
	(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? std::cout << "The frame buffer is complete!" << std::endl : std::cout << "The frame buffer is invalid, please re-check. Code: " << glCheckFramebufferStatus(frame_buffer) << std::endl;

	return true;
}

glm::mat4 get_projection_matrix(const vr::Hmd_Eye eye)
{
	vr::HmdMatrix44_t steamvr_proj_matrix = globalVRData.vr_context->GetProjectionMatrix(eye, 0.1f, 15.f);

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

	globalOpenglData.mainWindow = SDL_CreateWindow("Hello world VR", w_pos_x, w_pos_y, w_width, w_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

	glViewport(0, 0, w_width, w_height);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	{

		std::cout << "Initializing shaders" << std::endl;
		testingShader = ViR2::Shader("shader/shader.vert","shader/shader.frag");
		passthroughShader = ViR2::Shader("shader/pass_tex.vert", "shader/pass_tex.frag");
		skyboxShader = ViR2::Shader("shader/skybox.vert", "shader/skybox.frag");

		testingShader.init();
		passthroughShader.init();
		skyboxShader.init();

		testingGeometry = ViR2::MeshGeometry("models/teddy.obj", &testingShader);
		fullscreenQuad = ViR2::MeshGeometry(screenCoords, sizeof(screenCoords), &passthroughShader);

		testingObject = ViR2::RenderObject(&testingGeometry, &testingShader);
		fullscreen = ViR2::FullScreenQuadObject(&fullscreenQuad, &passthroughShader, &(globalVRData.l_tex_color_buffer) );
		skybox = ViR2::SkyboxObject(&fullscreenQuad, &skyboxShader, "data/skybox/nightsky");

		testingObject.move(glm::vec3(-3.0f, 1.0f, 0.0f));

	}

	return success;
}

int initOpenVR() {

	if (vr::VR_IsHmdPresent())
	{
		std::cout << "An HMD was successfully found in the system" << std::endl;

		if (vr::VR_IsRuntimeInstalled()) {
			const char* runtime_path = vr::VR_RuntimePath();
			std::cout << "Runtime correctly installed at '" << runtime_path << "'" << std::endl;
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
	if (globalVRData.base_stations_count < 2)
	{
		std::cout << "There was a problem indentifying the base stations, please check they are powered on" << std::endl;
		return -1;
	}

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
	if (!setup_frame_buffer(globalVRData.render_width, globalVRData.render_height, globalVRData.l_frame_buffer, globalVRData.l_render_buffer_depth_stencil, globalVRData.l_tex_color_buffer)) return -1;
	std::cout << "Left eye frame buffer setup completed!" << std::endl;

	std::cout << "Setting up right eye frame buffer..." << std::endl;
	if (!setup_frame_buffer(globalVRData.render_width, globalVRData.render_height, globalVRData.r_frame_buffer, globalVRData.r_render_buffer_depth_stencil, globalVRData.r_tex_color_buffer)) return -1;
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

	w_width = 1200; w_height = 900;
	w_pos_x = 100; w_pos_y = 100;

	globalVRData.base_stations_count = 0;

	globalVRData.view_matrix_left = globalVRData.view_matrix_right = glm::mat4(1.0);
	globalVRData.projection_matrix_left = globalVRData.projection_matrix_right = glm::mat4(1.0);

	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; nDevice++)
	{
		tracked_device_pose_matrix[nDevice] = glm::mat4(1.0);
	}

	globalVRData.m_strDriver = "No Driver";
	globalVRData.m_strDisplay = "No Display";

	if (initSDL() == -1) return -1;
	if (initOpenGL() == -1) return -1;
#ifndef _NON_VR_VERSION_ONLY
	if (initOpenVR() == -1) return -1;
#else
	globalVRData.projection_matrix_left = glm::perspective( glm::radians(60.0f), float(w_width)/float(w_height), 0.1f, 100.f );
	globalVRData.view_matrix_left = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-3.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));;
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
		std::cout << "Pressed button " << globalVRData.vr_context->GetButtonIdNameFromEnum((vr::EVRButtonId)controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << std::endl;


		vr::VRControllerState_t controller_state;
		vr::TrackedDevicePose_t td_pose;
		if (globalVRData.vr_context->GetControllerStateWithPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, event.trackedDeviceIndex, &controller_state, sizeof(controller_state), &td_pose)) {
			if ((vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_Axis1) & controller_state.ulButtonPressed) != 0) {
				std::cout << "Trigger button pressed!" << std::endl;
				std::cout << "Pose information" << std::endl;
				std::cout << "  Tracking result: " << td_pose.eTrackingResult << std::endl;
				std::cout << "  Tracking velocity: (" << td_pose.vVelocity.v[0] << "," << td_pose.vVelocity.v[1] << "," << td_pose.vVelocity.v[2] << ")" << std::endl;
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

		vr::VRCompositor()->WaitGetPoses(tracked_device_pose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; nDevice++)
		{
			if (tracked_device_pose[nDevice].bPoseIsValid)
			{
				tracked_device_pose_matrix[nDevice] = convert_SteamVRMat_to_GLMMat(tracked_device_pose[nDevice].mDeviceToAbsoluteTracking);
			}
		}

		if (tracked_device_pose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			globalVRData.hmd_pose_matrix = tracked_device_pose_matrix[vr::k_unTrackedDeviceIndex_Hmd];
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
				testingObject.rotate((-20.0f*M_PI)/180.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			case SDLK_g: {
				// Cover with green and update
				testingObject.rotate((20.0f*M_PI) / 180.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			case SDLK_b: {
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			default:
				break;
			}
		}
	}
}

void ViR2::draw()
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

	skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
	testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));

	//glDisable(GL_DEPTH_TEST);
	for (int rm_id=0; rm_id<vr::k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (globalVRData.VR_devices[rm_id] != NULL)
		{
			globalVRData.VR_devices[rm_id]->draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left * glm::inverse(globalVRData.hmd_pose_matrix), tracked_device_pose_matrix[rm_id]);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//////////////////////////////////////////////
	// Render right eye...

	glBindFramebuffer(GL_FRAMEBUFFER, globalVRData.r_frame_buffer);
	glViewport(0, 0, globalVRData.render_width, globalVRData.render_height);
	glEnable(GL_DEPTH_TEST);

	// Make our background white
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));
	testingObject.draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), glm::mat4(1.0f));

	//glDisable(GL_DEPTH_TEST);
	for (int rm_id = 0; rm_id < vr::k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (globalVRData.VR_devices[rm_id] != NULL)
		{
			globalVRData.VR_devices[rm_id]->draw(globalVRData.projection_matrix_right, globalVRData.view_matrix_right * glm::inverse(globalVRData.hmd_pose_matrix), tracked_device_pose_matrix[rm_id]);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Submit render buffers to HMD

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)globalVRData.l_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)globalVRData.r_tex_color_buffer, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	/////////////////////////////////////////////
	// Render what companion window will show
	// In this case we render what left eye is actually seeing
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, w_width, w_height);
	glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	fullscreen.draw(glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f));

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << err << std::endl;
	}

#else 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w_width, w_height);
	glEnable(GL_DEPTH_TEST);

	// Make our background white
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f));
	testingObject.draw(globalVRData.projection_matrix_left, globalVRData.view_matrix_left, glm::mat4(1.0f));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

	// Swap our buffers to make our changes visible
	SDL_GL_SwapWindow( globalOpenglData.mainWindow );
	
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