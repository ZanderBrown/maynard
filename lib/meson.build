libmaynard_deps = [
    gtk_dep,
    glib_dep,
    gnome_desktop_dep,
    upower_dep,
    libgvc_dep,
]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
libmaynard_deps += m_dep

libmaynard_sources = [
    'gtk-list-models/gtkrbtree.c',
    'gtk-list-models/gtkrbtreeprivate.h',
    'gtk-list-models/gtkfilterlistmodel.c',
    'gtk-list-models/gtkfilterlistmodel.h',
    'gtk-list-models/gtksortlistmodel.c',
    'gtk-list-models/gtksortlistmodel.h',
    'items/mnd-panel-button.c',
    'items/mnd-panel-button.h',
    'items/mnd-power.c',
    'items/mnd-power.h',
    'items/mnd-clock.c',
    'items/mnd-clock.h',
    'items/mnd-sound.c',
    'items/mnd-sound.h',
    'items/mnd-sound-popover.c',
    'items/mnd-sound-popover.h',
    'clock.c',
    'clock.h',
    'mnd-favorites-button.c',
    'mnd-favorites-button.h',
    'mnd-favorites.c',
    'mnd-favorites.h',
    'mnd-app-list-model.c',
    'mnd-app-list-model.h',
    'panel.c',
    'panel.h',
    'mnd-app-launcher.c',
    'mnd-app-launcher.h',
    'mnd-launcher.c',
    'mnd-launcher.h',
    'mnd-wallpaper.c',
    'mnd-wallpaper.h',
]

libmaynard_sources += gnome.compile_resources('maynard-resources',
  'maynard.gresource.xml',
  c_name: 'maynard'
)

libmaynard_lib = static_library ('maynard', libmaynard_sources,
                                 dependencies: libmaynard_deps,
                                 install: true,
                                 install_rpath: maynard_libdir)

libmaynard_inc = include_directories ('.')

libmaynard_dep = declare_dependency (link_whole: libmaynard_lib,
                                     dependencies: libmaynard_deps,
                                     include_directories: libmaynard_inc)


