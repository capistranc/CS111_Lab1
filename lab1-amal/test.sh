#! /bin/sh

echo a > b.txt

cat b.txt

echo 123 > file.txt

cat file.txt > b.txt

rm b.txt
 
rm file.txt