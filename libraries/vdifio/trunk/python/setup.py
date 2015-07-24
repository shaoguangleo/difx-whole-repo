import os, sys

try:
    import ctypeslib.h2xml as h2xml
    import ctypeslib.xml2py as xml2py
    import ctypeslib.codegen as codegen
except:
    print ('Error: required Python ctypeslib module not installed!')
    sys.exit(-1)

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup
    from distutils.core import Command

src_path = os.path.relpath(os.path.join(os.path.dirname(__file__),'.'))
src_path += '/'

c_h_file = src_path + '../src/vdifio.h'
ctypes_xml_file = 'vdifio_api.xml'
ctypes_api_file = 'vdifio/vdifio.py'


# Ctypeslib generator invocations based on "reference" examples at
# https://github.com/piranna/cusepy/blob/master/setup.py
# http://s3ql.googlecode.com/hg-history/release-0.20/setup.py
def build_ctypes():


    # Step 1: Remove old files
    try:
        os.remove(ctypes_xml_file)
    except:
        pass
    try:
        os.remove(ctypes_api_file)
    except:
        pass

    # Configure ctypeslib
    codegen.ASSUME_STRINGS = False
    codegen.CDLL_SET_ERRNO = False
    codegen.PREFIX = ('# Code autogenerated by ctypeslib. Any changes will be lost!\n\n'
        '#pylint: disable-all\n'
        '#@PydevCodeAnalysisIgnore\n\n')

    # Generate XML file from C/C++ header file
    h2xml_args = ['h2xml.py', c_h_file, '-o',ctypes_xml_file, '-I',src_path]
    h2xml.main(h2xml_args)

    # Generate Python bindings from XML file
    # Note 1: to get -r <regex> to work correctly in xml2py.py v0.5.6 a patch is necessary (see README)
    # Note 2: uses libvdifio.so from the C/C++ build tree because 'make install' may not have been done yet
    print ('creating bindings %s ...' % (ctypes_api_file))
    xml2py_flags = ['-o',ctypes_api_file+'.tmp']
    xml2py_flags.extend(['-k','esf'])                   # standard flags
    xml2py_flags.extend(['-s','VDIFHeaderPrintLevel'])  # the enums to include in wrapper
    xml2py_flags.extend(['-s','vdif_header'])           # structs to include in wrapper
    xml2py_flags.extend(['-s','vdif_edv1_header', '-s','vdif_edv3_header', '-s','vdif_mux'])
    xml2py_flags.extend(['-s','vdif_mux_statistics', '-s','vdif_file_summary'])
    xml2py_flags.extend(['-r','VDIF', '-r','vdif', '-r','vdifmux']) # functions to include in wrapper
    xml2py_flags.extend(['-l','../src/.libs/libvdifio.so'])
    xml2py_args = ['xml2py.py', 'vdifio_api.xml']
    xml2py_args.extend(xml2py_flags)
    xml2py.main(xml2py_args)

    # Rename
    try:
        with open(ctypes_api_file, 'w') as fout:
            with open(ctypes_api_file+'.tmp', 'r') as fin:
                for line in fin:
                    fout.write(line.replace('../src/.libs/libvdifio.so', 'libvdifio.so'))
        os.remove(ctypes_api_file+'.tmp')
    except:
        pass


if sys.argv[-1]=='build':
    build_ctypes()

setup(
    name = 'vdifio',
    packages=['vdifio'],
    version = '1.1',
    description = ('A ctypes-based Python wrapper to the vdifio C/C++ library'),
    long_description=open('README').read(),
    license = 'LICENSE',
    # install_requires = 'ctypes>=1.1.0',
    requires = 'ctypes',
    package_dir={'': src_path},
    # cmdclass={'build_ctypes': build_ctypes},
)
