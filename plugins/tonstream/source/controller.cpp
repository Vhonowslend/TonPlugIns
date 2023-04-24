// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "controller.hpp"

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

Steinberg::FUnknown* tonplugins::tonstream::controller::create(void* data)
{
	return reinterpret_cast<Steinberg::FUnknown*>(new tonplugins::tonstream::controller(data));
}
