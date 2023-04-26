// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include <warning-disable.hpp>
#include <string_view>

#include <public.sdk/source/vst/vstaudioeffect.h>
#include <warning-enable.hpp>

namespace tonplugins::tonstream {
	class processor : public Steinberg::Vst::AudioEffect {
		processor(void* data);
		~processor();

		public /* IPluginBase */:
		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

		public /* IComponent */:
		Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

		Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

		public /* IAudioProcessor */:
		void PLUGIN_API update(FUnknown* changedUnknown, Steinberg::int32 message) override;

		Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) override;

		Steinberg::tresult PLUGIN_API getBusArrangement(Steinberg::Vst::BusDirection dir, Steinberg::int32 index, Steinberg::Vst::SpeakerArrangement& arr) override;

		Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

		Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;

		Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state) override;

		Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;

		Steinberg::uint32 PLUGIN_API getLatencySamples() override;

		Steinberg::uint32 PLUGIN_API getTailSamples() override;

		public:
		static FUnknown* create(void* data);

		static constexpr std::string_view tuid = "XmrPTonStream   ";
	};
} // namespace tonplugins::tonstream
