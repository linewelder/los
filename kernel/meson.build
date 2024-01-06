headers = include_directories('include')
sources = run_command('find', '.', '-type', 'f', '-name', '*.cpp', '-or', '-name', '*.s', check: true) \
    .stdout() \
    .strip() \
    .split('\n')

executable(
    'los.bin',
    sources,
    include_directories: headers,
    link_args: ['-T', meson.current_source_dir() / 'linker.ld'],

    install: true,
    install_dir: 'boot')
