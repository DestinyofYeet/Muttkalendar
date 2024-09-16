#!/bin/bash

/usr/bin/valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all $@
