add_llvm_component_group(MEDA26)

add_llvm_target(MEDA26CodeGen
	MEDA26TargetMachine.cpp

	LINK_COMPONENTS
	CodeGen
	MEDA26Desc
	MEDA26Info
	MC
	Target
	Support

	ADD_TO_COMPONENT
	MEDA26
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
