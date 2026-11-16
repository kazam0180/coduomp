server_exe = executable('coduo_lnxded_recovered', server_sources,
  c_args: server_c_args,
  cpp_args: server_cxx_args,
  include_directories: top_include,
  dependencies: server_deps,
  link_with: server_link_with,
  install: true,
  install_dir: get_option('bindir'))

if get_option('game')
  subdir('uo')
endif
