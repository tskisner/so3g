project(
  'so3g',
  'c', 'c++',
  license: 'MIT',
  meson_version: '>= 1.0.0',
  default_options: [
    'buildtype=release',
    'c++_std=c++17',
  ]
)

openmp = dependency('openmp', required: false)
libflac = dependency('flac', version: '>= 1.4.0', static: false)

py = import('python').find_installation(pure: false)

subdir('src')
