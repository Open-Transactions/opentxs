#!/bin/bash

find . -name '*.json' -not -path './Seednodes.json' -exec xxd -i {} xxd/{}.h ';'
