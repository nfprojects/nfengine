/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations for Renderer utility functions used by Scenes
 */

#pragma once

#include <string>

bool InitRenderer(const std::string& renderer);
bool InitRenderTarget(void* winHandle); // RT should be outside scenes, needs winHandle
bool InitBlendState(bool enabled); // turning BlendState off is not so easy on scenes

void ReleaseResources();
void ReleaseBlendState();
void ReleaseRenderTarget();
void ReleaseRenderer();

/// Helper creators for Scenes
bool CreateShaderProgram(bool useCBuffer, bool useTexture);
bool CreateVertexBuffer(bool withExtraVert);
bool CreateIndexBuffer();
bool CreateConstantBuffer();
bool CreateTexture();

/// Draw commands
void PreDrawLoop();
void Draw();
