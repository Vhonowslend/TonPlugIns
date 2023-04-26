// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "processor.hpp"
#include <core.hpp>
#include <ringbuffer.hpp>
#include "controller.hpp"

#include <warning-disable.hpp>
#include <stdexcept>
#include <warning-enable.hpp>

tonplugins::tonstream::processor::processor(void* data)
	: Steinberg::Vst::AudioEffect()
{
	setControllerClass(Steinberg::FUID::fromTUID(tonplugins::tonstream::controller::tuid.data()));
}

tonplugins::tonstream::processor::~processor()
{
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::initialize(Steinberg::FUnknown* context)
{
	if (auto res = AudioEffect::initialize(context); res != Steinberg::kResultOk) {
		return res;
	}
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::setState(Steinberg::IBStream* state)
{
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::getState(Steinberg::IBStream* state)
{
	return Steinberg::kResultOk;
}

void PLUGIN_API tonplugins::tonstream::processor::update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message)
{
	return;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts)
{
	if (auto res = AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts); res != Steinberg::kResultOk) {
		return res;
	}
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::getBusArrangement(Steinberg::Vst::BusDirection dir, Steinberg::int32 index, Steinberg::Vst::SpeakerArrangement& arr)
{
	if (auto res = AudioEffect::getBusArrangement(dir, index, arr); res != Steinberg::kResultOk) {
		return res;
	}
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::canProcessSampleSize(Steinberg::int32 symbolicSampleSize)
{
	if (auto res = AudioEffect::canProcessSampleSize(symbolicSampleSize); res != Steinberg::kResultOk) {
		return res;
	}
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::setupProcessing(Steinberg::Vst::ProcessSetup& setup)
{
	if (auto res = AudioEffect::setupProcessing(setup); res != Steinberg::kResultOk) {
		return res;
	}
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::setProcessing(Steinberg::TBool state)
{
	return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API tonplugins::tonstream::processor::process(Steinberg::Vst::ProcessData& data)
{
	return Steinberg::kResultOk;
}

Steinberg::uint32 PLUGIN_API tonplugins::tonstream::processor::getLatencySamples()
{
	return 0;
}

Steinberg::uint32 PLUGIN_API tonplugins::tonstream::processor::getTailSamples()
{
	return 0;
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
