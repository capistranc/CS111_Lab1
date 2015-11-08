#! /bin/sh
ls | cat > newfile.txt
cat newfile.txt > newfile2.txt
echo There is no place like venus > newfile2.txt
cat newfile1.txt

((echo abc) > newfile1.txt)
