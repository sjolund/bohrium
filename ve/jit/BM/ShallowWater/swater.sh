#! /bin/tcsh
echo "--- C ---"
foreach i (1 2 3 4 5)
    ./swater-boost 1000 10 | grep time | awk '{print $6}'
end
echo "--- NumPy ---"
foreach i (1 2 3 4 5)
    python2.7 swater.py --size=1000*10 --cphvb=False
end
echo "--- Bh ---"
foreach i (1 2 3 4 5)
    python2.7 swater.py --size=1000*10 --cphvb=True
end

