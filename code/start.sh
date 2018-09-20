redis-cli set lindex  -19725907.844670,6726597.418098,-14457310.511787,11535997.595048
redis-cli set pindex -19666112.513398,6772685.563274,-14460657.755292,11536208.957654
nohup mpirun -np 32 ./Buffer >mpi.log 2>&1 &
nohup uwsgi --http :55559 --wsgi-file ./uwsgitileserver/server.py --master --processes 16 --threads 2 > ./uwsgiserver.log 2>&1 &
