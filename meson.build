project('maynard', 'c')

i18n = import('i18n')
gnome = import('gnome')

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', 'maynard')
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
configure_file(
  output: 'config.h',
  configuration: config_h,
)

add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

wl_client_dep = dependency ('wayland-client', version: '>= 1.0.2')
gtk_dep = dependency ('gtk+-3.0', version: '>= 3.24')
gtk_wl_dep = dependency ('gtk+-wayland-3.0')
glib_dep = dependency ('glib-2.0')
gnome_menu_dep = dependency ('libgnome-menu-3.0')
gnome_desktop_dep = dependency ('gnome-desktop-3.0')
alsa_dep = dependency ('alsa')
upower_dep = dependency ('upower-glib')
libweston_dep = dependency ('libweston-6')
wl_server_dep = dependency ('wayland-server')
weston_protocols_dep = dependency ('libweston-6-protocols')
pixman_dep = dependency ('pixman-1')

subdir('data')
subdir('po')
subdir('protocol')
subdir('lib')
subdir('src')

meson.add_install_script('build-aux/meson/postinstall.py')
