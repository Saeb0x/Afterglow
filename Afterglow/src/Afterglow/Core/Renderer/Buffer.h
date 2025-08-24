#pragma once

#include "Afterglow/Core/Assert.h"

namespace Afterglow
{
	enum class ShaderDataType
	{
		None = 0,
		Int, Int2, Int3, Int4,
		Float, Float2, Float3, Float4,
		Mat3, Mat4
	};

	static uint32_t GetSizeFromShaderDataType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Int:       return 4;
			case ShaderDataType::Int2:      return 4 * 2;
			case ShaderDataType::Int3:      return 4 * 3;
			case ShaderDataType::Int4:      return 4 * 4;
			case ShaderDataType::Float:     return 4;
			case ShaderDataType::Float2:    return 4 * 2;
			case ShaderDataType::Float3:    return 4 * 3;
			case ShaderDataType::Float4:    return 4 * 4;
			case ShaderDataType::Mat3:      return 4 * 3 * 3;
			case ShaderDataType::Mat4:      return 4 * 4 * 4;
		}

		AG_ASSERT(false, "Unknown Shader Data Type!");
		return 0;
	}

	static uint32_t GetCountFromShaderDataType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Int:       return 1;
			case ShaderDataType::Int2:      return 2;
			case ShaderDataType::Int3:      return 3;
			case ShaderDataType::Int4:      return 4;
			case ShaderDataType::Float:     return 1;
			case ShaderDataType::Float2:    return 2;
			case ShaderDataType::Float3:    return 3;
			case ShaderDataType::Float4:    return 4;
			case ShaderDataType::Mat3:      return 3 * 3;
			case ShaderDataType::Mat4:      return 4 * 4;
		}

		AG_ASSERT(false, "Unknown Shader Data Type!");
		return 0;
	}

	struct BufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Count;
		uint32_t Size;
		size_t Offset;
		bool Normalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Type(type), Name(name), Count(GetCountFromShaderDataType(Type)), Size(GetSizeFromShaderDataType(Type)), Offset(0), Normalized(normalized)
		{
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

		uint32_t GetStride() const { return m_Stride; }

	private:
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;

		static VertexBuffer* Create(uint32_t size, const float* vertices);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		static IndexBuffer* Create(uint32_t count, const uint32_t* indices);
	};
}