// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "plugin.hpp"
#include "controller.hpp"
#include "processor.hpp"

#include <warning-disable.hpp>
#include <public.sdk/source/main/pluginfactory.h>

#include <public.sdk/source/main/moduleinit.cpp>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <public.sdk/source/main/dllmain.cpp>
#elif __APPLE__
#include <public.sdk/source/main/macmain.cpp>
#else
#include <public.sdk/source/main/linuxmain.cpp>
#endif

BEGIN_FACTORY_DEF("Xaymar", "https://xaymar.com/", "mailto:info@xaymar.com")
DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::FUID::fromTUID(tonplugins::tonstream::processor::tuid.data())),
		   PClassInfo::kManyInstances,                    // Allow many instances
		   kVstAudioEffectClass,                          // Type
		   "TonStream",                                   // Name
		   Vst::kDistributable,                           // Allow cross-computer usage.
		   Vst::PlugType::kFxNetwork,                     // Categories (separate with |)
		   tonplugins::get_version().to_string().c_str(), // Version
		   kVstVersionString,                             // VST SDK Version
		   tonplugins::tonstream::processor::create       // Function to create the instance.
)
DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::FUID::fromTUID(tonplugins::tonstream::controller::tuid.data())),
		   PClassInfo::kManyInstances,                    // Allow many instances
		   kVstComponentControllerClass,                  // Type
		   "TonStream (Controller)",                      // Name
		   0,                                             // Unused
		   "",                                            // Unused
		   tonplugins::get_version().to_string().c_str(), // Version
		   kVstVersionString,                             // VST SDK Version
		   tonplugins::tonstream::controller::create      // Function to create the instance.
)
END_FACTORY
#include <warning-enable.hpp>

static std::shared_ptr<tonplugins::core> core;

static auto initializer = Steinberg::ModuleInitializer([]() {
	core = tonplugins::core::instance("TonStream");
});

static auto finalizer = Steinberg::ModuleTerminator([]() {
	core.reset();
});
