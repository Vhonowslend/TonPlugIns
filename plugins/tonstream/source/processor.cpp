// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "processor.hpp"

#include <warning-disable.hpp>
#include <ringbuffer.hpp>
#include <warning-enable.hpp>

tonplugins::tonstream::processor::processor(void* data)
	: Steinberg::Vst::AudioEffect()
{
	auto rb = tonplugins::memory::float_ring_t(2048);
}

tonplugins::tonstream::processor::~processor()
{
}

Steinberg::FUnknown* tonplugins::tonstream::processor::create(void* data)
{
	return reinterpret_cast<Steinberg::FUnknown*>(new tonplugins::tonstream::processor(data));
}
