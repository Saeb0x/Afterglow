#include <Afterglow.h>

class Sandbox : public Afterglow::Application
{
public:
	Sandbox() {

	}

	~Sandbox(){

	}

	void Init() override {
		AG_LOG_INFO("Engine says hello");

		AG_APP_LOG_WARNING("WARNING");
		AG_APP_LOG_ERROR("ERROR");
	}
};

IMPLEMENT_AFTERGLOW_APP(Sandbox);