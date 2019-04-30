helper_sources = [
    'shell-helper.c',
    shell_helper_server_protocol_h,
    shell_helper_protocol_c,
]

helper_deps = [
    libweston_dep,
    wl_server_dep,
    pixman_dep,
]

shared_library ('shell-helper', helper_sources,
                dependencies: helper_deps,
                name_prefix: '',
                install: true,
                install_dir: join_paths (get_option('libdir'), 'weston'))
