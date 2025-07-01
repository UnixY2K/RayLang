project(
    'RayLang',
    'cpp',
    default_options: ['cpp_std=c++23'],
    version: '1.0.0',
    license: 'BSD-3',
)

msgpack_dep = dependency(
                'msgpackc-cc',
                fallback: ['msgpackc-cxx', 'msgpack_cxx_dep']
)

subdir('RayC')
subdir('RayVM')
subdir('RayVM-cli')
