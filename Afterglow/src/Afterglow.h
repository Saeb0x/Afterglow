#pragma once

// ------------ Entry Point ------------
#include "Afterglow/Core/EntryPoint.h"
// -------------------------------------

#include "Afterglow/Core/Base.h"
#include "Afterglow/Core/Timestep.h"

#include "Afterglow/Core/Application.h"
#include "Afterglow/Core/Layer.h"

#include "Afterglow/Core/Input.h"
#include "Afterglow/Core/Events/Event.h"
#include "Afterglow/Core/Events/InputEvents.h"
#include "Afterglow/Core/Events/WindowEvents.h"
#include "Afterglow/Core/OrthographicCameraController.h"

// ------------ Renderer ---------------
#include "Afterglow/Core/Renderer/Renderer.h"
#include "Afterglow/Core/Renderer/Renderer2D.h"
#include "Afterglow/Core/Renderer/RenderCommand.h"

#include "Afterglow/Core/Renderer/VertexArray.h"
#include "Afterglow/Core/Renderer/Buffer.h"
#include "Afterglow/Core/Renderer/Shader.h"

#include "Afterglow/Core/Renderer/Texture.h"
// -------------------------------------

#ifdef AG_NETWORKING
#include "Afterglow/Networking/HTTP/HttpClient.h"
#endif