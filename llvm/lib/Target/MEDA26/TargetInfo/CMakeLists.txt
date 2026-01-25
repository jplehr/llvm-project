add_llvm_component_library(LLVMMEDA26Info
	MEDA26TargetInfo.cpp

	LINK_COMPONENTS
	MC
	Support

	ADD_TO_COMPONENT
	MEDA26
)
