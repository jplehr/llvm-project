add_llvm_component_library(LLVMMEDA26Desc
	MEDA26MCTargetDesc.cpp
	MEDA26MCAsmInfo.cpp

	LINK_COMPONENTS
	MC
	Support
	MEDA26Info

	ADD_TO_COMPONENT
	MEDA26
)
