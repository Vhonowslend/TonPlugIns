/// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2023-2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// AUTOGENERATED COPYRIGHT HEADER END
int main(int argc, const char** argv) {
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
	#error "TARGET_SYSTEM_ARCHITECTURE=x86_64"
#elif (defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_X86)) && !defined(_M_ARM64EC)
	#error "TARGET_SYSTEM_ARCHITECTURE=x86_32"
#elif defined(_M_ARM64EC)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARM64EC"
#elif defined(__aarch64__) || defined(_M_ARM64)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARM64"
#elif defined(__ARM_ARCH_7S__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv7S"
#elif defined(__ARM_ARCH_7M__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv7M"
#elif defined(mips) || defined(__mips__) || defined(__mips)
	#error "TARGET_SYSTEM_ARCHITECTURE=MIPS"
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv7R"
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv7A"
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv7"
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv6"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv6T2"
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv5"
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv4T"
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv3"
#elif defined(__ARM_ARCH_2__)
	#error "TARGET_SYSTEM_ARCHITECTURE=ARMv2"
#elif defined(__sh__)
	#error "TARGET_SYSTEM_ARCHITECTURE=SUPERH"
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
	#error "TARGET_SYSTEM_ARCHITECTURE=POWERPC"
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
	#error "TARGET_SYSTEM_ARCHITECTURE=POWERPC64"
#elif defined(__sparc__) || defined(__sparc)
	#error "TARGET_SYSTEM_ARCHITECTURE=SPARC"
#elif defined(__m68k__)
	#error "TARGET_SYSTEM_ARCHITECTURE=M68K"
#else
	#error "TARGET_SYSTEM_ARCHITECTURE=UNKNOWN"
#endif
	return 0;
}
