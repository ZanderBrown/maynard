# maynard

This is a desktop shell client for Weston based on GTK. It was based
on weston-gtk-shell, a project by Tiago Vignatti.

License information can be found in the LICENSE file in the same
directory as this README.

To build maynard use:

     ./configure --prefix=<installdir>
     make
     make install

Before running maynard you need a ~/.config/weston.ini like

    [core]
    modules=<installdir>/lib/weston/shell-helper.so
   
    [shell]
    client=<installdir>/libexec/maynard
    focus-animation=dim-layer
    animation=zoom

You can then run maynard with

    <installdir>/bin/maynard
