project(
    'los', 'cpp',
    default_options: [
        'warning_level=1',
        'cpp_std=c++20',
        'optimization=2'])

subdir('kernel')

install_subdir(
    'root',
    install_dir: '',
    strip_directory: true)
