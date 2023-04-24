// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include <string_view>

#include <pluginterfaces/vst/ivstchannelcontextinfo.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

namespace tonplugins::tonstream {
	class controller : public Steinberg::Vst::EditControllerEx1, public Steinberg::Vst::ChannelContext::IInfoListener {
		public:
		controller(void* data);
		~controller();

		public /* IPluginBase */:
		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

		Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;

		Steinberg::tresult PLUGIN_API setChannelContextInfos(Steinberg::Vst::IAttributeList* list) override;

		public:
		OBJ_METHODS(controller, Steinberg::Vst::EditControllerEx1)
		DEFINE_INTERFACES
		DEF_INTERFACE(Steinberg::Vst::ChannelContext::IInfoListener)
		END_DEFINE_INTERFACES(Steinberg::Vst::EditController)
		DELEGATE_REFCOUNT(Steinberg::Vst::EditControllerEx1)

		public:
		static Steinberg::FUnknown* create(void* data);

		static constexpr std::string_view tuid = "XmrCTonStream   ";
	};

} // namespace tonplugins::tonstream
