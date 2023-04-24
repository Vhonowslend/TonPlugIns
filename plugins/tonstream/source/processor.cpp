// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "processor.hpp"

tonplugins::tonstream::processor::processor(void* data)
	: Steinberg::Vst::AudioEffect()
{
}

tonplugins::tonstream::processor::~processor()
{
}

Steinberg::FUnknown* tonplugins::tonstream::processor::create(void* data)
{
	return reinterpret_cast<Steinberg::FUnknown*>(new tonplugins::tonstream::processor(data));
}
