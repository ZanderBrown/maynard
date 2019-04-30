project('maynard', 'c')

i18n = import('i18n')

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

subdir('data')
subdir('po')
subdir('protocol')
subdir('shell')

meson.add_install_script('build-aux/meson/postinstall.py')
