#! /bin/tcsh
echo "--- C ---"
foreach i (1 2 3 4 5)
    ./BlackScholes | grep time | awk '{print $6}'
end
echo "--- Numpy ---"
foreach i (1 2 3 4 5)
    python BlackScholes.py
end
echo "--- Bohrium ---"
foreach i (1 2 3 4 5)
    python BlackScholes-bh.py
end

