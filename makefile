all:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build"
kissat:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" kissat
tissat:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" tissat
clean:
	rm -f "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2"/makefile
	-$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" clean
	rm -rf "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build"
coverage:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" coverage
indent:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" indent
test:
	$(MAKE) -C "/home/sabrine/Documents/sat_solvers/kissat-MAB-saga/kissat-MAB/kissat-1.0.3-79d8d8f2/build" test
.PHONY: all clean coverage indent kissat test tissat
