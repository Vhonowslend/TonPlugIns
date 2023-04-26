// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "controller.hpp"
#include <core.hpp>

#include <warning-disable.hpp>
#include <vstgui/plugin-bindings/vst3editor.h>
#include <warning-enable.hpp>

tonplugins::tonstream::controller::controller(void* data)
	: Steinberg::Vst::EditControllerEx1(), Steinberg::Vst::ChannelContext::IInfoListener()
{
}

tonplugins::tonstream::controller::~controller()
{
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::controller::initialize(Steinberg::FUnknown* context)
{
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::controller::setComponentState(Steinberg::IBStream* state)
{
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::controller::setChannelContextInfos(Steinberg::Vst::IAttributeList* list)
{
	return Steinberg::kResultOk;
}

Steinberg::IPlugView* PLUGIN_API tonplugins::tonstream::controller::createView(Steinberg::FIDString name)
{
	if (strcmp(name, Steinberg::Vst::ViewType::kEditor) == 0) {
		return new VSTGUI::VST3Editor(this, "view", "myEditor.uidesc");
	}
	return 0;
}

Steinberg::FUnknown* tonplugins::tonstream::controller::create(void* data)
{
	try {
		return static_cast<Steinberg::Vst::IEditController*>(new tonplugins::tonstream::controller(data));
	} catch (std::exception const& ex) {
		tonplugins::core::instance()->log("%s: %s", __FUNCTION_SIG__, ex.what());
	} catch (...) {
		tonplugins::core::instance()->log("%s: An unknown error occurred.");
	}
	return nullptr;
}
