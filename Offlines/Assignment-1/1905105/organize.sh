#!/bin/bash

Usage()
{
    echo "Usage:"
    echo "./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]"
    echo "-v: verbose"
    echo "-noexecute: do not execute code files"
    kill -INT $$
}

v=0
nex=0

if [ $# -lt 4 ]
then 
    Usage
fi

if [ $# -eq 5 ]
then
    if [ $5 = "-v" ]
    then
        v=1
    elif [ $5 = "-noexecute" ]
    then
        nex=1
    else
        Usage
    fi
fi

if [ $# -eq 6 ]
then
    if [ $5 = "-v" ]
    then
        v=1
    else
        Usage
    fi
    if [ $6 = "-noexecute" ]
    then
        nex=1
    else
        Usage
    fi
fi

if [ ! -d "$2" ]
then
    mkdir "$2"
fi

C_d="$2/C"
J_d="$2/Java"
P_d="$2/Python"
test_d="$3"
ans_d="$4"
string1="n"
target="$2"

if [ -d "$C_d" ]
then
    rm -r "$C_d"
fi
if [ -d "$J_d" ]
then
    rm -r "$J_d"
fi
if [ -d "$P_d" ]
then
    rm -r "$P_d"
fi
if [ -f "$2/result.csv" ]
then
    rm "$2/result.csv"
fi

mkdir $C_d $J_d $P_d
mkdir "temp"

if [ $nex -eq 0 ]
then
    touch "$2/result.csv"
    chmod u+w "$2/result.csv"
    echo "student_id,type,matched,not_matched" >>"$2/result.csv"
fi

verbo()
{
    if [ $v -eq 1 ]
    then
        echo "Organizing files of $string1"
    fi 
}

exec()
{
    if [ $v -eq 1 ]
    then
        echo "Executing files of $string1"
    fi 
}

visit()
{
	if [ -d "$1" ]
	then
	
		for i in "$1"/*
		do
			visit "$i"
		done
	
	elif [ -f "$1" ]
	then
		count=1
        matched=0
        not_matched=0
        if [ ${i##*.} = "c" ]
        then
            verbo
            mkdir "$C_d/$string1"
            mv -- "$i" "main.c"
            if [ $nex -eq 0 ]
            then
            gcc "main.c" -o "main.out"
            fi
            mv "main.c" "$C_d/$string1"
            if [ $nex -eq 0 ]
            then
            mv "main.out" "$C_d/$string1"            
            exec
            for tfiles in "$test_d"/*
            do
                "./$C_d/$string1/main.out" <$tfiles >"$C_d/$string1/out$count.txt"
                diff -q "$C_d/$string1/out$count.txt" "$ans_d/ans$count.txt" > /dev/null 2>&1
                if [ $? -eq 0 ]
                then
                   matched=$(($matched + 1))
                else
                    not_matched=$(($not_matched + 1))
                fi
                count=$(($count + 1))
            done
            echo "$string1,C,$matched,$not_matched" >>"$target/result.csv"
            fi
        elif [ ${i##*.} = "java" ]
        then 
            verbo
            mkdir "$J_d/$string1"
            mv -- "$i" "Main.java"
            if [ $nex -eq 0 ]
            then
                javac "Main.java"
            fi
            mv "Main.java" "$J_d/$string1"
            if [ $nex -eq 0 ]
            then
            mv "Main.class" "$J_d/$string1"
            exec
            for tfiles in "$test_d"/*
            do
               java -cp "$J_d/$string1/" "Main" <$tfiles >"$J_d/$string1/out$count.txt"
               diff -q "$J_d/$string1/out$count.txt" "$ans_d/ans$count.txt" > /dev/null 2>&1
                if [ $? -eq 0 ]
                then
                   matched=$(($matched + 1))
                else
                    not_matched=$(($not_matched + 1))
                fi
               count=$(($count + 1))
            done
            echo "$string1,Java,$matched,$not_matched" >>"$target/result.csv"
            fi
        elif [ ${i##*.} = "py" ]
        then 
            verbo
            mkdir "$P_d/$string1"
            mv -- "$i" "main.py"
            mv "main.py" "$P_d/$string1"
            if [ $nex -eq 0 ]
            then
            exec
            for tfiles in "$test_d"/*
            do
               python3 "$P_d/$string1/main.py" <$tfiles >"$P_d/$string1/out$count.txt"
               diff -q "$P_d/$string1/out$count.txt" "$ans_d/ans$count.txt" > /dev/null 2>&1
                if [ $? -eq 0 ]
                then
                   matched=$(($matched + 1))
                else
                    not_matched=$(($not_matched + 1))
                fi
               count=$(($count + 1))
            done
            echo "$string1,Python,$matched,$not_matched" >>"$target/result.csv"
            fi
        fi
	fi
}

for files in "$1"/*
do
    string=${files%.zip}
    string1="${string: -7}"
    unzip -q "$files" -d "temp"
    visit "temp"
    # rm -r "temp"/*
done

rm -r temp
