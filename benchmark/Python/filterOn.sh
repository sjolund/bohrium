#!/bin/bash
dst="/home/chris/.bohrium/config.ini"
src_filterOn="/home/chris/.bohrium/config-filterOn.ini"
src_filterOff="/home/chris/.bohrium/config-filterOff.ini"


echo "Filter turned on"
cp $src_filterOn $dst

