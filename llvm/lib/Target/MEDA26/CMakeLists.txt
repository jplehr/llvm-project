add_llvm_component_group(MEDA26)

add_llvm_target(MEDA26CodeGen
	MEDA26TargetMachine.cpp
	MEDA26Subtarget.cpp
	MEDA26ISelLowering.cpp
	MEDA26TargetTransformInfo.cpp

	LINK_COMPONENTS
	Analysis
	Core
	CodeGen
	MEDA26Desc
	MEDA26Info
	MC
	Target
	TargetParser
	TransformUtils
	SelectionDAG
	Support

	ADD_TO_COMPONENT
	MEDA26
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
