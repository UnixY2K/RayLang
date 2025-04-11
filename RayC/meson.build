rayc_incl = include_directories('include')

rayc_srcs = [
	'src/cli/cli_args.cpp',
	'src/cli/options.cpp',
	'src/cli/terminal.cpp',
	'src/compiler/lexer/lexer_error.cpp',
	'src/compiler/lexer/lexer.cpp',
	'src/compiler/lexer/token.cpp',
	'src/compiler/parser/parser.cpp',
	'src/compiler/error_bag.cpp',
	'src/source.cpp',
]
rayc_args = []
rayc_link = []
rayc_deps = []
rayc_defs = []

if get_option('cli_terminal_color_support')
	rayc_defs += ['RAYC_APP_TERMINAL_COLOR_SUPPORT=true']
endif

# iterate over defines and add them to cpp_args
foreach def : rayc_defs
	rayc_args += ['-D' + def]
endforeach

executable(
	'rayc',
	rayc_srcs,
	include_directories: [rayc_incl],
	cpp_args: rayc_args,
	dependencies: rayc_deps,
)
