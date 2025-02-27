# Installation management script for XACC benchmark plugin installation
#
import sys
import argparse
import os
import configparser
from shutil import copy
import xacc
MASTER_DIRS = ['vqe']

MASTER_PACKAGES = {}

TOP_PATH = os.path.dirname(os.path.realpath(__file__))

XACC_PYTHON_PLUGIN_PATH = "/root/.xacc/py-plugins/"

PLUGIN_INSTALLATIONS = {}


def parse_args(args):
    parser = argparse.ArgumentParser(description="Installation manager for XACC benchmark plugins.",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     fromfile_prefix_chars='@')
    parser.add_argument("-i", "--install", type=str, help="Install a plugin package to the XACC plugin directory.", required=False)
    parser.add_argument("-l", "--list", help="List all available plugin packages for installation.", required=False, action='store_true')
    parser.add_argument("-p", "--path", help="Set the XACC Python Plugin path (default = /root/.xacc/py-plugins)", required=False)

    opts = parser.parse_args(args)
    return opts

def install_package(install_name):
    try:
        package_path = PLUGIN_INSTALLATIONS[install_name]
    except KeyError as ex:
        xacc.info("There is no '{}' XACC Python plugin package available.".format(install_name))
        exit(1)

    install_directive = os.path.join(package_path+"/install.ini") if os.path.isfile(package_path+"/install.ini") else None
    plugin_files = []
    if not install_directive:
        plugin_files += [package_path+"/"+f for f in os.listdir(
            package_path) if os.path.isfile(os.path.join(package_path, f)) and f.endswith(".py")]
    else:
        plugin_dict, plugin_list = read_install_directive(install_directive, package_path)
        for k,v in plugin_dict.items():
            mini_package_path = v
            plugin_files += [v+"/"+f for f in os.listdir(v) if os.path.isfile(os.path.join(v, f)) and f.endswith(".py")]
    n_plugins = len(plugin_files)
    for plugin in plugin_files:
        copy(os.path.join(plugin), XACC_PYTHON_PLUGIN_PATH)

    xacc.info("Installed {} plugins from the '{}' package to the {} directory.".format(n_plugins, install_name, XACC_PYTHON_PLUGIN_PATH))

def set_plugin_path(path):
    global XACC_PYTHON_PLUGIN_PATH
    XACC_PYTHON_PLUGIN_PATH = path

def read_install_directive(install_file, parent):
    config = configparser.RawConfigParser()
    config.read(install_file)
    results = {}
    packages_here = []
    for section in config.sections():
        for installation in config.items(section):
            name, folder = installation
            packages_here.append(name)
            folder = parent+folder
            results.update(dict([(name, folder)]))
    return results, packages_here

def get_packages():

    for mdir in MASTER_DIRS:
        plugins_path = os.path.join(TOP_PATH, mdir)
        install_directives = plugins_path+"/install.ini"
        package_dict, package_list = read_install_directive(install_directives, plugins_path)
        PLUGIN_INSTALLATIONS.update(package_dict)

        if mdir not in MASTER_PACKAGES:
            MASTER_PACKAGES[mdir] = package_list
        else:
            MASTER_PACKAGES[mdir] += package_list

def main(argv=None):
    opts = parse_args(sys.argv[1:])
    get_packages()

    if opts.path:
        set_plugin_path(opts.path)

    if opts.install:
        install_package(opts.install)

    if opts.list:
        xacc.info("Available XACC Python plugin packages:")
        for k, v in MASTER_PACKAGES.items():
            xacc.info("{:5}: {!s}".format(k, v))

if __name__ == "__main__":
    sys.exit(main())
