#include <memory>
#include <string>
#include <vector>

#include "component/BaseComponent.h"

namespace Afterglow 
{
    namespace Core 
    {
        namespace Entity 
        {
            class GameObject 
            {
            public:
                GameObject(const std::string& name);
                ~GameObject();

                inline const std::string& GetName() const { return m_Name; }

                void Start();
                void Update(float deltaTime);
            public:
                template<typename T>
                void AddComponent(std::shared_ptr<T> component) 
                {
                    component->SetOwner(this);
                    m_Components.push_back(component);
                }

                template<typename T>
                std::shared_ptr<T> GetComponent() const 
                {
                    for (const auto& component : m_Components) {
                        if (auto castedComponent = std::dynamic_pointer_cast<T>(component)) {
                            return castedComponent;
                        }
                    }
                    return nullptr; // Component not found
                }

                template<typename T>
                void RemoveComponent(std::shared_ptr<T> component) 
                {
                    auto it = std::find(m_Components.begin(), m_Components.end(), component);
                    if (it != m_Components.end()) {
                        m_Components.erase(it);
                    }
                }
            private:
                std::string m_Name;
                std::vector<std::shared_ptr<Component::BaseComponent>> m_Components;
            };
        } 
    } 
}
