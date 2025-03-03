rayvm_incl = include_directories('include')

rayvm_srcs = [
	'src/assembler.cpp',
	'src/memory.cpp',
	'src/vm.cpp',
]
rayvm_args = []
rayvm_link = []
rayvm_deps = []

rayvm_lib = library(
	'rayvm',
	rayvm_srcs,
	include_directories: [rayvm_incl],
	cpp_args: rayvm_args,
	dependencies: rayvm_deps,
)

rayvm_dep = declare_dependency(
	link_with: rayvm_lib,
	include_directories: rayvm_incl,
)
