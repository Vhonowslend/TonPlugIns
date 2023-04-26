// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "processor.hpp"
#include <core.hpp>

#include <warning-disable.hpp>
#include <ringbuffer.hpp>
#include <warning-enable.hpp>

tonplugins::tonstream::processor::processor(void* data)
	: Steinberg::Vst::AudioEffect()
{
}

tonplugins::tonstream::processor::~processor()
{
}

Steinberg::FUnknown* tonplugins::tonstream::processor::create(void* data)
{
	try {
		return static_cast<Steinberg::Vst::IAudioProcessor*>(new tonplugins::tonstream::processor(data));
	} catch (std::exception const& ex) {
		tonplugins::core::instance()->log("%s: %s", __FUNCTION_SIG__, ex.what());
	} catch (...) {
		tonplugins::core::instance()->log("%s: An unknown error occurred.");
	}
	return nullptr;
}
