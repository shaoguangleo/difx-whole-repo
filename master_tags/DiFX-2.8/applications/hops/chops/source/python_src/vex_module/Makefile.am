#
# $Id$
#
# MAKEFLAGS = -k if desperate

AM_CPPFLAGS = -I. -I$(srcdir)/include \
	-I$(srcdir)/../../c_src/msg/include \
	-I$(srcdir)/../../c_src/mk4util/include \
	-I$(srcdir)/../../c_src/vex/include  -Wall -Wextra -DHAVE_CONFIG_H

pkginclude_HEADERS = ./include/vexpy_module_extern.h
pkglib_LTLIBRARIES = libvexpyb.la

vexyorigsources = src/vexpy_module_extern.c
vexyhacksources = src/block_params.c \
	src/check_intrange.c src/check_realrange.c \
	src/check_scan.c src/check_stloc.c \
	src/check_strrange.c src/cvex_info.c \
	src/decode_pval.c src/do_antenna.c \
	src/do_bbc.c src/do_clock.c \
	src/do_das.c src/do_eop.c \
	src/do_exper.c src/do_freq.c \
	src/do_head_pos.c src/do_if.c \
	src/do_pass_order.c src/do_phase_cal_detect.c \
	src/do_roll.c src/do_site.c \
	src/do_source.c src/do_track.c \
	src/evex_info.c src/fill_deflists.c \
	src/fill_scanlist.c src/fill_station_parms.c \
	src/find_statements.c src/get_block_mode.c \
	src/get_chip_mode.c src/get_corr_bd_parms.c \
	src/get_corr_mode.c src/get_def.c \
	src/get_drive_init.c src/get_global_deflist.c \
	src/get_logscan.c src/get_mode_deflist.c \
	src/get_pbs_init.c src/get_pcm_config.c \
	src/get_pcm_tables.c src/get_section_mode.c \
	src/get_statement.c src/get_station_deflist.c \
	src/get_su_chan_out.c src/get_su_connect.c \
	src/get_trm_config.c src/get_val_list.c \
	src/get_version.c src/get_vex.c \
	src/in_comment.c src/in_quote.c \
	src/init_scan.c src/ivex_info.c \
	src/locate_blocks.c src/locate_cq.c \
	src/lvex_info.c src/nextchar.c \
	src/param_formats.c src/parse_date.c \
	src/parse_dec.c src/parse_pval.c \
	src/parse_ra.c src/parse_ref.c \
	src/parse_units.c src/parse_vexfile.c \
	src/print_location.c src/process_qstring.c \
	src/read_file.c src/scan_info.c \
	src/strip_text.c src/svex_info.c \
	src/vex_init.c src/write_vexfile.c

# give automake a clue it is in for trouble
BUILT_SOURCES = $(vexyorigsources) $(vexyhacksources) importstamp

$(vexyorigsources): importstamp
$(vexyhacksources): importstamp

importstamp: $(srcdir)/src $(srcdir)/../../c_src/vex/src
	[ -f Makefile.am ] || { [ -d src ] || mkdir src ;\
	[ -f src/vexpy_module_extern.c ] || cp -p $(srcdir)/src/vexpy_module_extern.c src ;\
	for s in $(vexyhacksources); do [ -f $$s ] || cp -p $(srcdir)/../../c_src/vex/$$s src ; done ; }
	touch importstamp

libvexpyb_la_SOURCES = $(vexyorigsources) $(vexyhacksources)

libvexpyb_la_LIBADD = ../../c_src/msg/libmsgb.la ../../c_src/mk4util/libmk4utilb.la

libvexpyb_la_LDFLAGS = -avoid-version

SRC_FILES = vexpy/__init__.py \
	vexpy/vexpy.py

EXTRA_DIST = $(SRC_FILES) setup.py.in

if HAVE_PYTHON

all-local:
	(mkdir -p ./build)
	(rsync -av --exclude 'Makefile.*' $(srcdir)/ .)
	(chmod -R u+rwx .)
	($(PYTHON) setup.py build --verbose)

clean-local:
	rm -rf ./build
	[ -f Makefile.am ] || rm -rf src

install-exec-local:
	($(PYTHON) setup.py install \
		--prefix $(DESTDIR)$(prefix) \
		--record $(DESTDIR)$(pythondir)/$(PKG_NAME)/install_vexpy_files.txt \
		--verbose)

uninstall-local:
	cat $(DESTDIR)$(pythondir)/$(PKG_NAME)/install_vexpy_files.txt | xargs rm -rf
	rm -rf $(DESTDIR)$(pythondir)/$(PKG_NAME)/install_vexpy_files.txt

CLEANFILES = $(SRC_FILES) ./include/vexpy_module_extern.h ./src/vexpy_module_extern.c setup.py.in \
	importstamp

DISTCLEANFILES = setup.py

endif

#
# eof
#
