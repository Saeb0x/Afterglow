#include <Afterglow.h>

#include "iostream"

class Sandbox : public Afterglow::Application
{
public:
	Sandbox() {

	}

	~Sandbox(){

	}

	void Init() override {
		std::cout << "Sandbox Initialization..." << std::endl;
	}
};

IMPLEMENT_AFTERGLOW_APP(Sandbox);