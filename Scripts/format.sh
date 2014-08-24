#!/bin/bash

astyle --options=style.cfg -r -v --suffix=none ../nfEngine/*.cpp ../nfEngine/*.h ../nfEngineTest/*.cpp ../nfEngineTest/*.h
