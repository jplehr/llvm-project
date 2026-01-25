add_llvm_component_group(MEDA26)

add_llvm_target(MEDA26CodeGen
	MEDA26TargetMachine.cpp

	LINK_COMPONENTS
	MEDA26Desc
	MEDA26Info
	Support

	ADD_TO_COMPONENT
	MEDA26
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
