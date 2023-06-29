project('Examples','c',version : '1.0.0',
	default_options : ['warning_level=3'],meson_version: '>= 0.43')

project_description = 'Compile all examples'

sourceRoot = meson.source_root()
cc = meson.get_compiler('c')


warning_flags = [
  '-Wmissing-declarations',
  '-Wredundant-decls',
  '-Wwrite-strings',
  '-Winit-self',
  '-Wmissing-include-dirs',
  '-Wno-multichar',
  '-Wvla',
  '-Wall',
  '-Wpointer-arith',
  '-Wmissing-prototypes',
  '-Wdeclaration-after-statement',
  '-Wold-style-definition',
]

executable_dir = sourceRoot + '/executables'

subdir('ImageCapture')
subdir('GPIO')
