// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include <string_view>

#include <public.sdk/source/vst/vstaudioeffect.h>

namespace tonplugins::tonstream {
	class processor : public Steinberg::Vst::AudioEffect {
		processor(void* data);
		~processor();

		public:
		static FUnknown* create(void* data);

		static constexpr std::string_view tuid = "XmrPTonStream   ";
	};
} // namespace tonplugins::tonstream
