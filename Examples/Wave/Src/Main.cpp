#include <Afterglow.h>

#define NOMINMAX
#include <Windows.h>
#include <glad/glad.h>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

static constexpr float k_Pi = 3.14159265358979f;
static constexpr int k_GridN = 60;					// Number of quads per side
static constexpr float k_GridSpacing = 0.15f;		// World-space distance between vertices

static const char* k_VertSrc = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;
uniform float uAmplitude;
uniform float uFrequency;
uniform int uMode;			// 0 = flat, 1 = single-source wave, 2 = two-source interference
uniform vec2 uSource1;
uniform vec2 uSource2;

out vec3 vWorldPos;

void main()
{
    vec3 pos = aPos;

    if (uMode >= 1)
    {
        float d1 = length(pos.xz - uSource1);
        float wave = uAmplitude * sin(uFrequency * d1 - uTime * 2.5);

        if (uMode == 2)
        {
            float d2 = length(pos.xz - uSource2);
            wave += uAmplitude * sin(uFrequency * d2 - uTime * 2.5);
        }

        pos.y = wave;
    }

    vWorldPos = pos;
    gl_Position = uProj * uView * vec4(pos, 1.0);
}
)";

static const char* k_FragSrc = R"(
#version 330 core

in vec3 vWorldPos;

uniform int uFillMode;		// 0 = wireframe (height-tinted), 1 = solid (screen-space normals)
uniform float uAmplitude;

out vec4 FragColor;

void main()
{
    float t = clamp((vWorldPos.y / max(uAmplitude * 2.0, 0.001)) * 0.5 + 0.5, 0.0, 1.0);

    if (uFillMode == 0)
    {
        FragColor = vec4(mix(vec3(0.1, 0.3, 0.8), vec3(0.4, 0.9, 1.0), t), 1.0);
    }
    else
    {
        // Compute surface normal from screen-space derivatives - no normals stored in the VBO
        vec3 dx = dFdx(vWorldPos);
        vec3 dy = dFdy(vWorldPos);
        vec3 normal = normalize(cross(dx, dy));

        vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
        float diff = max(dot(normal, lightDir), 0.0);
        float ambient = 0.15;

        vec3 color = mix(vec3(0.05, 0.3, 0.7), vec3(0.3, 0.8, 1.0), t);
        FragColor = vec4(color * (ambient + diff * 0.85), 1.0);
    }
}
)";

enum class Stage
{
	FlatGrid,
	Construction,
	WaveWireframe,
	SolidFill,
	Interference,
	Count
};

static const char* StageName(Stage stage)
{
	switch (stage)
	{
		case Stage::FlatGrid:		return "Flat Grid";
		case Stage::Construction:	return "Construction";
		case Stage::WaveWireframe:	return "Wave (Wireframe)";
		case Stage::SolidFill:		return "Wave (Solid Fill)";
		case Stage::Interference:	return "Interference";
		default:					return "Unknown";
	}
}

// Minimal column-major 4x4 matrix (matches GLSL layout)
// NOTE(saeb): Afterglow should provide a math library - this is a local stand-in
struct Mat4
{
	float m[16] = {};

	static Mat4 Identity()
	{
		Mat4 r;
		r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;

		return r;
	}

	static Mat4 Perspective(float fovYRad, float aspect, float zNear, float zFar)
	{
		Mat4  r;

		float f = 1.0f / tanf(fovYRad * 0.5f);
		float rz = zFar - zNear;

		r.m[0] = f / aspect;
		r.m[5] = f;
		r.m[10] = -(zFar + zNear) / rz;
		r.m[11] = -1.0f;
		r.m[14] = -(2.0f * zFar * zNear) / rz;

		return r;
	}

	static Mat4 LookAt(float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy, float uz)
	{
		// Forward
		float fx = cx - ex, fy = cy - ey, fz = cz - ez;
		float fl = sqrtf(fx*fx + fy*fy + fz*fz);
		fx /= fl; fy /= fl; fz /= fl;

		// Right = forward x up
		float rx = fy*uz - fz*uy, ry = fz*ux - fx*uz, rz_v = fx*uy - fy*ux;
		float rl = sqrtf(rx*rx + ry*ry + rz_v*rz_v);
		rx /= rl; ry /= rl; rz_v /= rl;

		// Up = right z forward
		float vx = ry*fz - rz_v*fy, vy = rz_v*fx - rx*fz, vz = rx*fy - ry*fx;

		Mat4 r;
		r.m[0] = rx; r.m[4] = ry; r.m[8] = rz_v; r.m[12] = -(rx*ex + ry*ey + rz_v*ez);
		r.m[1] = vx; r.m[5] = vy; r.m[9] = vz; r.m[13] = -(vx*ex + vy*ey + vz*ez);
		r.m[2] = -fx; r.m[6] = -fy; r.m[10] = -fz; r.m[14] = (fx*ex + fy*ey + fz*ez);
		r.m[15] = 1.0f;

		return r;
	}
};

class Wave : public Afterglow::Application
{
public:
	Wave() = default;
	~Wave() override = default;

	Afterglow::LoggerConfig GetLoggerConfig() override
	{
		Afterglow::LoggerConfig config;
		config.ClientLoggerName = "WAVE";
		config.ClientLogFileName = "Afterglow-Wave.log";
		
		return config;
	}

	Afterglow::WindowConfig GetWindowConfig() override
	{
		Afterglow::WindowConfig config;
		config.Title = "Wave - Afterglow";
		config.Width = 1600;
		config.Height = 900;
		
		return config;
	}

	void OnInit() override
	{
		// NOTE(saeb): Afterglow should expose delta time through OnUpdate - using QPC directly as a workaround
		QueryPerformanceFrequency(&m_QPCFrequency);
		QueryPerformanceCounter(&m_LastTime);

		CompileShaders();
		BuildMesh();
		CreateMSAAFramebuffer();
		InitHUDFont();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		EnterStage(Stage::FlatGrid);
	}

	void OnUpdate() override
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		float dt = static_cast<float>(now.QuadPart - m_LastTime.QuadPart) / static_cast<float>(m_QPCFrequency.QuadPart);
		m_LastTime = now;

		// Cap dt to avoid spiral of death when window loses focus
		dt = std::min(dt, 0.05f);

		HandleInput(dt);

		if (!m_Paused)
			m_Time += dt;

		// Construction stage: reveal quads outward from center
		if (m_Stage == Stage::Construction)
			m_DrawCount = std::min(m_DrawCount + 4 * 6, m_TotalIndexCount);

		// WaveWireframe: ramp amplitude up from zero
		if (m_Stage == Stage::WaveWireframe)
		{
			m_Amplitude = std::min(m_Amplitude + dt * 0.3f, 0.6f);
			glUseProgram(m_Program);
			glUniform1f(glGetUniformLocation(m_Program, "uAmplitude"), m_Amplitude);
		}

		UpdateCamera(dt);
		RenderScene();
		RenderHUD();
	}

	void OnShutdown() override
	{
		DeleteMSAAFramebuffer();

		if (m_FontListBase)
			glDeleteLists(m_FontListBase, 96);

		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
		glDeleteProgram(m_Program);
	}

private:
	void EnterStage(Stage stage)
	{
		m_Stage = stage;

		switch (stage)
		{
			case Stage::FlatGrid:
			{
				m_DrawCount = m_TotalIndexCount;
				SetUniforms(0, 0, 0.0f);
				break;
			}
			case Stage::Construction:
			{
				m_DrawCount = 0;
				SetUniforms(0, 0, 0.0f);
				break;
			}
			case Stage::WaveWireframe:
			{
				m_DrawCount = m_TotalIndexCount;
				m_Amplitude = 0.0f;
				SetUniforms(1, 0, 0.0f);
				break;
			}
			case Stage::SolidFill:
			{
				m_DrawCount = m_TotalIndexCount;
				SetUniforms(1, 1, m_Amplitude);
				break;
			}
			case Stage::Interference:
			{
				m_DrawCount = m_TotalIndexCount;
				SetUniforms(2, 1, m_Amplitude);
				break;
			}
			default:
				break;
		}
	}

	void SetUniforms(int mode, int fillMode, float amplitude)
	{
		glUseProgram(m_Program);
		glUniform1i(glGetUniformLocation(m_Program, "uMode"), mode);
		glUniform1i(glGetUniformLocation(m_Program, "uFillMode"), fillMode);
		glUniform1f(glGetUniformLocation(m_Program, "uAmplitude"), amplitude);
		glUniform1f(glGetUniformLocation(m_Program, "uFrequency"), m_Frequency);
		glUniform2f(glGetUniformLocation(m_Program, "uSource1"), m_Source1[0], m_Source1[1]);
		glUniform2f(glGetUniformLocation(m_Program, "uSource2"), m_Source2[0], m_Source2[1]);
	}

	void HandleInput(float dt)
	{
		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::Escape))
			Close();

		// Stage navigation
		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::N))
		{
			int next = static_cast<int>(m_Stage) + 1;

			if (next < static_cast<int>(Stage::Count))
				EnterStage(static_cast<Stage>(next));
		}

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::B))
		{
			int prev = static_cast<int>(m_Stage) - 1;

			if (prev >= 0)
				EnterStage(static_cast<Stage>(prev));
		}

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::P))
			m_Paused = !m_Paused;

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::O))
			m_AutoOrbit = !m_AutoOrbit;

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::M))
			m_MSAAEnabled = !m_MSAAEnabled;

		// Wireframe overlay (meaningful only on solid stages)
		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::Tab))
			m_WireframeOverlay = !m_WireframeOverlay;

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::V))
			GetWindow().SetVSync(!GetWindow().IsVSync());
	}

	void UpdateCamera(float dt)
	{
		if (m_AutoOrbit)
			m_Azimuth += 0.3f * dt;

		if (Afterglow::Input::IsKeyDown(Afterglow::Key::Left))
			m_Azimuth -= 1.5f * dt;
		if (Afterglow::Input::IsKeyDown(Afterglow::Key::Right))
			m_Azimuth += 1.5f * dt;
		if (Afterglow::Input::IsKeyDown(Afterglow::Key::Up))
			m_Elevation = std::min(m_Elevation + 1.0f * dt, k_Pi * 0.48f);
		if (Afterglow::Input::IsKeyDown(Afterglow::Key::Down))
			m_Elevation = std::max(m_Elevation - 1.0f * dt, 0.05f);
	}

	Mat4 GetViewMatrix() const
	{
		float x = m_Radius * cosf(m_Elevation) * cosf(m_Azimuth);
		float y = m_Radius * sinf(m_Elevation);
		float z = m_Radius * cosf(m_Elevation) * sinf(m_Azimuth);

		return Mat4::LookAt(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	}

	void RenderScene()
	{
		uint32_t w = GetWindow().GetWidth();
		uint32_t h = GetWindow().GetHeight();

		if (m_MSAAEnabled)
			glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

		glViewport(0, 0, w, h);
		Afterglow::RenderCommand::SetClearColor(0.05f, 0.05f, 0.08f, 1.0f);
		Afterglow::RenderCommand::Clear();

		glUseProgram(m_Program);

		// Update per-frame uniforms
		glUniform1f(glGetUniformLocation(m_Program, "uTime"), m_Time);
		glUniform1f(glGetUniformLocation(m_Program, "uAmplitude"), m_Amplitude);

		float aspect = static_cast<float>(w) / static_cast<float>(h);
		Mat4 proj = Mat4::Perspective(k_Pi / 4.0f, aspect, 0.1f, 100.0f);
		Mat4 view = GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(m_Program, "uProj"), 1, GL_FALSE, proj.m);
		glUniformMatrix4fv(glGetUniformLocation(m_Program, "uView"), 1, GL_FALSE, view.m);

		glBindVertexArray(m_VAO);

		bool solidStage = (m_Stage == Stage::SolidFill || m_Stage == Stage::Interference);

		if (solidStage)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES, m_DrawCount, GL_UNSIGNED_INT, nullptr);

			if (m_WireframeOverlay)
			{
				// Draw wireframe on top with a depth offset to avoid z-fighting
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0f, -1.0f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glUniform1i(glGetUniformLocation(m_Program, "uFillMode"), 0);
				glDrawElements(GL_TRIANGLES, m_DrawCount, GL_UNSIGNED_INT, nullptr);
				glDisable(GL_POLYGON_OFFSET_LINE);
				glUniform1i(glGetUniformLocation(m_Program, "uFillMode"), 1);
			}
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, m_DrawCount, GL_UNSIGNED_INT, nullptr);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(0);

		if (m_MSAAEnabled)
		{
			// Resolve MSAA into the default framebuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void RenderHUD()
	{
		if (!m_FontReady)
			return;

		uint32_t w = GetWindow().GetWidth();
		uint32_t h = GetWindow().GetHeight();

		// Switch to fixed-function 2D projection for overlay text
		// NOTE(saeb): Afterglow should provide a text/HUD rendering API to avoid direct fixed-function calls here
		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, static_cast<double>(w), 0.0, static_cast<double>(h), -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		auto drawText = [&](int x, int y, const std::string& text)
		{
			glRasterPos2i(x, y);
			glListBase(m_FontListBase - 32);
			glCallLists(static_cast<GLsizei>(text.size()), GL_UNSIGNED_BYTE, text.c_str());
		};

		int top = static_cast<int>(h) - 22;

		glColor3f(0.9f, 0.9f, 0.9f);
		drawText(10, top, std::string("Stage:  ") + StageName(m_Stage));
		drawText(10, top - 20, std::string("MSAA:   ") + (m_MSAAEnabled ? "On (4x)" : "Off") + "  [M]");
		drawText(10, top - 40, std::string("Orbit:  ") + (m_AutoOrbit ? "Auto" : "Manual") + "  [O]");
		drawText(10, top - 60, std::string("Paused: ") + (m_Paused ? "Yes" : "No") + "  [P]");

		glColor3f(0.45f, 0.45f, 0.45f);
		drawText(10, 10, "N: Next   B: Back   Arrows: Camera   Tab: Wireframe overlay   V: VSync   Esc: Quit");

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glEnable(GL_DEPTH_TEST);
	}

	void CompileShaders()
	{
		auto compile = [](GLenum type, const char* src) -> GLuint
		{
			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &src, nullptr);
			glCompileShader(shader);

			GLint ok;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
			if (!ok)
			{
				char log[512];
				glGetShaderInfoLog(shader, 512, nullptr, log);
				AG_LOG_ERROR("Shader compile error: {}", log);
			}

			return shader;
		};

		GLuint vert = compile(GL_VERTEX_SHADER,   k_VertSrc);
		GLuint frag = compile(GL_FRAGMENT_SHADER, k_FragSrc);

		m_Program = glCreateProgram();
		glAttachShader(m_Program, vert);
		glAttachShader(m_Program, frag);
		glLinkProgram(m_Program);

		GLint ok;
		glGetProgramiv(m_Program, GL_LINK_STATUS, &ok);
		if (!ok)
		{
			char log[512];
			glGetProgramInfoLog(m_Program, 512, nullptr, log);
			AG_LOG_ERROR("Shader link error: {}", log);
		}

		glDeleteShader(vert);
		glDeleteShader(frag);
	}

	void BuildMesh()
	{
		// Build a flat k_GridN x k_GridN grid centered at the origin
		int vN = k_GridN + 1;
		float half = k_GridN * k_GridSpacing * 0.5f;

		std::vector<float> vertices;
		vertices.reserve(vN * vN * 3);

		for (int z = 0; z < vN; ++z)
		for (int x = 0; x < vN; ++x)
		{
			vertices.push_back(x * k_GridSpacing - half);
			vertices.push_back(0.0f);
			vertices.push_back(z * k_GridSpacing - half);
		}

		// Tag each quad with its distance from the grid center, then sort so quads are ordered by ring - the Construction stage can reveal them incrementally by simply increasing m_DrawCount each frame
		struct TaggedQuad
		{
			uint32_t idx[6];
			float distSq;
		};

		float center = k_GridN * 0.5f;

		std::vector<TaggedQuad> quads;
		quads.reserve(k_GridN * k_GridN);

		for (int z = 0; z < k_GridN; ++z)
		for (int x = 0; x < k_GridN; ++x)
		{
			uint32_t bl = z * vN + x;
			uint32_t br = bl + 1;
			uint32_t tl = bl + vN;
			uint32_t tr = tl + 1;

			float dx = (x + 0.5f) - center;
			float dz = (z + 0.5f) - center;

			quads.push_back({ { tl, bl, br, tl, br, tr }, dx*dx + dz*dz });
		}

		std::sort(quads.begin(), quads.end(), [](const TaggedQuad& a, const TaggedQuad& b) 
			{ 
				return a.distSq < b.distSq;
			}
		);

		m_SortedIndices.reserve(quads.size() * 6);
		for (auto& q : quads)
			for (int i = 0; i < 6; ++i)
				m_SortedIndices.push_back(q.idx[i]);

		m_TotalIndexCount = static_cast<int>(m_SortedIndices.size());
		m_DrawCount = m_TotalIndexCount;

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_SortedIndices.size() * sizeof(uint32_t), m_SortedIndices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}

	void CreateMSAAFramebuffer()
	{
		// NOTE(saeb): Afterglow should expose MSAA settings through WindowConfig or a dedicated RenderSettings struct
		uint32_t w = GetWindow().GetWidth();
		uint32_t h = GetWindow().GetHeight();

		glGenFramebuffers(1, &m_MSAAFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

		glGenRenderbuffers(1, &m_MSAAColorRB);
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAAColorRB);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples, GL_RGBA8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_MSAAColorRB);

		glGenRenderbuffers(1, &m_MSAADepthRB);
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthRB);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthRB);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DeleteMSAAFramebuffer()
	{
		glDeleteFramebuffers(1, &m_MSAAFBO);
		glDeleteRenderbuffers(1, &m_MSAAColorRB);
		glDeleteRenderbuffers(1, &m_MSAADepthRB);
		m_MSAAFBO = m_MSAAColorRB = m_MSAADepthRB = 0;
	}

	void InitHUDFont()
	{
		// NOTE(saeb): Afterglow should expose the HDC through GraphicsContext - casting GetNativeHandle to HWND is a workaround
		HWND hwnd = static_cast<HWND>(GetWindow().GetNativeHandle());
		HDC hdc = GetDC(hwnd);

		m_FontListBase = glGenLists(96);
		wglUseFontBitmaps(hdc, 32, 96, m_FontListBase);

		ReleaseDC(hwnd, hdc);
		m_FontReady = true;
	}

private:
	// Stage
	Stage m_Stage = Stage::FlatGrid;
	bool m_Paused = false;

	// GPU resources
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_EBO = 0;
	GLuint m_Program = 0;

	// Mesh
	std::vector<uint32_t> m_SortedIndices;
	int m_TotalIndexCount = 0;
	int m_DrawCount = 0;

	// MSAA
	GLuint m_MSAAFBO = 0;
	GLuint m_MSAAColorRB = 0;
	GLuint m_MSAADepthRB = 0;
	bool m_MSAAEnabled = true;
	int m_MSAASamples = 4;

	// Camera
	float m_Azimuth = 0.0f;
	float m_Elevation = 0.5f;
	float m_Radius = 8.0f;
	bool m_AutoOrbit = true;

	// Wave
	float m_Time = 0.0f;
	float m_Amplitude = 0.0f;
	float m_Frequency = 3.0f;
	float m_Source1[2] = { -1.5f, 0.0f };
	float m_Source2[2] = {  1.5f, 0.0f };

	// Wireframe overlay (solid stages only)
	bool m_WireframeOverlay = false;

	// HUD font
	GLuint m_FontListBase = 0;
	bool m_FontReady = false;

	// Timing (QPC)
	LARGE_INTEGER m_QPCFrequency = {};
	LARGE_INTEGER m_LastTime = {};
};

Afterglow::Application* Afterglow::CreateApplication()
{
	return new Wave();
}