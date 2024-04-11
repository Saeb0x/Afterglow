#include "SpriteRenderer.h"
#include "../GameObject.h"

#include <iostream>
#include <imgui.h>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				SpriteRenderer::SpriteRenderer(const glm::vec4& color) : BaseComponent(), m_LastTransform(nullptr), m_IsDirty(true)
				{
					m_Color = color;
					m_Sprite = std::make_shared<Graphics::Sprite>(nullptr);
				}

				SpriteRenderer::SpriteRenderer(const std::shared_ptr<Graphics::Sprite>& sprite) : BaseComponent(), m_LastTransform(nullptr), m_IsDirty(true)
				{
					m_Sprite = sprite;
					m_Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
				}

				SpriteRenderer::~SpriteRenderer() {}

				void SpriteRenderer::SetColor(const glm::vec4& color)
				{
					if (m_Color != color)
					{
						m_Color = color;
						m_IsDirty = true;
					}
				}

				void SpriteRenderer::SetSprite(const std::shared_ptr<Graphics::Sprite>& sprite)
				{
					m_Sprite = sprite;
					m_IsDirty = true;
				}

				void SpriteRenderer::Start() 
				{
					m_LastTransform = GetOwner()->GetTransform()->Copy();
				}

				void SpriteRenderer::Update(float deltaTime) 
				{
					if (!(m_LastTransform == GetOwner()->GetTransform()))
					{
						GetOwner()->GetTransform()->Copy(m_LastTransform);
						m_IsDirty = true;
					}
				}

				void SpriteRenderer::ImGui()
				{
					float color[4] = { m_Color.x, m_Color.y, m_Color.z, m_Color.w };
					if (ImGui::ColorPicker4("Color Picker", color))
					{
						m_Color = glm::vec4({color[0], color[1], color[2], color[3]});
						m_IsDirty = true;
					}
				}

				void SpriteRenderer::Serialize(rapidjson::Value& object, rapidjson::Document& document)
				{
					if (!document.IsArray()) {
						std::cerr << "RapidJSON Document is not an array" << std::endl;
						return;
					}

					rapidjson::Value spriteRenderer(rapidjson::kObjectType);

					// Serialize Z-Index
					object.AddMember("Z-Index", GetOwner()->GetZIndex(), document.GetAllocator());

					// Serialize Transform
					rapidjson::Value transform(rapidjson::kObjectType);

					rapidjson::Value position(rapidjson::kObjectType);
					position.AddMember("x", GetOwner()->GetTransform()->GetPosition().x, document.GetAllocator());
					position.AddMember("y", GetOwner()->GetTransform()->GetPosition().y, document.GetAllocator());

					rapidjson::Value scale(rapidjson::kObjectType);
					scale.AddMember("x", GetOwner()->GetTransform()->GetScale().x, document.GetAllocator());
					scale.AddMember("y", GetOwner()->GetTransform()->GetScale().y, document.GetAllocator());

					transform.AddMember("Position", position, document.GetAllocator());
					transform.AddMember("Scale", scale, document.GetAllocator());

					object.AddMember("Transform", transform, document.GetAllocator());

					// Serialize Color
					rapidjson::Value color(rapidjson::kObjectType);
					color.AddMember("r", m_Color.r, document.GetAllocator());
					color.AddMember("g", m_Color.g, document.GetAllocator());
					color.AddMember("b", m_Color.b, document.GetAllocator());
					color.AddMember("a", m_Color.a, document.GetAllocator());
					spriteRenderer.AddMember("Color", color, document.GetAllocator());

					// Serialize Sprite (if available)
					if (m_Sprite) {
						m_Sprite->Serialize(spriteRenderer, document);
					}

					// Add the spriteRenderer object to the main object
					object.AddMember("Sprite Renderer", spriteRenderer, document.GetAllocator());
				}
			}
		}
	}
}