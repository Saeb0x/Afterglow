#include "AfterglowEditor.h"

#include "EditorLayer.h"

AfterglowEditor::AfterglowEditor()
	: Afterglow::Application("Afterglow Editor")
{
	PushLayer(new EditorLayer());
}