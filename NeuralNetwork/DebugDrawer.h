#pragma once

#include <vector>

#include "GLShader.h"
#include "math.h"

static const math::vec4 DEBUG_RED		= math::vec4(1.00f, 0.41f, 0.38f, 1.0f);
static const math::vec4 DEBUG_GREEN		= math::vec4(0.47f, 0.87f, 0.47f, 1.0f);
static const math::vec4 DEBUG_BLUE		= math::vec4(0.68f, 0.78f, 0.90f, 1.0f);
static const math::vec4 DEBUG_YELLOW	= math::vec4(0.99f, 0.99f, 0.59f, 1.0f);

class GraphicsManager;
class DebugDrawer
{
	struct DebugModel
	{
		GLuint vao;
		math::mat4 model;
		math::vec4 color;

		GLenum primitive;
		GLsizei count;
	};

public:
	DebugDrawer(GraphicsManager& graphicsMgr);
	~DebugDrawer();

	void RenderDebugShapes();

	// utils
	void AddDebugCircle(const math::mat4& model, const math::vec4& color);
	void AddDebugBox(const math::mat4& model, const math::vec4& color);
	void AddDebugLine(const math::vec3& start, const math::vec3& end, const math::vec4& color, float thickness = 5.f);

	void AddFilledDebugBox(const math::mat4& model, const math::vec4& color);
	void AddDebugFilledCircle(const math::vec3& worldpos, float radius, const math::vec4& color);

private:
	GLShader m_debugShader;
	std::vector<DebugModel> m_models;
	GraphicsManager& m_graphicsMgr;

	struct GraphicsBuffer
	{
		GLuint m_VAO, m_VBO, m_EBO;
	}m_boxBuffer, m_lineBuffer, m_circleBuffer;

	void InitializeBuffers();

	template<typename T, typename V, unsigned K, unsigned L>
	void SetupVAOs(DebugDrawer::GraphicsBuffer& buffer, const T(&vertices)[K], const V(&indices)[L]) const;

	template<typename T, typename V>
	void SetupVAOs(DebugDrawer::GraphicsBuffer& buffer, std::vector<T> const & vertices, const std::vector<V> & indices) const;
};


template<typename T, typename V, unsigned K, unsigned L>
void DebugDrawer::SetupVAOs(DebugDrawer::GraphicsBuffer& buffer, const T(&vertices)[K], const V(&indices)[L]) const
{
	//Vertex Buffers 
	glGenVertexArrays(1, &buffer.m_VAO);
	glGenBuffers(1, &buffer.m_VBO);
	glGenBuffers(1, &buffer.m_EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(buffer.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
}

template<typename T, typename V>
void DebugDrawer::SetupVAOs(DebugDrawer::GraphicsBuffer& buffer, std::vector<T> const & vertices, const std::vector<V> & indices) const
{
	glGenVertexArrays(1, &buffer.m_VAO);
	glGenBuffers(1, &buffer.m_VBO);
	glGenBuffers(1, &buffer.m_EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(buffer.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
}