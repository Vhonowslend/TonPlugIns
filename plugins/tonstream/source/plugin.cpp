// Copyright 2023 Michael Fabian 'Xaymar' Dirks < info @xaymar.com>

#include "plugin.hpp"
#include "controller.hpp"
#include "processor.hpp"

#include <warning-disable.hpp>
#include <public.sdk/source/main/pluginfactory.h>

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

TONPLUGINS_EXPORT bool InitModule()
{
	return true;
}

TONPLUGINS_EXPORT bool DeinitModule()
{
	return true;
}

#ifdef _WIN32
#include <Windows.h>

TONPLUGINS_EXPORT DWORD DllMain(LPVOID handle, DWORD reason, LPVOID reserved)
{
	return 0;
}
#endif
