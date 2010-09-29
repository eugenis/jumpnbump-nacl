#!/usr/bin/env python

# Author: Martin Willemoes Hansen
# License: Gnu GPL

# Next two lines are a workaround for Debian bug 163794
import sys

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import gtk.gdk
import gobject
import os
import tempfile
import shutil
import gettext

RESOURCE_DIR='/usr/share/games/jumpnbump'
BINARY_DIR='/usr/games'

application = "jumpnbump-menu"
gettext.install(application)

def populate_treeview():
    levels = []
    for file in os.listdir (RESOURCE_DIR):
        if (file.endswith ('.dat')):
            levels.append (file)

    levels.sort()

    COLUMN_LEVEL = 0
    store = gtk.ListStore (gobject.TYPE_STRING)

    for level in levels:
        iter = store.append()
        store.set (iter, COLUMN_LEVEL, level)
                        
    treeview.set_model (store)

    renderer = gtk.CellRendererText()
    treeview.append_column (gtk.TreeViewColumn (_('Level'), renderer, text=COLUMN_LEVEL))

def standalone_mode (widget):
    disable_enable_level (1)
    disable_enable_server (0)
    num_clients.set_sensitive (0)
    nogore.set_sensitive (1)
    noflies.set_sensitive (1)

def client_mode (widget):
    disable_enable_level (1)
    disable_enable_server (1)
    num_clients.set_sensitive (0)
    nogore.set_sensitive (1)
    noflies.set_sensitive (1)

def server_mode (widget):
    disable_enable_level (1)
    disable_enable_server (0)
    num_clients.set_sensitive (1)
    nogore.set_sensitive (1)
    noflies.set_sensitive (1)

def fireworks_mode (widget):
    disable_enable_level (0)
    disable_enable_server (0)
    nogore.set_sensitive (0)
    nogore.set_active (0)
    noflies.set_sensitive (0)
    noflies.set_active (0)

def disable_enable_server (setting):
    server_entry.set_sensitive (setting)
    player_num.set_sensitive (setting)

def disable_enable_level (setting):
    treeview.set_sensitive (setting)
    mirror.set_sensitive (setting)
    if (not setting):
        mirror.set_active (setting)

def level_changed (widget):
    model, iter = treeview.get_selection().get_selected()
    global choosen_level
    choosen_level = '%s/%s' % (RESOURCE_DIR, model.get_value (iter, 0))
    unpackdir = None
    try:
        unpackdir = tempfile.mkdtemp ("", "jumpnbump-menu-")
        os.chdir(unpackdir)
        os.spawnlp (os.P_WAIT, '/usr/lib/jumpnbump/unpack', 'unpack', choosen_level)
        os.spawnlp (os.P_WAIT, 'convert', 'convert', '-scale', '50%', 'level.pcx', 'level_scaled.pcx')
        os.spawnlp (os.P_WAIT, 'convert', 'convert', 'level_scaled.pcx', 'level.png')
        image.set_from_file ('level.png')
    except Exception, err:
        print err
    finally:
        if unpackdir != None:
            shutil.rmtree(unpackdir)

    image.show()

def about (widget):
    global about_dialog

    if (not about_dialog):
        gui = gtk.glade.XML (gladefile, 'about', domain=application)
        about_dialog = gui.get_widget ('about')
        gui.signal_connect ('ok', about_close)

def about_close (widget):
    global about_dialog

    about_dialog.destroy()
    about_dialog = None

def run (widget):
    if (standalone.get_active()):
        execute (*get_level()+common_options())
    elif (fireworks.get_active()):
        execute ('-fireworks', *common_options())
    elif (client.get_active()):
        execute ('-player', str (player_num.get_value_as_int()),
                 '-connect', server_entry.get_text(),
                 *get_level() + common_options())
    else:
        execute ('-server', str (num_clients.get_value_as_int()),
                 *get_level() + common_options())

def get_level():
    level = []
    if (mirror.get_active()):
        level.append ('-mirror')
    else:
        level.append ('-dat')
        level.append (choosen_level)

    return level

def common_options():
    options = []

    if (fullscreen.get_active()):
        options.append ('-fullscreen')
    if (nogore.get_active()):
        options.append ('-nogore')
    if (double_res.get_active()):
        options.append ('-scaleup')
    if (nosound.get_active()):
        options.append ('-nosound')
    if (noflies.get_active()):
        options.append ('-noflies')
    if (withmusic.get_active()):
        options.append ('-musicnosound')
    if (mirror.get_active()):
        options.append ('-mirror')

    return options

def execute (*cmd):
    try:
        os.spawnl (os.P_NOWAIT, BINARY_DIR + '/jumpnbump', 'jumpnbump', *cmd)
    except Exception, err:
        print err

def main():
    global gladefile

    global_gladefile = RESOURCE_DIR + '/jumpnbump_menu.glade'
    local_gladefile = './jumpnbump_menu.glade'

    if (os.access (global_gladefile, os.R_OK)):
        gladefile = global_gladefile
        del local_gladefile
    elif (os.access (local_gladefile, os.R_OK)):
        gladefile = local_gladefile
        del global_gladefile
    else:
        print 'Could not find the glade file'
        return 0

    gui = gtk.glade.XML (gladefile, 'main', domain=application)

    global about_dialog, choosen_level, standalone, fireworks, client, server, treeview, \
           mirror, num_clients, server_entry, player_num, fullscreen, \
           nogore, double_res, nosound, noflies, withmusic, image

    about_dialog = None
    choosen_level = ''
    standalone = gui.get_widget ('standalone')
    fireworks = gui.get_widget ('fireworks')
    client = gui.get_widget ('client')
    server = gui.get_widget ('server')
    treeview = gui.get_widget ('level_treeview')
    populate_treeview()
    mirror = gui.get_widget ('mirror')
    num_clients = gui.get_widget ('num_of_clients')
    server_entry = gui.get_widget ('server_entry')
    player_num = gui.get_widget ('player_num')
    fullscreen = gui.get_widget ('fullscreen')
    nogore = gui.get_widget ('nogore')
    double_res = gui.get_widget ('double_res')
    nosound = gui.get_widget ('nosound')
    noflies = gui.get_widget ('noflies')
    withmusic = gui.get_widget ('withmusic')
    image = gui.get_widget ('image')
    
    gui.signal_autoconnect ({'standalone_mode': standalone_mode,
                             'client_mode': client_mode,
                             'server_mode': server_mode,
                             'fireworks_mode': fireworks_mode,
                             'level_changed': level_changed, 
                             'quit': lambda *args: gtk.main_quit(),
                             'run': run,
                             'about': about})
    
    gtk.main()

if __name__ == '__main__':
    main()
