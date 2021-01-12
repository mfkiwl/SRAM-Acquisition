project('SRAM Recovery Station',
        'cpp',
        default_options : ['warning_level=2'],
        version: '0.1')

inc_dir = include_directories('include')

boost_dep = dependency('boost')
fmt_dep = dependency('fmt')
mongo_dep = dependency('libmongocxx')
thread_dep = dependency('threads')
served_dep = dependency('served')

src_files = [
  'include/influxdb.hpp',
  'include/packet.hpp',
  'include/device_manager.hpp',
  'src/device_manager.cpp',
  'include/db_manager.hpp',
  'src/db_manager.cpp',
  'include/log_manager.hpp',
  'src/log_manager.cpp',
  'include/station.hpp',
  'src/station.cpp',
  'src/main.cpp'
]

deps = [
  fmt_dep,
  boost_dep,
  thread_dep,
  mongo_dep,
  served_dep
]

executable('station', src_files,
           dependencies : deps,
           include_directories : inc_dir,
           cpp_args : '-std=c++2a')
