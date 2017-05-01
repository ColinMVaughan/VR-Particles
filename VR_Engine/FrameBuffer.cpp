#include "FrameBuffer.h"


FrameBuffer::FrameBuffer(unsigned numColourAttachments)
{
	_NumColorAttachments = numColourAttachments;
	glGenFramebuffers(1, &_FBO);
	_ColorAttachments = new GLuint[_NumColorAttachments];

	_Buffs = new GLenum[_NumColorAttachments];
	for (int i = 0; i < _NumColorAttachments; i++)
	{
		_Buffs[i] = GL_COLOR_ATTACHMENT0 + i;
	}

}

FrameBuffer::~FrameBuffer()
{
	Unload();
}


void FrameBuffer::InitDepthTexture(unsigned width, unsigned height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	//Create DeapthTexture
	glGenTextures(1, &_DepthAttachment);
	glBindTexture(GL_TEXTURE_2D, _DepthAttachment);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	//Bind texture to fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthAttachment, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);


}

void FrameBuffer::InitColorTexture(unsigned index, unsigned width, unsigned height, GLint internalFormat, GLint filter, GLint Wrap)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	//Create DeapthTexture
	glGenTextures(1, &_ColorAttachments[index]);
	glBindTexture(GL_TEXTURE_2D, _ColorAttachments[index]);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap);


	//Bind texture to fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, _ColorAttachments[index], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

//---------------------------------
//TODO: Make function more generic
//----------------------------------
void FrameBuffer::Init1DColorTexture(unsigned index, unsigned size, GLint internalFormat, GLint filter)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	glGenTextures(1, &_ColorAttachments[index]);
	glBindTexture(GL_TEXTURE_1D, _ColorAttachments[index]);

	glTexStorage1D(GL_TEXTURE_1D, 1, internalFormat, size);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, filter);
	
	glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_1D, _ColorAttachments[index], 0);

	glBindTexture(GL_TEXTURE_1D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

bool FrameBuffer::CheckFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Unload();
		return false;
	}

	return true;

}

void FrameBuffer::Unload()
{
	if (_Buffs != nullptr)
	{
		delete[] _Buffs;
		_Buffs = nullptr;
	}

	if (_ColorAttachments != nullptr)
	{
		for (int i = 0; i < _NumColorAttachments; i++)
		{
			glDeleteTextures(1, &_ColorAttachments[i]);
		}

		delete[] _ColorAttachments;
		_ColorAttachments = nullptr;
	}

	if (_DepthAttachment != GL_NONE)
	{
		glDeleteTextures(1, &_DepthAttachment);
		_DepthAttachment = GL_NONE;
	}

	_NumColorAttachments = 0;
}


void FrameBuffer::Clear()
{
	GLbitfield temp = 0;

	if (_DepthAttachment != GL_NONE)
	{
		temp = temp | GL_DEPTH_BUFFER_BIT;
	}

	if (_ColorAttachments != nullptr)
	{
		temp = temp | GL_COLOR_BUFFER_BIT;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
	glClear(temp);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
	glDrawBuffers(_NumColorAttachments, _Buffs);
}

void FrameBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::MoveToBackBuffer(int windowWidth, int windowHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);

	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

GLuint FrameBuffer::GetDepthHandle() const
{
	return _DepthAttachment;
}

GLuint FrameBuffer::GetColorHandle(unsigned index) const
{
	return _ColorAttachments[index];
}

GLuint FrameBuffer::GetID()
{
	return _FBO;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
RenderBuffer::RenderBuffer()
{

}

RenderBuffer::~RenderBuffer()
{
}

bool RenderBuffer::InitalizeBuffer(int width, int height)
{

	glGenFramebuffers(1, &m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);

	glGenRenderbuffers(1, &m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_nDepthBufferId);

	glGenTextures(1, &m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_nRenderTextureId, 0);

	glGenFramebuffers(1, &m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_nResolveFramebufferId);

	glGenTextures(1, &m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nResolveTextureId, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}


//void RenderBuffer::InitDepthTexture(unsigned width, unsigned height)
//{
//	glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
//
//	glGenRenderbuffers(1, &m_nDepthBufferId);
//	glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthBufferId);
//	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_nDepthBufferId);
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
//
//void RenderBuffer::InitColorTexture(unsigned width, unsigned height)
//{
//	glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
//
//	glGenTextures(1, &m_nRenderTextureId);
//	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_nRenderTextureId);
//	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_nRenderTextureId);
//
//
//
//	glBindFramebuffer(GL_FRAMEBUFFER, m_nResolveFramebufferId);
//
//	glGenTextures(1, &m_nResolveTextureId);
//	glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nResolveTextureId, 0);
//
//	glBindBuffer(GL_FRAMEBUFFER, 0);
//
//}

bool RenderBuffer::CheckFBO()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_nResolveTextureId);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	return true;
}

void RenderBuffer::BindRender()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
}

void RenderBuffer::BindResolve()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_nResolveFramebufferId);
}

void RenderBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}