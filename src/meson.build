maynard_deps = [
    wl_client_dep,
    gtk_dep,
    gtk_wl_dep,
    glib_dep,
    libweston_dep,
    weston_protocols_dep,
    libmaynard_dep,
]

maynard_sources = [
    'maynard.c',
    weston_desktop_shell_client_protocol_h,
    weston_desktop_shell_protocol_c,
    shell_helper_protocol_c,
    shell_helper_client_protocol_h,
]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
maynard_deps += m_dep

executable ('maynard', maynard_sources,
            dependencies: maynard_deps,
            install: true,
            install_dir: get_option('libexecdir'))

subdir('helper')
