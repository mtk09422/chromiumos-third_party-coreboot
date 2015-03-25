cbfsobj :=
cbfsobj += cbfstool.o
cbfsobj += common.o
cbfsobj += compress.o
cbfsobj += cbfs_image.o
cbfsobj += cbfs-mkstage.o
cbfsobj += cbfs-mkpayload.o
cbfsobj += elfheaders.o
cbfsobj += xdr.o
cbfsobj += fit.o
cbfsobj += partitioned_file.o
# LZMA
cbfsobj += lzma.o
cbfsobj += LzFind.o
cbfsobj += LzmaDec.o
cbfsobj += LzmaEnc.o
# FMAP
cbfsobj += fmap.o
cbfsobj += kv_pair.o
cbfsobj += valstr.o
# linux as payload
cbfsobj += linux_trampoline.o
cbfsobj += cbfs-payload-linux.o

fmapobj :=
fmapobj += fmaptool.o
fmapobj += cbfs_sections.o
fmapobj += fmap_from_fmd.o
fmapobj += fmd.o
fmapobj += fmd_parser.o
fmapobj += fmd_scanner.o
# FMAP
fmapobj += fmap.o
fmapobj += kv_pair.o
fmapobj += valstr.o

rmodobj :=
rmodobj += rmodtool.o
rmodobj += rmodule.o
rmodobj += common.o
rmodobj += elfheaders.o
rmodobj += xdr.o

TOOLCFLAGS := -Og -g3
TOOLCFLAGS += -std=c99 -Werror -Wall -Wextra
TOOLCFLAGS += -Wcast-qual -Wmissing-prototypes -Wredundant-decls -Wshadow
TOOLCFLAGS += -Wstrict-prototypes -Wwrite-strings
TOOLCPPFLAGS := -D_POSIX_C_SOURCE=200809L # strdup() from string.h
TOOLCPPFLAGS += -D_SVID_SOURCE # memccpy() from string.h
TOOLCPPFLAGS += -D_7ZIP_ST
TOOLCPPFLAGS += -I$(top)/util/cbfstool/flashmap
TOOLLINKFLAGS := -g3

ifeq ($(shell uname -s | cut -c-7 2>/dev/null), MINGW32)
TOOLFLAGS+=-mno-ms-bitfields
endif

$(objutil)/cbfstool:
	mkdir -p $@

$(objutil)/cbfstool/%.o: $(top)/util/cbfstool/%.c
	printf "    HOSTCC     $(subst $(objutil)/,,$(@))\n"
	$(HOSTCC) $(TOOLCPPFLAGS) $(TOOLCFLAGS) $(HOSTCFLAGS) -c -o $@ $<

$(objutil)/cbfstool/%.o: $(top)/util/cbfstool/flashmap/%.c
	printf "    HOSTCC     $(subst $(objutil)/,,$(@))\n"
	$(HOSTCC) $(TOOLCPPFLAGS) $(TOOLCFLAGS) $(HOSTCFLAGS) -c -o $@ $<

$(objutil)/cbfstool/%.o: $(top)/util/cbfstool/lzma/%.c
	printf "    HOSTCC     $(subst $(objutil)/,,$(@))\n"
	$(HOSTCC) $(TOOLCPPFLAGS) $(TOOLCFLAGS) $(HOSTCFLAGS) -c -o $@ $<

$(objutil)/cbfstool/%.o: $(top)/util/cbfstool/lzma/C/%.c
	printf "    HOSTCC     $(subst $(objutil)/,,$(@))\n"
	$(HOSTCC) $(TOOLCPPFLAGS) $(TOOLCFLAGS) $(HOSTCFLAGS) -c -o $@ $<

$(objutil)/cbfstool/cbfstool: $(objutil)/cbfstool $(addprefix $(objutil)/cbfstool/,$(cbfsobj))
	printf "    HOSTCC     $(subst $(objutil)/,,$(@)) (link)\n"
	$(HOSTCC) $(TOOLLINKFLAGS) -o $@ $(addprefix $(objutil)/cbfstool/,$(cbfsobj))

$(objutil)/cbfstool/fmaptool: $(objutil)/cbfstool $(addprefix $(objutil)/cbfstool/,$(fmapobj))
	printf "    HOSTCC     $(subst $(objutil)/,,$(@)) (link)\n"
	$(HOSTCC) $(TOOLLDFLAGS) -o $@ $(addprefix $(objutil)/cbfstool/,$(fmapobj))

$(objutil)/cbfstool/rmodtool: $(objutil)/cbfstool $(addprefix $(objutil)/cbfstool/,$(rmodobj))
	printf "    HOSTCC     $(subst $(objutil)/,,$(@)) (link)\n"
	$(HOSTCC) $(TOOLLINKFLAGS) -o $@ $(addprefix $(objutil)/cbfstool/,$(rmodobj))

# Yacc source is superset of header
$(objutil)/cbfstool/fmd_parser.o: TOOLCFLAGS += -Wno-redundant-decls
# Lex generates unneeded functions
$(objutil)/cbfstool/fmd_scanner.o: TOOLCFLAGS += -Wno-unused-function
# Tolerate lzma sdk warnings
$(objutil)/cbfstool/LzmaEnc.o: TOOLCFLAGS += -Wno-sign-compare -Wno-cast-qual

ifeq ($(CONFIG_FMD_GENPARSER),y)
$(objutil)/cbfstool/fmd.o: $(top)/util/cbfstool/fmd_parser.c
$(objutil)/cbfstool/fmd.o: $(top)/util/cbfstool/fmd_scanner.c
$(objutil)/cbfstool/fmd_parser.o: $(objutil)/cbfstool/fmd.o
$(objutil)/cbfstool/fmd_scanner.o: $(objutil)/cbfstool/fmd.o
$(top)/util/cbfstool/fmd_scanner.c $(top)/util/cbfstool/fmd_scanner.h: $(top)/util/cbfstool/fmd_scanner.l
	echo "    LEX        util/cbfstool/fmd_scanner.[ch]"
	$(LEX) -t --header-file=$(top)/util/cbfstool/fmd_scanner.h $< >$(top)/util/cbfstool/fmd_scanner.c
$(top)/util/cbfstool/fmd_parser.c $(top)/util/cbfstool/fmd_parser.h: $(top)/util/cbfstool/fmd_parser.y
	echo "    YACC       util/cbfstool/fmd_parser.[ch]"
	$(YACC) -d $<
	mv -f y.tab.c $(top)/util/cbfstool/fmd_parser.c
	mv -f y.tab.h $(top)/util/cbfstool/fmd_parser.h
endif
