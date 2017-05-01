#pragma once
#include <OpenVR/openvr.h>
#include "Matrices.h"
#include "Vectors.h"
#include <vector>

#include "FrameBuffer.h"

#include <glew/glew.h>
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Texture.h"
#include <sdl2-2.0.3/SDL.h>

void ThreadSleep(unsigned long nMilliseconds);
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);


class VrApp
{
public:
	VrApp();
	virtual ~VrApp();


	bool InitVr();

	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t & event);

	void SubmitRender();
	void RenderCompanionWindow();
	void RenderCompanionWindow(GLuint id, int width, int height);
	void RenderTrackedDevices(vr::Hmd_Eye nEye);
	void RenderTrackedDevices(Matrix4 VP, bool renderHeadset);


	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);

	Matrix4 GetTrackedDevicePose(int index);
	bool GetControllerState(unsigned index, vr::VRControllerState_t* state);

	void UpdateHMDMatrixPose();

	void Shutdown();


private:

	bool InitGL();
	bool InitCompositor();

	void SetupRenderModels();


	//void RunMainLoop();


	//bool SetupTexturemaps();

	//void SetupScene();
	//void AddCubeToScene(Matrix4 mat, std::vector<float> &vertdata);
	//void AddCubeVertex(float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata);

	//void RenderControllerAxes();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();
	//bool CreateFrameBuffer(int nWidth, int nHeight, FrameBuffer& framebuffer);

	//void RenderStereoTargets();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	void FindOrLoadRenderModel(const char *pchRenderModelName, Mesh*& a_pMesh, Texture*& a_pTexture);


public:

	int ControllerIndex;

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	RenderBuffer leftEyeBuffer;
	RenderBuffer rightEyeBuffer;

	uint32_t m_unCompanionWindowWidth;
	uint32_t m_unCompanionWindowHeight;

private:
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels * m_pRenderModels;
	
	std::string m_strDriver;
	std::string m_strDisplay;

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class


	//Window Handling
	//-------------------------
	SDL_Window* m_pCompanionWindow;

	SDL_GLContext m_pContext;


	bool m_bVblank;

	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;

	Matrix4 m_mat4HMDPose;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;


	//Not going to be nessisary;
	ShaderProgram m_unSceneProgramID;
	ShaderProgram m_unCompanionWindowProgramID;
	ShaderProgram m_unControllerTransformProgramID;
	ShaderProgram m_unRenderModelProgramID;


	//Device Position and rendering stuff
	Mesh *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];
	Texture *m_rTrackedDeviceTexture[vr::k_unMaxTrackedDeviceCount];

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];

	std::vector<Mesh*> m_vecRenderModels;
	std::vector<Texture*> m_vecRenderTextures;

	Matrix4* m_pControllerMat;
};
