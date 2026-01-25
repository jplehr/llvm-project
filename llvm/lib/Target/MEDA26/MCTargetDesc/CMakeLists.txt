add_llvm_component_library(LLVMMEDA26Desc
	MEDA26MCTargetDesc.cpp

	LINK_COMPONENTS
	Support

	ADD_TO_COMPONENT
	MEDA26
)
