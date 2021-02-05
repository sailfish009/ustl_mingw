-include Config.mk

################ Source files ##########################################

SRCS	:= $(wildcard *.cc)
INCS	:= $(wildcard *.h)
OBJS	:= $(addprefix $O,$(SRCS:.cc=.o))
DEPS	:= ${OBJS:.o=.d}
MKDEPS	:= Makefile Config.mk config.h $O.d
ONAME	:= $(notdir $(abspath $O))
LIBA	:= $Olib${NAME}.a

################ Compilation ###########################################

.PHONY: all clean html check distclean maintainer-clean

all:	${LIBA}

${LIBA}:	${OBJS}
	@echo "Linking $(notdir $@) ..."
	@rm -f $@
	@${AR} qc $@ ${OBJS}
	@${RANLIB} $@

$O%.o:	%.cc
	@echo "    Compiling $< ..."
	@${CXX} ${CXXFLAGS} -MMD -MT "$(<:.cc=.s) $@" -o $@ -c $<

%.s:	%.cc
	@echo "    Compiling $< to assembly ..."
	@${CXX} ${CXXFLAGS} -S -o $@ -c $<

include test/Module.mk

################ Installation ##########################################

.PHONY:	install uninstall install-incs uninstall-incs

####### Install headers

ifdef INCDIR	# These ifdefs allow cold bootstrap to work correctly
LIDIR	:= ${INCDIR}/${NAME}
INCSI	:= $(addprefix ${LIDIR}/,$(filter-out ${NAME}.h,${INCS}))
RINCI	:= ${LIDIR}.h

install:	install-incs
install-incs: ${INCSI} ${RINCI}
${LIDIR}:
	@echo "Creating $@ ..."
	@mkdir -p $@
${INCSI}: ${LIDIR}/%.h: %.h |${LIDIR}
	@echo "Installing $@ ..."
	@${INSTALLDATA} $< $@
${RINCI}: ${NAME}.h |${LIDIR}
	@echo "Installing $@ ..."
	@${INSTALLDATA} $< $@
uninstall:	uninstall-incs
uninstall-incs:
	@if [ -d ${LIDIR} -o -f ${RINCI} ]; then\
	    echo "Removing ${LIDIR}/ and ${RINCI} ...";\
	    rm -f ${INCSI} ${RINCI};\
	    ${RMPATH} ${LIDIR};\
	fi
endif

####### Install libraries (shared and/or static)

ifdef LIBDIR
LIBTI	:= ${LIBDIR}/$(notdir ${SLIBT})
LIBLI	:= $(addprefix ${LIBDIR}/,$(notdir ${SLINKS}))
LIBAI	:= ${LIBDIR}/$(notdir ${LIBA})

${LIBDIR}:
	@echo "Creating $@ ..."
	@mkdir -p $@

install:	${LIBAI}
${LIBAI}:	${LIBA} |${LIBDIR}
	@echo "Installing $@ ..."
	@${INSTALLLIB} $< $@

uninstall:
	@if [ -f ${LIBAI} ]; then\
	    echo "Removing library from ${LIBDIR} ...";\
	    rm -f ${LIBAI};\
	fi
endif
ifdef PKGCONFIGDIR
PCI	:= ${PKGCONFIGDIR}/ustl.pc
install:	${PCI}
${PKGCONFIGDIR}:
	@echo "Creating $@ ..."
	@mkdir -p $@
${PCI}:	ustl.pc |${PKGCONFIGDIR}
	@echo "Installing $@ ..."
	@${INSTALLDATA} $< $@

uninstall:	uninstall-pc
uninstall-pc:
	@if [ -f ${PCI} ]; then echo "Removing ${PCI} ..."; rm -f ${PCI}; fi
endif

################ Maintenance ###########################################

clean:
	@if [ -h ${ONAME} ]; then\
	    rm -f ${OBJS} ${DEPS} ${LIBA} $O.d ${ONAME};\
	    ${RMPATH} ${BUILDDIR} > /dev/null 2>&1 || true;\
	fi

distclean:	clean
	@rm -f Config.mk config.h config.status

maintainer-clean: distclean

$O.d:	${BUILDDIR}/.d
	@[ -h ${ONAME} ] || ln -sf ${BUILDDIR} ${ONAME}
${BUILDDIR}/.d:	Makefile
	@mkdir -p ${BUILDDIR} && touch ${BUILDDIR}/.d

${OBJS}:		${MKDEPS}
Config.mk:		Config.mk.in
config.h:		config.h.in| Config.mk
ustl.pc:		ustl.pc.in| Config.mk
Config.mk config.h ustl.pc:	configure
	@if [ -x config.status ]; then			\
	    echo "Reconfiguring ..."; ./config.status;	\
	else						\
	    echo "Running configure ..."; ./configure;	\
	fi

-include ${DEPS}
