maynard_sources = [
    'maynard.c',
    'app-icon.c',
    'app-icon.h',
    'items/mnd-panel-button.c',
    'items/mnd-panel-button.h',
    'items/mnd-power.c',
    'items/mnd-power.h',
    'clock.c',
    'clock.h',
    'sound.c',
    'sound.h',
    'favorites.c',
    'favorites.h',
    'shell-app-system.c',
    'shell-app-system.h',
    'panel.c',
    'panel.h',
    'vertical-clock.c',
    'vertical-clock.h',
    'launcher.c',
    'launcher.h',
    weston_desktop_shell_client_protocol_h,
    weston_desktop_shell_protocol_c,
    shell_helper_protocol_c,
    shell_helper_client_protocol_h,
]

gnome = import('gnome')

maynard_sources += gnome.compile_resources('maynard-resources',
  'maynard.gresource.xml',
  c_name: 'maynard'
)


wl_dep = dependency ('wayland-client', version: '>= 1.0.2')
gtk_dep = dependency ('gtk+-3.0', version: '>= 3.24')
gtk_wl_dep = dependency ('gtk+-wayland-3.0')
glib_dep = dependency ('glib-2.0')
gnome_menu_dep = dependency ('libgnome-menu-3.0')
gnome_desktop_dep = dependency ('gnome-desktop-3.0')
alsa_dep = dependency ('alsa')
upower_dep = dependency ('upower-glib')
libweston_dep = dependency ('libweston-6')
wayland_server_dep = dependency ('wayland-server')
weston_protocols_dep = dependency ('libweston-6-protocols')
pixman_dep = dependency ('pixman-1')

maynard_deps = [
    wl_dep,
    gtk_dep,
    gtk_wl_dep,
    glib_dep,
    gnome_menu_dep,
    gnome_desktop_dep,
    alsa_dep,
    upower_dep,
    libweston_dep,
    weston_protocols_dep,
    wayland_server_dep,
    pixman_dep,
]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
maynard_deps += m_dep

executable ('maynard', maynard_sources,
            dependencies: maynard_deps,
            install: true,
            install_dir: get_option('libexecdir'))

helper_sources = [
    'shell-helper.c',
    shell_helper_server_protocol_h,
    shell_helper_protocol_c,
]

shared_library ('shell-helper', helper_sources,
                dependencies: maynard_deps,
                name_prefix: '',
                install: true,
                install_dir: join_paths (get_option('libdir'), 'weston'))
