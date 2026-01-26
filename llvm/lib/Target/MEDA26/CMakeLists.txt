add_llvm_component_group(MEDA26)

add_llvm_target(MEDA26CodeGen
	MEDA26TargetMachine.cpp
	MEDA26Subtarget.cpp
	MEDA26ISelLowering.cpp

	LINK_COMPONENTS
	Core
	CodeGen
	MEDA26Desc
	MEDA26Info
	MC
	Target
	TargetParser
	SelectionDAG
	Support

	ADD_TO_COMPONENT
	MEDA26
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
