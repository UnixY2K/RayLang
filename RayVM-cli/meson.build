rayvm_cli_incl = include_directories('include')

rayvm_cli_srcs = [
    'src/source.cpp',
]
rayvm_cli_args = []
rayvm_cli_link = []
rayvm_cli_deps = [rayvm_dep]

executable(
    'ray',
    rayvm_cli_srcs,
    include_directories: [rayvm_cli_incl],
    cpp_args: rayvm_cli_args,
    dependencies: rayvm_cli_deps,
)
