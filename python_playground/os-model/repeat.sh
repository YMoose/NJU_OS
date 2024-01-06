for i in $(seq 1 300) 
do
    /home/yzh/NJU_os/python_playground/mosaic/mosaic.py -r /home/yzh/NJU_os/python_playground/os-model/race.py | grep SUM | grep stdout
done