g++ main.cpp -o compiler
g++ runtime/runtime.cpp -o runner
./compiler abc.txt 
./runner out.txt