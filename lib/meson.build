libmaynard_deps = [
    gtk_dep,
    glib_dep,
    gnome_menu_dep,
    gnome_desktop_dep,
    alsa_dep,
    upower_dep,
]

libmaynard_sources =     [
    'items/mnd-panel-button.c',
    'items/mnd-panel-button.h',
    'items/mnd-power.c',
    'items/mnd-power.h',
    'items/mnd-clock.c',
    'items/mnd-clock.h',
    'clock.c',
    'clock.h',
    'sound.c',
    'sound.h',
    'mnd-favorites-button.c',
    'mnd-favorites-button.h',
    'mnd-favorites.c',
    'mnd-favorites.h',
    'shell-app-system.c',
    'shell-app-system.h',
    'panel.c',
    'panel.h',
    'launcher.c',
    'launcher.h',
]

libmaynard_sources += gnome.compile_resources('maynard-resources',
  'maynard.gresource.xml',
  c_name: 'maynard'
)

libmaynard_lib = library ('maynard', libmaynard_sources,
                          dependencies: libmaynard_deps,
                          install: true)

libmaynard_inc = include_directories ('.')

libmaynard_dep = declare_dependency (link_with: libmaynard_lib,
                                     dependencies: libmaynard_deps,
                                     include_directories: libmaynard_inc)


