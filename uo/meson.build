cgame = shared_library('uo_cgame_mp_' + module_arch, cgame_c,
  c_args: cgame_c_args,
  include_directories: top_include,
  dependencies: [m_dep],
  name_prefix: '',
  name_suffix: 'so',
  link_args: ['-Wl,--no-undefined'],
  install: true,
  install_dir: join_paths(get_option('libdir'), 'uo'))

ui = shared_library('uo_ui_mp_' + module_arch, ui_c,
  c_args: ui_c_args,
  include_directories: top_include,
  dependencies: [m_dep],
  name_prefix: '',
  name_suffix: 'so',
  link_args: ['-Wl,--no-undefined'],
  install: true,
  install_dir: join_paths(get_option('libdir'), 'uo'))

if get_option('game')
  game_client_link = []
  if need_softfloat
    game_client_link = [softfloat_lib]
  endif

  game_client = shared_library('uo_game_mp_' + module_arch, game_c,
    c_args: game_client_args,
    include_directories: [top_include, softfloat_include],
    dependencies: [m_dep],
    link_with: game_client_link,
    name_prefix: '',
    name_suffix: 'so',
    override_options: ['b_lundef=false'],
    install: true,
    install_dir: join_paths(get_option('libdir'), 'uo'))
endif
