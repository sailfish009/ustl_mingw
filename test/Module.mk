################ Source files ##########################################

test/SRCS	:= $(wildcard test/?????.cc)
test/TESTS	:= $(addprefix $O,$(test/SRCS:.cc=))
test/OBJS	:= $(addprefix $O,$(test/SRCS:.cc=.o))
test/LIBS	:= ${LIBA}
ifdef NOLIBSTDCPP
test/LIBS	+= ${LIBS} -lm
endif
test/DEPS	:= ${test/OBJS:.o=.d} $Otest/stdtest.d
test/OUTS	:= $(addprefix $O,$(test/SRCS:.cc=.out))

################ Compilation ###########################################

.PHONY:	test/all test/run test/clean test/check

test/all:	${test/TESTS}

# The correct output of a test is stored in testXX.std
# When the test runs, its output is compared to .std
#
test/run:	${test/TESTS}
	@echo "Running verification tests:";\
	for i in ${test/TESTS}; do	\
	    TEST="test/$$(basename $$i)";	\
	    echo "Running $$i";		\
	    ./$$i < $$TEST.cc > $$i.out 2>&1;	\
	    diff $$TEST.std $$i.out && rm -f $$i.out; \
	done

${test/TESTS}: $Otest/%: $Otest/%.o $Otest/stdtest.o ${LIBA}
	@echo "Linking $@ ..."
	@${LD} ${LDFLAGS} -o $@ $< $Otest/stdtest.o ${test/LIBS}

$Otest/.d:	$O.d
	@[ -d $Otest ] || mkdir $Otest
	@touch $Otest/.d

################ Maintenance ###########################################

clean:	test/clean
test/clean:
	@if [ -d $Otest ]; then\
	    rm -f ${test/TESTS} ${test/OBJS} ${test/DEPS} ${test/OUTS} $Otest/stdtest.o $Otest/.d;\
	    rmdir $Otest;\
	fi
check:		test/run
test/check:	check

${test/OBJS} $Otest/stdtest.o:	${MKDEPS} $Otest/.d

-include ${test/DEPS}
