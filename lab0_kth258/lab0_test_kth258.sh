date
echo "Compiling intro_kth258.cpp..."
g++ intro_kth258.cpp -o intro_kth258
echo "Running program first time..."
./intro_kth258
echo "Running program second time, saving output..."
./intro_kth258 > my.out
echo "Comparing with expected..."
diff -y expected.out my.out
echo "Current user is: $(whoami)"
