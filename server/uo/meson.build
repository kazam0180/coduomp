game_server = shared_library('game.mp.uo.' + game_module_arch, game_c,
  c_args: game_server_args + ['-fPIC'],
  include_directories: top_include,
  dependencies: [m_dep],
  name_prefix: '',
  name_suffix: 'so',
  override_options: ['b_lundef=false'],
  install: true,
  install_dir: join_paths(get_option('bindir'), 'uo'))
