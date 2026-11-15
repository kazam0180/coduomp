game_server_link = []
if emulate_x87 and software_x87_backend == 'softfloat'
  game_server_link = [softfloat_lib]
endif

game_server = shared_library('game.mp.uo.' + game_module_arch, game_c,
  c_args: game_server_args + ['-fPIC'],
  include_directories: [top_include, softfloat_include],
  dependencies: [m_dep],
  link_with: game_server_link,
  name_prefix: '',
  name_suffix: 'so',
  override_options: ['b_lundef=false'],
  install: true,
  install_dir: join_paths(get_option('bindir'), 'uo'))
