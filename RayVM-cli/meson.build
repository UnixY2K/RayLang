rayvm_cli_incl = include_directories('include')

rayvm_cli_srcs = [
	'src/cli_args.cpp',
	'src/options.cpp',
	'src/source.cpp',
	'src/terminal.cpp',
]
rayvm_cli_args = []
rayvm_cli_link = []
rayvm_cli_deps = [rayvm_dep]
rayvm_cli_defs = []

if get_option('cli_terminal_color_support')
	rayvm_cli_defs += ['RAYVM_APP_TERMINAL_COLOR_SUPPORT=true']
endif

# iterate over defines and add them to cpp_args
foreach def : rayvm_cli_defs
	rayvm_cli_args += ['-D' + def]
endforeach

executable(
	'ray',
	rayvm_cli_srcs,
	include_directories: [rayvm_cli_incl],
	cpp_args: rayvm_cli_args,
	dependencies: rayvm_cli_deps,
)
