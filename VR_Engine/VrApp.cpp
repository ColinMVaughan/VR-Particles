#include "VrApp.h"
#include <basetsd.h>
#include <windows.h>
#include "Utilities.h"


void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}

std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


VrApp::VrApp() 
	:m_nRenderWidth(0)
	,m_nRenderHeight(0)
	,m_unCompanionWindowWidth(1920)
	,m_unCompanionWindowHeight(1080)
	,leftEyeBuffer()
	,rightEyeBuffer()
	,m_pHMD(nullptr)
	,m_pRenderModels(nullptr)
	,m_bVblank(false)
	,m_iTrackedControllerCount(0)
	,m_iTrackedControllerCount_Last(-1)
	,m_iValidPoseCount(0)
	,m_iValidPoseCount_Last(-1)
{
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
}

VrApp::~VrApp()
{
}

//-----------------------
// Initalize Vr App
//----------------------
bool VrApp::InitVr()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene); //Get Pointer to HMD manager.

	if(eError != vr::VRInitError_None)
	{
		m_pHMD = nullptr;
		std::cout << "\nCould not init vr runtime: " << vr::VR_GetVRInitErrorAsEnglishDescription;
		return false;
	}

	m_pRenderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if(!m_pRenderModels)
	{
		m_pHMD = nullptr;
		vr::VR_Shutdown();
		std::cout << "\nUnable to get render model interface: ";
		return false;
	}

	///////////////////
	//Gutted some stuff
	/////////////////
	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	m_pCompanionWindow = SDL_CreateWindow("VR_Engine", nWindowPosX, nWindowPosY, m_unCompanionWindowWidth, m_unCompanionWindowHeight, unWindowFlags);
	if (m_pCompanionWindow == NULL)
	{
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
	if (m_pContext == NULL)
	{
		printf("%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if (SDL_GL_SetSwapInterval(m_bVblank ? 1 : 0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	///////////////////////////

	//Not really nessisary, will gut later
	m_strDisplay = "No Display";
	m_strDriver = "No Driver";

	if(!InitGL())	//Will be removing this, most opengl commands will happen outside the wrapper.
	{
		std::cout << "\nUnable to initalize openGl: \n" << __FUNCTION__;
		return false;
	}
	if (!InitCompositor())	//Initalize the compositor to mirror to computer window.
	{
		std::cout << "\nUnable to initalize the Compositor: \n" << __FUNCTION__;
		return false;
	}
	return true;
}

//-----------------------------------------
// Initalizes openGL. I will not end up using this.
//----------------------------------------
bool VrApp::InitGL()
{
	//TODO: check if the shader loaded
	if(!m_unRenderModelProgramID.Load("./assets/shaders/VR/StaticGeometry.vert", "./assets/shaders/VR/UnshadedTexture.frag"))
	{
		std::cout << "\nHeyRetard";
	}


	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();
	SetupRenderModels();


	return true;
}

//---------------------------------
//Initalizes the VR compositor.
//---------------------------------
bool VrApp::InitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if(!vr::VRCompositor())
	{
		std::cout << "\nCompositor Initalization Failed. See log file for details.";
		return false;
	}
	return true;
}

//--------------------------------
//Shuts down the Vr System
//-------------------------------
void VrApp::Shutdown()
{
	if(m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = nullptr;
	}

	for(auto i = m_vecRenderModels.begin(); i!=m_vecRenderModels.end(); ++i)
	{
		delete (*i);
	}
	m_vecRenderModels.clear();

	
}


//------------------------
//Handle vr controller input
//-------------------------
bool VrApp::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		if (sdlEvent.type == SDL_QUIT)
		{
			bRet = true;
		}
	}



	//vr::VREvent_t VREvent;
	//while (m_pHMD->PollNextEvent(&VREvent, sizeof(VREvent))) //Polls the event in the queue
	//{
	//	ProcessVREvent(VREvent); //essesntially just notifies you what the event was
	//}

	//std::cout << "\n---------------------------------------";
	//for(vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; ++unDevice)
	//{
	//	vr::VRControllerState_t state;
	//	if(m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
	//	{
	//		std::cout << "\nPressed" << state.ulButtonPressed;
	//	}
	//}
	return true;
}


//--------------------------------
//Processes & print a single VR event
//-----------------------------
void VrApp::ProcessVREvent(const vr::VREvent_t& event)
{
	switch(event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
		SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		std::cout << "\nDevice" << event.trackedDeviceIndex << " Attached. Setting up render Model";
		break;

	case vr::VREvent_TrackedDeviceDeactivated:
		std::cout << "\nDevice " << event.trackedDeviceIndex << " Detached";
		break;

	case vr::VREvent_TrackedDeviceUpdated:
		std::cout << "\nDevice " << event.trackedDeviceIndex << " updated.";
		break;
	}
}


//Must Be called after 
void VrApp::SubmitRender()
{
	if(m_pHMD)
	{
		//Not Sure About this one...
		vr::Texture_t leftEyeTexture = { (void*)leftEyeBuffer.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::Texture_t rightEyeTexture = { (void*)rightEyeBuffer.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	}

	if (m_bVblank)
	{
		//THIS IS A HACK. Appaerently there is some issue with vsync on nvidia cards
		//and calling glFinish() fixes it for some reason. dont ask me why.
		//Got this info from the openVr samples and doc...
		glFinish();
	}
	////Swap Window (not really)//////
	SDL_GL_SwapWindow(m_pCompanionWindow);

	//clear here so that glFinish waits for the present to complele and not just the command submission.
	//clear here so glFinish will wait for the swap.
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//flush and wait for swap
	if(m_bVblank)
	{
		glFlush();
		glFinish();
	}

	if(m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;

		std::cout << "\nPose Count: " << m_iValidPoseCount<<"("<< m_strPoseClasses.c_str() <<")" <<"Controller Count: "<< m_iTrackedControllerCount;
	}

}


//------------------------------------------------
//Sets up the matrices for each eye of the HMD
//-------------------------------------------------
void VrApp::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);

	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

//---------------------------------------------
//Create Frame Buffer for one eye of the HMD.
//----------------------------------------------
//bool VrApp::CreateFrameBuffer(int nWidth, int nHeight, RenderBuffer& renderbuffer)
//{
//	renderbuffer.InitDepthTexture(0, 0);
//	renderbuffer.InitDepthTexture(0, 0);
//
//	
//	if(renderbuffer.CheckFBO())
//	{
//		std::cout << "\nEye buffer failed to initalize.";
//		return false;
//	}
//
//	return true;
//}


//--------------------------------------------------
//Sets up the framebuffers for each eye of the HMD
//---------------------------------------------------
bool VrApp::SetupStereoRenderTargets()
{

	if (!m_pHMD)
		return false;

	m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

	leftEyeBuffer.InitalizeBuffer(m_nRenderWidth, m_nRenderHeight);
	rightEyeBuffer.InitalizeBuffer(m_nRenderWidth, m_nRenderHeight);

	return true;
}

//-------------------------------------------------
//Sets up window on the computer (Not Nessisary)
//--------------------------------------------------
void VrApp::SetupCompanionWindow()
{
	//Nah
	if (!m_unCompanionWindowProgramID.Load("./assets/shaders/VR/PassThrough.vert", "./assets/shaders/VR/UnshadedTexture.frag"))
	{
		std::cout << "\nFailed to load default shader.";
	}
	InitFullScreenQuad();
}



//--------------------------------------------------------------------
// Ok, so what we are going to keep is the device drawing stuff.
// We are going to get rid of the shader binding, and buffer clearing,
// and keep the rest.
//----------------------------------------------------------------------
void VrApp::RenderTrackedDevices(vr::Hmd_Eye nEye)
{

	//Get View projection matrix and pass it to RenderTrackedDevice
	Matrix4 VP = GetCurrentViewProjectionMatrix(nEye);
	RenderTrackedDevices(VP, false);


}

void VrApp::RenderTrackedDevices(Matrix4 VP, bool RenderHeadset)
{

	uint32_t start;
	if (RenderHeadset)
		start = 0;
	else
		start = 1;


	for (uint32_t unTrackedDevices = start; unTrackedDevices < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevices)
	{
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevices] || !m_rbShowTrackedDevice[unTrackedDevices])
			continue;

		const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevices];
		if (!pose.bPoseIsValid)
			continue;

		//Get Matrix for the device
		const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevices];
		Matrix4 matMVP = VP * matDeviceToTracking;

		m_unRenderModelProgramID.Bind();
		m_unRenderModelProgramID.SendUniformMat4("MVP", matMVP.get(), false);


		//Render Model
		m_rTrackedDeviceTexture[unTrackedDevices]->Bind();

		glBindVertexArray(m_rTrackedDeviceToRenderModel[unTrackedDevices]->VAO);
		//glDrawArrays(GL_TRIANGLES, 0, m_rTrackedDeviceToRenderModel[unTrackedDevices]->GetNumVertices());
		glDrawElements(GL_TRIANGLES, m_rTrackedDeviceToRenderModel[unTrackedDevices]->GetNumVertices(), GL_UNSIGNED_SHORT, 0);

		glBindVertexArray(0);

		m_rTrackedDeviceTexture[unTrackedDevices]->UnBind();
		m_unRenderModelProgramID.UnBind();
	}
}


//----------------------
//Not Nessisary
//----------------------
void VrApp::RenderCompanionWindow()
{
	//Nah.
	//But I thin the 'resolve framebuffer' is used for the window...
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_unCompanionWindowWidth, m_unCompanionWindowHeight);


	m_unCompanionWindowProgramID.Bind();

	glBindTexture(GL_TEXTURE_2D, leftEyeBuffer.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	DrawFullScreenQuad();

	glBindTexture(GL_TEXTURE_2D, 0);
	m_unCompanionWindowProgramID.UnBind();
}

void VrApp::RenderCompanionWindow(GLuint textureID, int width, int height)
{
	//Nah.
	//But I thin the 'resolve framebuffer' is used for the window...
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);

	m_unCompanionWindowProgramID.Bind();

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	DrawFullScreenQuad();

	glBindTexture(GL_TEXTURE_2D, 0);
	m_unCompanionWindowProgramID.UnBind();
}

//-----------------------------------
//Get a projection matrix for nEye
//-----------------------------------
Matrix4 VrApp::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, 0.01f, 1000.0f);
	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);


}

//----------------------------------------------
//Get eye position matrix
//-----------------------------------------------
Matrix4 VrApp::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight= m_pHMD->GetEyeToHeadTransform(nEye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	return matrixObj.invert();
}

//-------------------------------------
// Get MVP matrix of eye
//------------------------------------
Matrix4 VrApp::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
{
	Matrix4 matMVP;
	//Multiply out the matracies for eather eye. may get rid of this to avoid
	//the extra if statement.
	if(nEye == vr::Eye_Left)
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if(nEye == vr::Eye_Right)
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * m_mat4HMDPose;
	}

	return matMVP;
}

Matrix4 VrApp::GetTrackedDevicePose(int index)
{
	m_iTrackedControllerCount = -1;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (m_pHMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;


		m_iTrackedControllerCount += 1;

		if(index != m_iTrackedControllerCount)
			continue;

		if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];
		Matrix4 newMat = mat;

		return newMat;
	}



	return Matrix4();
}

bool VrApp::GetControllerState(unsigned index, vr::VRControllerState_t* state)
{
	int controllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (m_rDevClassChar[unTrackedDevice] != 'C') //if the tracked device is not a controller, continure;
			continue;

		if(index == controllerCount)
		{
			m_pHMD->GetControllerState(unTrackedDevice, state, sizeof(*state));
			return true;
		}

		controllerCount++;
	}

	return false;
	
}

void VrApp::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;

	//get ready to recieve the device poses
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for(int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if(m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				//Get the type of the device.
				switch(m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:			m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:				m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:			m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:		m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference:	m_rDevClassChar[nDevice] = 'T'; break;
				default:										m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	//If the device has a valid pose, get it and invert the matrix. (because of how openVr handles matrices)
	if(m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose.invert();
	}

	//If the device has a valid pose, get it and invert the matrix. (because of how openVr handles matrices)

}

Matrix4 VrApp::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);

	return matrixObj;
}


//TODO: POSSIBLE BUG IF WE FIND A LOADED RENDER MODEL BUT NOT A RENDER TEXTURE
void VrApp::FindOrLoadRenderModel(const char* pchRenderModelName, Mesh*& a_pMesh, Texture*& a_pTexture)
{
	 
	//Iterate through all loaded models and compare names.
	//If we have found the model we are looking for: break. otherwise continue.
	for(auto i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); ++i)
	{
		if(!strcmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			a_pMesh = *i;
			break;
		}
	}

	//if we didnt find an existing model, Load a new one.
	if(!a_pMesh)
	{
		a_pMesh = new Mesh();
		a_pTexture = new Texture();

		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError eError;

		//We are loading the mesh asyncronusly, this is why we are in an enless loop.
		//The loop will break when the model has finished loading.
		while(1) //fuck that.
		{
			eError = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (eError != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1); //sleep for a milisecond so we arent looping at mach-10 unessisaraly.
		}

		//if we could not load anything, immediatly return null.
		if(eError != vr::VRRenderModelError_None)
		{
			std::cout << "\nUnable to load render model.";
		}


		//Texture also loaded asyncronusly
		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			eError = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (eError != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if(eError != vr::VRRenderModelError_None)
		{
			std::cout << "\nUnable to load render texture.";
			vr::VRRenderModels()->FreeRenderModel(pModel);
		}

		a_pMesh->LoadFromRenderModel(*pModel);
		a_pTexture->LoadFromRenderModel(*pTexture);
		
		m_vecRenderModels.push_back(a_pMesh);
		m_vecRenderTextures.push_back(a_pTexture);

		//Free openVR data once we have copied it.
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);

	}
	
}

void VrApp::SetupRenderModels()
{
	memset(m_rTrackedDeviceToRenderModel, 0, sizeof(m_rTrackedDeviceToRenderModel));
	memset(m_rTrackedDeviceTexture, 0, sizeof(m_rTrackedDeviceTexture));

	if (!m_pHMD)
		return;

	for(uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}
}

void VrApp::SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	Mesh* pRenderModel = nullptr;
	Texture* pRenderTexture = nullptr;

	std::string sRenderModelName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	FindOrLoadRenderModel(sRenderModelName.c_str(), pRenderModel, pRenderTexture);
	if(!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		std::cout << "Unable to load render model for tracked device:";
	}
	else
	{
		m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
		m_rTrackedDeviceTexture[unTrackedDeviceIndex] = pRenderTexture;
		m_rbShowTrackedDevice[unTrackedDeviceIndex] = true;
	}
}
