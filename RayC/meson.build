rayc_incl = include_directories('include')

rayc_srcs = [
	'src/cli/cli_args.cpp',
	'src/cli/options.cpp',
	'src/cli/terminal.cpp',
	'src/compiler/ast/intrinsic.cpp',
	'src/compiler/generators/c/c_transpiler.cpp',
	'src/compiler/generators/wasm/wasm_text.cpp',
	'src/compiler/lang/sourceUnit.cpp',
	'src/compiler/lang/type.cpp',
	'src/compiler/lexer/lexer_error.cpp',
	'src/compiler/lexer/lexer.cpp',
	'src/compiler/lexer/token.cpp',
	'src/compiler/parser/parser.cpp',
	'src/compiler/passes/symbol_mangler.cpp',
	'src/compiler/passes/topLevelResolver.cpp',
	'src/compiler/passes/typeChecker.cpp',
	'src/compiler/S1/lang/S1SourceUnit.cpp',
	'src/compiler/message_bag.cpp',
	'src/source.cpp',
]
rayc_args = []
rayc_link = []
rayc_deps = [
	msgpack_dep,
]
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