project('echoNet', 'c')

# Directories
src_dir = 'src'
test_dir = 'tests'
unity_dir = 'third_party/unity/src'

# Code files
src_files = files(
    join_paths(src_dir, 'server.c')
)
test_files = files(
    join_paths(test_dir, 'test_server.c')
)

# Unity files
unity_files = files(
    join_paths(unity_dir, 'unity.c')
)

include_dirs = include_directories(src_dir, unity_dir)
unity_lib = static_library('unity', unity_files, 
                            include_directories: include_dirs)

# Executable test files
test_exe = executable('test_my_code', test_files + src_files,
                      include_directories: include_dirs,
                      link_with: unity_lib)

# Test
test('unit_test', test_exe)

