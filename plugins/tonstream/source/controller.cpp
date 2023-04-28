// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "controller.hpp"
#include <core.hpp>

#include <warning-disable.hpp>
#include <base/source/fstreamer.h>
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
	if (state == nullptr) {
		return Steinberg::kResultFalse;
	}

	Steinberg::IBStreamer streamer(state, kLittleEndian);
	if (bool v; streamer.readBool(v) == true) {
		_cfg_submitter = v;
	}
	{ // Identifier
		std::vector<char> v(256);
		if (auto len = streamer.readString8(v.data(), v.size()); len > 0) {
			_cfg_identifier = std::string(v.data());
		}
	}
	{ // IP
		std::vector<char> v(256);
		if (auto len = streamer.readString8(v.data(), v.size()); len > 0) {
			_cfg_ip = std::string(v.data());
		}
	} // Port
	if (uint16_t v; streamer.readInt16u(v) == true) {
		_cfg_port = v;
	}

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
