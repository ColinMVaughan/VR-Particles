#pragma once
#include <glew/glew.h>

class FrameBuffer
{
public:
	FrameBuffer(unsigned numColourAttachments);
	~FrameBuffer();

	void InitDepthTexture(unsigned width, unsigned height);
	void InitColorTexture(unsigned index, unsigned width, unsigned height, GLint internalFormat, GLint filter, GLint Wrap);
	void Init1DColorTexture(unsigned index, unsigned size, GLint internalFormat, GLint filter);
	bool CheckFBO();

	//- Clears all opengl memory
	void Unload();

	//- Clears all attached textures
	void Clear();

	void Bind();
	void UnBind();

	void MoveToBackBuffer(int windowWidth, int windowHeight);

	GLuint GetDepthHandle() const;
	GLuint GetColorHandle(unsigned index) const;
	GLuint GetID();

private:

	GLuint _FBO = GL_NONE;
	GLuint _DepthAttachment = GL_NONE;
	GLuint *_ColorAttachments = nullptr;
	GLenum *_Buffs = nullptr;

	unsigned int _NumColorAttachments;
};

class RenderBuffer
{
public:
	RenderBuffer();
	~RenderBuffer();

	bool InitalizeBuffer(int width, int height);

	//void InitDepthTexture(unsigned width, unsigned height);
	//void InitColorTexture(unsigned width, unsigned height);
	bool CheckFBO();

	//- Clears all opengl memory
	//void Unload();

	//- Clears all attached textures
	//void Clear();

	void BindRender();
	void BindResolve();
	void UnBind();



public:

	GLuint m_nDepthBufferId;
	GLuint m_nRenderTextureId;

	GLuint m_nRenderFramebufferId;

	GLuint m_nResolveTextureId;
	GLuint m_nResolveFramebufferId;

};