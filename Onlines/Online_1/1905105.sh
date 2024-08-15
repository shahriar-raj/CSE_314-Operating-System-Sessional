#!/bin/bash

name=$1
count=0
touch "out.txt"
wc -l $name >> "out.txt"

read $count < "out.txt"
echo $count 